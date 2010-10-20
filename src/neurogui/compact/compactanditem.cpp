/*
Neurocognitive Linguistics Lab
Copyright (c) 2010, Gordon Tisher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

 - Neither the name of the Neurocognitive Linguistics Lab nor the
   names of its contributors may be used to endorse or promote
   products derived from this software without specific prior
   written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include "compactanditem.h"
#include "../mixins/mixinarrow.h"
#include "../labnetwork.h"
#include "../labscene.h"

using namespace NeuroLib;

namespace NeuroGui
{

    CompactAndItem::CompactAndItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : CompactNodeItem(network, scenePos, context),
        _sequential(false), _delay(1),
        _sequential_property(this, &CompactAndItem::sequential, &CompactAndItem::setSequential,
                           tr("Sequential"), tr("Whether or not the node will sequence its inputs/outputs in time.")),
        _delay_property(this, &CompactAndItem::delay, &CompactAndItem::setDelay,
                        tr("Delay"), tr("How long the node will delay between firing (if sequential)."))
    {
        _delay_property.setEditable(false);
    }

    CompactAndItem::~CompactAndItem()
    {
        if (_ui_delete)
        {
            teardownDelayLines();
        }
    }

    void CompactAndItem::setSequential(const bool &seq)
    {
        if (seq != _sequential)
        {
            // disconnect from connections
            foreach (NeuroItem *ni, connections())
                removeEdges(ni);

            // build or tear down delay lines
            if (seq)
            {
                buildDelayLines();
            }
            else
            {
                teardownDelayLines();
            }

            _sequential = seq;

            // re-connect
            foreach (NeuroItem *ni, connections())
                addEdges(ni);

            _delay_property.setEditable(_sequential);
            adjustLinks();
        }
    }

    void CompactAndItem::setDelay(const qint32 & d)
    {
        bool updateValue = false;
        qint32 newDelay = d;

        if (newDelay < 1)
        {
            updateValue = true;
            newDelay = 1;
        }

        if (newDelay != _delay)
        {
            // disconnect
            foreach (NeuroItem *ni, connections())
                removeEdges(ni);

            // tear down then build delay lines
            teardownDelayLines();

            _delay = newDelay;
            buildDelayLines();

            // re-connect
            foreach (NeuroItem *ni, connections())
                addEdges(ni);
        }

        if (updateValue)
            _delay_property.setValue(QVariant(newDelay));
    }

    NeuroCell::Index CompactAndItem::getIncomingCellFor(const NeuroItem *item) const
    {
        if (item == _tipLinkItem)
        {
            return _backwardTipCell;
        }
        else if (_sequential)
        {
            int index = _baseLinkItems.indexOf(const_cast<NeuroItem *>(item));
            if (index != -1 && index < _frontwardDelayLines.size())
                return _frontwardDelayLines[index].size() > 0 ? _frontwardDelayLines[index].first() : _frontwardTipCell;
            else
                return -1;
        }
        else
        {
            return _frontwardTipCell;
        }
    }

    NeuroCell::Index CompactAndItem::getOutgoingCellFor(const NeuroItem *item) const
    {
        if (item == _tipLinkItem)
        {
            return _frontwardTipCell;
        }
        else if (_sequential)
        {
            int index = _baseLinkItems.indexOf(const_cast<NeuroItem *>(item));
            if (index != -1 && index < _backwardDelayLines.size())
                return _backwardDelayLines[index].size() > 0 ? _backwardDelayLines[index].last() : _backwardTipCell;
            else
                return -1;
        }
        else
        {
            return _backwardTipCell;
        }
    }

    bool CompactAndItem::canBeAttachedBy(const QPointF & pos, NeuroItem *item)
    {
        if (dynamic_cast<MixinArrow *>(item) != 0 && item != _tipLinkItem)
        {
            if (scenePosOnTip(pos))
                return _tipLinkItem == 0;
            else
                return !_baseLinkItems.contains(item);
        }

        return false;
    }

    void CompactAndItem::onAttachedBy(NeuroItem *item)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        // get the position
        bool onTip = false;
        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        if (link)
        {
            if (link->dragFront() && link->frontLinkTarget() == this)
                onTip = scenePosOnTip(link->line().p2());
            else if (!link->dragFront() && link->backLinkTarget() == this)
                onTip = scenePosOnTip(link->line().p1());
        }
        else
        {
            onTip = scenePosOnTip(item->scenePos());
        }

        // if it's attached to the tip, connect in and out
        if (onTip)
        {
            _tipLinkItem = item;
        }

        // if it's attached to the base, connect in and out; adjust node threshold
        else
        {
            _baseLinkItems.append(item);
            adjustNodeThreshold();
        }

        // this will re-space the nodes
        CompactNodeItem::onAttachedBy(item);

        // add delay lines for sequence node
        if (_sequential && !onTip)
        {
            foreach (NeuroItem *ni, connections())
                removeEdges(ni);

            // frontwards has to add on the beginning of all of them
            for (int i = 0; i < _frontwardDelayLines.size(); ++i)
            {
                NeuroCell::Index prevIndex = _frontwardDelayLines[i].size() > 0 ? _frontwardDelayLines[i].first() : _frontwardTipCell;

                for (int j = 0; j < _delay; ++j)
                {
                    NeuroCell::Index newIndex = network()->neuronet()->addNode(NeuroCell(NeuroCell::EXCITORY_LINK));
                    if (prevIndex != -1 && newIndex != -1)
                        network()->neuronet()->addEdge(prevIndex, newIndex);
                    _frontwardDelayLines[i].insert(0, newIndex);
                }
            }
            _frontwardDelayLines.append(buildDelayLine(0, -1, _frontwardTipCell));

            // backwards just add one on the end
            _backwardDelayLines.append(buildDelayLine(_baseLinkItems.size()-1, _backwardTipCell, -1));

            foreach (NeuroItem *ni, connections())
                addEdges(ni);

            adjustLinks();
        }
        else
        {
            addEdges(item);
        }
    }

    void CompactAndItem::onDetach(NeuroItem *item)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        // remove delay line
        if (_sequential && item != _tipLinkItem)
        {
            // disconnect cells
            foreach (NeuroItem *ni, connections())
                removeEdges(ni);

            // trim delay lines
            int item_index = _baseLinkItems.indexOf(item);
            if (item_index != -1)
            {
                // frontwards; remove the first cell from each up to the item
                for (int i = 0; i < item_index; ++i)
                {
                    for (int j = 0; j < _delay; ++j)
                    {
                        network()->neuronet()->removeNode(_frontwardDelayLines[i].first());
                        _frontwardDelayLines[i].removeFirst();
                    }
                }

                clearDelayLine(_frontwardDelayLines[item_index]);
                _frontwardDelayLines.removeAt(item_index);

                // backwards; remove last cell from lines after the item
                for (int i = item_index + 1; i < _backwardDelayLines.size(); ++i)
                {
                    for (int j = 0; j < _delay; ++j)
                    {
                        network()->neuronet()->removeNode(_backwardDelayLines[i].last());
                        _backwardDelayLines[i].removeLast();
                    }
                }

                clearDelayLine(_backwardDelayLines[item_index]);
                _backwardDelayLines.removeAt(item_index);

                // remove the item
                _baseLinkItems.removeAt(item_index);
            }

            // re-connect cells
            foreach (NeuroItem *ni, connections())
                if (ni != item)
                    addEdges(ni);

            // re-space links
            adjustLinks();
        }
        else
        {
            removeEdges(item);
        }

        // clean up
        CompactNodeItem::onDetach(item);
        adjustNodeThreshold();
    }

    void CompactAndItem::buildDelayLines()
    {
        // frontwards -- goes from big to small
        _frontwardDelayLines.clear();
        for (int i = 0; i < _baseLinkItems.size(); ++i)
        {
            int num = (_baseLinkItems.size() - i) - 1;
            _frontwardDelayLines.append(buildDelayLine(num, -1, _frontwardTipCell));
        }

        // backwards -- goes from small to big
        _backwardDelayLines.clear();
        for (int i = 0; i < _baseLinkItems.size(); ++i)
        {
            int num = i;
            _backwardDelayLines.append(buildDelayLine(num, _backwardTipCell, -1));
        }
    }

    QList<NeuroCell::Index> CompactAndItem::buildDelayLine(int len, NeuroCell::Index in, NeuroCell::Index out)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        QList<NeuroCell::Index> line;

        NeuroCell::Index lastIndex = in;
        for (int i = 0; i < len; ++i)
        {
            for (int j = 0; j < _delay; ++j)
            {
                NeuroCell::Index newIndex = network()->neuronet()->addNode(NeuroCell(NeuroCell::EXCITORY_LINK));
                if (lastIndex != -1 && newIndex != -1)
                    network()->neuronet()->addEdge(newIndex, lastIndex);
                line.append(newIndex);
                lastIndex = newIndex;
            }
        }

        if (lastIndex != -1 && out != -1)
            network()->neuronet()->addEdge(out, lastIndex);

        return line;
    }

    void CompactAndItem::teardownDelayLines()
    {
        clearDelayLines(_frontwardDelayLines);
        clearDelayLines(_backwardDelayLines);
    }

    void CompactAndItem::clearDelayLines(QList<QList<NeuroCell::Index> > & delayLines)
    {
        for (QMutableListIterator< QList<NeuroCell::Index> > line(delayLines); line.hasNext(); )
            clearDelayLine(line.next());
        delayLines.clear();
    }

    void CompactAndItem::clearDelayLine(QList<NeuroCell::Index> & delayLine)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        foreach (NeuroCell::Index index, delayLine)
            network()->neuronet()->removeNode(index);
        delayLine.clear();
    }

    void CompactAndItem::adjustNodeThreshold()
    {
        NeuroNet::ASYNC_STATE *frontwardCell = getCell(_frontwardTipCell);
        if (frontwardCell)
        {
            frontwardCell->current().setWeight(_baseLinkItems.size());
            frontwardCell->former().setWeight(_baseLinkItems.size());
        }
    }

    void CompactAndItem::addToShape(QPainterPath &drawPath, QList<TextPathRec> &) const
    {
        qreal radius = getRadius();
        qreal to_tip = getTip();

        drawPath.moveTo(-radius, -to_tip);
        drawPath.lineTo(0, to_tip);
        drawPath.lineTo(radius, -to_tip);
        drawPath.lineTo(-radius, -to_tip);
    }

    void CompactAndItem::adjustLinks()
    {
        if (_sequential)
        {
            if (_tipLinkItem)
            {
                MixinArrow *link = dynamic_cast<MixinArrow *>(_tipLinkItem);
                if (link)
                {
                    QSet<MixinArrow *> already;
                    adjustLink(link, already);
                }
            }

            QSet<NeuroItem *> itemsToRemember;

            for (int i = 0; i < _baseLinkItems.size(); ++i)
            {
                MixinArrow *link = dynamic_cast<MixinArrow *>(_baseLinkItems[i]);
                if (link)
                {
                    itemsToRemember.insert(_baseLinkItems[i]);

                    QLineF line = link->line();
                    QPointF back = line.p1();
                    QPointF front = line.p2();
                    QPointF & point = link->frontLinkTarget() == this ? front : back;

                    qreal x = (scenePos().x() - getRadius()) + (2.0f * getRadius() / (_baseLinkItems.size()+1)) * (i+1);

                    point.setX(x);
                    point.setY(scenePos().y() - getTip());

                    link->setLine(back, front);
                }
            }

            rememberItems(itemsToRemember, QVector2D(scenePos()));
        }
        else
        {
            MixinRemember::adjustLinks();
        }
    }

    QVector2D CompactAndItem::getAttachPos(const QVector2D & pos)
    {
        qreal x = 0;
        qreal y = posOnTip(pos.toPointF()) ? getTip() : -getTip();

        return QVector2D(x, y);
    }

    void CompactAndItem::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        CompactNodeItem::writeBinary(ds, file_version);

        ds << _sequential;
        ds << _delay;
        writeDelayLines(ds, file_version, _frontwardDelayLines);
        writeDelayLines(ds, file_version, _backwardDelayLines);
    }

    void CompactAndItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        CompactNodeItem::readBinary(ds, file_version);

        ds >> _sequential;
        ds >> _delay;

        _delay_property.setEditable(_sequential);

        readDelayLines(ds, file_version, _frontwardDelayLines);
        readDelayLines(ds, file_version, _backwardDelayLines);
    }

    void CompactAndItem::writeDelayLines(QDataStream &ds, const NeuroLabFileVersion &,
                                         const QList<QList<NeuroCell::Index> > &delayLines) const
    {
        qint32 num = delayLines.size();
        ds << num;

        for (int i = 0; i < num; ++i)
        {
            qint32 num2 = delayLines[i].size();
            ds << num2;

            for (int j = 0; j < num2; ++j)
                ds << static_cast<qint32>(delayLines[i][j]);
        }
    }

    void CompactAndItem::readDelayLines(QDataStream &ds, const NeuroLabFileVersion &, QList<QList<NeuroCell::Index> > &delayLines)
    {
        qint32 num;

        delayLines.clear();
        ds >> num;

        for (int i = 0; i < num; ++i)
        {
            QList<NeuroCell::Index> line;
            qint32 num2;
            ds >> num2;

            for (int j = 0; j < num2; ++j)
            {
                qint32 index;
                ds >> index;

                line.append(index);
            }

            delayLines.append(line);
        }
    }

    //////////////////////////////////////////////////////////////////

    NEUROITEM_DEFINE_CREATOR(CompactUpwardAndItem, QObject::tr("Abstract"), QObject::tr("AND Node (Upward)"));
    NEUROITEM_DEFINE_CREATOR(CompactDownwardAndItem, QObject::tr("Abstract"), QObject::tr("AND Node (Downward)"));

} // namespace NeuroGui
