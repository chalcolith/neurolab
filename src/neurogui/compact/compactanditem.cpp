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
        _sequence(false),
        _sequence_property(this, &CompactAndItem::sequence, &CompactAndItem::setSequence,
                           tr("Sequential"), tr("Whether or not the node will sequence its inputs/outputs in time."))
    {
    }

    CompactAndItem::~CompactAndItem()
    {
        if (_ui_delete)
        {
            teardownDelayLines();
        }
    }

    void CompactAndItem::setSequence(const bool &seq)
    {
        if (seq != _sequence)
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

            _sequence = seq;

            // re-connect
            foreach (NeuroItem *ni, connections())
                addEdges(ni);
        }
    }

    NeuroCell::Index CompactAndItem::getIncomingCellFor(const NeuroItem *item) const
    {
        // TODO: get delay cells
        if (item == _tipLinkItem)
            return _backwardTipCell;
        else
            return _frontwardTipCell;
    }

    NeuroCell::Index CompactAndItem::getOutgoingCellFor(const NeuroItem *item) const
    {
        // TODO: get delay cells
        if (item == _tipLinkItem)
            return _frontwardTipCell;
        else
            return _backwardTipCell;
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

        CompactNodeItem::onAttachedBy(item);

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

        // add delay lines for sequence node
        // frontwards has to add on the beginning of all of them
        for (int i = 0; i < _frontwardDelayLines.size(); ++i)
        {
            NeuroCell::Index prevIndex = _frontwardDelayLines[i].size() > 0 ? _frontwardDelayLines[i].first() : _frontwardTipCell;
            NeuroCell::Index newIndex = network()->neuronet()->addNode(NeuroCell(NeuroCell::EXCITORY_LINK));
            if (prevIndex != -1 && newIndex != -1)
                network()->neuronet()->addEdge(prevIndex, newIndex);
            _frontwardDelayLines[i].insert(0, newIndex);
        }
        _frontwardDelayLines.append(buildDelayLine(0, -1, _frontwardTipCell));

        // backwards just add one on the end
        _backwardDelayLines.append(buildDelayLine(_baseLinkItems.size()-1, _backwardTipCell, -1));

        // connect
        addEdges(item);
    }

    void CompactAndItem::onDetach(NeuroItem *item)
    {
        // disconnect
        removeEdges(item);

        // TODO: remove delay line


        //
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
            NeuroCell::Index newIndex = network()->neuronet()->addNode(NeuroCell(NeuroCell::EXCITORY_LINK));
            if (lastIndex != -1 && newIndex != -1)
                network()->neuronet()->addEdge(newIndex, lastIndex);
            line.append(newIndex);
            lastIndex = newIndex;
        }

        if (lastIndex != -1 && out != -1)
            network()->neuronet()->addEdge(out, lastIndex);

        return line;
    }

    void CompactAndItem::teardownDelayLines()
    {
        deleteDelayLines(_frontwardDelayLines);
        deleteDelayLines(_backwardDelayLines);
    }

    void CompactAndItem::deleteDelayLines(QList<QList<NeuroCell::Index> > & delayLines)
    {
        foreach (QList<NeuroCell::Index> & line, delayLines)
            deleteDelayLine(line);
        delayLines.clear();
    }

    void CompactAndItem::deleteDelayLine(QList<NeuroCell::Index> & delayLine)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        foreach (NeuroCell::Index index, delayLine)
            network()->neuronet()->removeNode(index);
        line.clear();
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

    void CompactAndItem::addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const
    {
        qreal radius = getRadius();
        qreal to_tip = getTip();

        drawPath.moveTo(-radius, -to_tip);
        drawPath.lineTo(0, to_tip);
        drawPath.lineTo(radius, -to_tip);
        drawPath.lineTo(-radius, -to_tip);
    }

    QVector2D CompactAndItem::getAttachPos(const QVector2D &dirTo)
    {
        if (dirTo.y() <= 0)
        {
            return _direction == DOWNWARD ? QVector2D(0, getTip()) : QVector2D(0, -getTip());
        }
        else
        {
            return _direction == DOWNWARD ? QVector2D(0, -getTip()) : QVector2D(0, getTip());
        }
    }

    //

    NEUROITEM_DEFINE_CREATOR(CompactUpwardAndItem, QObject::tr("Abstract|AND Node (Upward)"));
    NEUROITEM_DEFINE_CREATOR(CompactDownwardAndItem, QObject::tr("Abstract|AND Node (Downward)"));

} // namespace NeuroGui
