/*
Neurocognitive Linguistics Lab
Copyright (c) 2010,2011 Gordon Tisher
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

#include "compactoritem.h"
#include "../labnetwork.h"
#include "../labscene.h"

using namespace NeuroLib;

namespace NeuroGui
{

    CompactOrItem::CompactOrItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : CompactNodeItem(network, scenePos, context),
        _shortcut(true)
        //_shortcut_property(this, &CompactOrItem::shortcut, &CompactOrItem::setShortcut,
        //                   tr("Shortcut"), tr("Whether or not this OR node can be used for choosing alternatives."))
    {
        Q_ASSERT(network);

        //_shortcut_property.setEditable(false);
        connect(network, SIGNAL(postStep()), this, SLOT(postStep()));
    }

    CompactOrItem::~CompactOrItem()
    {
    }

    NeuroCell::Index CompactOrItem::getIncomingCellFor(const NeuroItem *item) const
    {
        if (item == _tipLinkItem)
            return _backwardTipCell;
        else
            return _frontwardTipCell;
    }

    NeuroCell::Index CompactOrItem::getOutgoingCellFor(const NeuroItem *item) const
    {
        if (item == _tipLinkItem)
            return _frontwardTipCell;
        else
            return _backwardTipCell;
    }

    QPointF CompactOrItem::targetPointFor(const NeuroItem *item) const
    {
        const MixinArrow *link;

        if (_shortcutItems.contains(const_cast<NeuroItem *>(item)) && (link = dynamic_cast<const MixinArrow *>(item)))
        {
            if (link->frontLinkTarget() == this)
                return QPointF(link->line().p2().x(), scenePos().y() + getTip());
            else
                return QPointF(link->line().p1().x(), scenePos().y() + getTip());
        }
        else
        {
            return scenePos();
        }
    }

    void CompactOrItem::postStep()
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        // check the output of all our shortcut links
        foreach (NeuroItem *ni, _shortcutItems)
        {
            // make sure the item is a link and a network item
            MixinArrow *link = dynamic_cast<MixinArrow *>(ni);
            if (!link)
                continue;

            NeuroNetworkItem *netItem = dynamic_cast<NeuroNetworkItem *>(ni);
            if (!netItem)
                continue;

            // get link target
            NeuroItem *linkTarget = link->frontLinkTarget() == this ? link->backLinkTarget() : link->frontLinkTarget();
            NeuroNetworkItem *netTarget = dynamic_cast<NeuroNetworkItem *>(linkTarget);
            if (!netTarget)
                continue;

            // if the link's output is not activated, forget about it
            NeuroCell::Index linkOutgoingIndex = netItem->getOutgoingCellFor(linkTarget);
            NeuroNet::ASYNC_STATE *linkOutgoingCell = getCell(linkOutgoingIndex);
            if (!linkOutgoingCell || linkOutgoingCell->current().outputValue() < NeuroCell::EPSILON)
                continue;

            // otherwise, get its incoming cell
            NeuroCell::Index targetCellIndex = netTarget->getIncomingCellFor(ni);
            if (targetCellIndex == -1)
                continue;

            NeuroNet::ASYNC_STATE *targetCell = getCell(targetCellIndex);
            if (!targetCell)
                continue;

            // copy the link target and all its neighbors to our little network
            _futureNetwork.clear();
            _futureNetwork.setDecay(network()->neuronet()->decay());
            _futureNetwork.setLinkLearnRate(network()->neuronet()->linkLearnRate());
            _futureNetwork.setNodeLearnRate(network()->neuronet()->nodeLearnRate());
            _futureNetwork.setNodeForgetRate(network()->neuronet()->nodeForgetRate());
            _futureNetwork.setLearnTime(network()->neuronet()->learnTime());

            NeuroCell::Index targetCellCopyIndex = _futureNetwork.addNode(targetCell->current());

            int num_neighbors;
            const NeuroCell::Index *neighbors = network()->neuronet()->neighbors(targetCellIndex, num_neighbors);
            for (int i = 0; i < num_neighbors; ++i)
            {
                NeuroNet::ASYNC_STATE *inputCell = getCell(neighbors[i]);
                if (inputCell)
                {
                    NeuroCell::Index inputCellCopyIndex = _futureNetwork.addNode(inputCell->current());
                    _futureNetwork.addEdge(targetCellCopyIndex, inputCellCopyIndex);
                }
            }

            // run the little network forward one step (three updates)
            _futureNetwork.stepInThread();
            _futureNetwork.stepInThread();
            _futureNetwork.stepInThread();

            // get the value of the target cell in the little network
            NeuroNet::ASYNC_STATE & future_cell = _futureNetwork[targetCellCopyIndex];
            NeuroCell::Value futureValue = future_cell.current().outputValue();
            if (futureValue > NeuroCell::EPSILON)
            {
                // inhibit central links
                foreach (NeuroItem *baseItem, _baseLinkItems)
                {
                    if (_shortcutItems.contains(baseItem))
                        continue;
                    NeuroNetworkItem *netBase = dynamic_cast<NeuroNetworkItem *>(baseItem);
                    if (!netBase)
                        continue;

                    NeuroCell::Value curValue = netBase->outputValue();
                    NeuroCell::Value newValue = 1.0f - qBound(0.0f, futureValue, 1.0f);
                    newValue = qMin(curValue, newValue);

                    // get base link's outgoing cell
                    MixinArrow *baseLink = dynamic_cast<MixinArrow *>(baseItem);
                    if (baseLink)
                    {
                        NeuroItem *baseTarget = baseLink->frontLinkTarget() == this ? baseLink->backLinkTarget() : baseLink->frontLinkTarget();
                        NeuroCell::Index baseOutgoingIndex = netBase->getOutgoingCellFor(baseTarget);
                        if (baseOutgoingIndex != -1)
                        {
                            NeuroNet::ASYNC_STATE *baseCell = &(*network()->neuronet())[baseOutgoingIndex];
                            baseCell->current().setOutputValue(newValue);
                            baseCell->former().setOutputValue(newValue);
                        }
                    }
                }
            }
            else
            {
                // inhibit this link's output
                linkOutgoingCell->current().setOutputValue(0);
                linkOutgoingCell->former().setOutputValue(0);
            }
        }
    }

    bool CompactOrItem::canBeAttachedBy(const QPointF & pos, NeuroItem *item) const
    {
        if (dynamic_cast<MixinArrow *>(item) && item != _tipLinkItem)
        {
            if (scenePosOnTip(pos))
                return _tipLinkItem == 0;
            else
                return !_baseLinkItems.contains(item);
        }

        return false;
    }

    void CompactOrItem::onAttachedBy(NeuroItem *item)
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
        }

        CompactNodeItem::onAttachedBy(item);
        addEdges(item);

        // add to shortcut nodes
        if (link)
        {
            QPointF attachPos = link->frontLinkTarget() == this ? link->line().p2() : link->line().p1();
            qreal diff = qAbs(attachPos.x() - scenePos().x());
            qreal bound = getRadius() * 0.5f;

            if (diff >= bound)
                _shortcutItems.insert(item);
        }
    }

    void CompactOrItem::onDetach(NeuroItem *item)
    {
        _shortcutItems.remove(item);

        removeEdges(item);
        CompactNodeItem::onDetach(item);
    }

    void CompactOrItem::addToShape(QPainterPath &drawPath, QList<TextPathRec> &) const
    {
        qreal radius = getRadius();
        qreal toTip = getTip();

        drawPath.moveTo(-radius, -toTip);
        drawPath.lineTo(-radius, 0);
        drawPath.lineTo(radius, 0);
        drawPath.lineTo(radius, -toTip);
    }

    void CompactOrItem::setBrushProperties(QBrush &brush) const
    {
        brush.setStyle(Qt::NoBrush);
    }

    void CompactOrItem::adjustLinks()
    {
        MixinRemember::adjustLinks();
    }

    QVector2D CompactOrItem::getAttachPos(const QVector2D & pos)
    {
        qreal x = 0, y;

        if (pos.y() <= 0)
            y = -2;
        else
            y = 2;

        if (!posOnTip(pos.toPointF()))
        {
            qreal radius = getRadius();
            if (qAbs(pos.x()) > radius/3.0f)
                x = radius * qBound(-1.0, pos.x(), 1.0) * 0.5f;
        }

        return QVector2D(x, y);
    }

    void CompactOrItem::writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const
    {
        CompactNodeItem::writeClipboard(ds, id_map);

        ds << static_cast<qint32>(_shortcutItems.size());
        foreach (NeuroItem *ni, _shortcutItems)
        {
            qint32 id = ni->id();

            if (id && id_map.contains(id))
                ds << static_cast<qint32>(id_map[id]);
            else
                ds << static_cast<qint32>(0);
        }
    }

    void CompactOrItem::readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> &id_map)
    {
        CompactNodeItem::readClipboard(ds, id_map);

        qint32 num;
        ds >> num;

        _shortcutItems.clear();
        for (qint32 i = 0; i < num; ++i)
        {
            qint32 id;
            ds >> id;

            if (id && id_map.contains(id))
                _shortcutItems.insert(id_map[id]);
        }
    }

    void CompactOrItem::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        CompactNodeItem::writeBinary(ds, file_version);

        ds << _shortcut;
    }

    void CompactOrItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        CompactNodeItem::readBinary(ds, file_version);

        ds >> _shortcut;
    }

    void CompactOrItem::writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        CompactNodeItem::writePointerIds(ds, file_version);

        ds << static_cast<qint32>(_shortcutItems.size());
        foreach (NeuroItem *ni, _shortcutItems)
        {
            ds << static_cast<NeuroItem::IdType>(ni->id());
        }
    }

    void CompactOrItem::readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        CompactNodeItem::readPointerIds(ds, file_version);

        qint32 num;
        ds >> num;

        _shortcutItems.clear();
        for (qint32 i = 0; i < num; ++i)
        {
            NeuroItem::IdType id;
            ds >> id;

            if (id)
                _shortcutItems.insert(reinterpret_cast<NeuroItem *>(id));
        }
    }

    void CompactOrItem::idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap)
    {
        CompactNodeItem::idsToPointers(idMap);

        QSet<NeuroItem *> itemsToAdd;
        foreach (NeuroItem *ni, _shortcutItems)
        {
            NeuroItem::IdType wanted_id = reinterpret_cast<NeuroItem::IdType>(ni);
            NeuroItem *wanted_item = idMap[wanted_id];

            if (wanted_item)
                itemsToAdd.insert(wanted_item);
            else
                throw Common::Exception(tr("Dangling node ID in file: %1").arg(wanted_id));
        }

        _shortcutItems = itemsToAdd;
    }


    //

    NEUROITEM_DEFINE_CREATOR(CompactUpwardOrItem, QObject::tr("Abstract"), QObject::tr("OR Node (Upward)"));
    NEUROITEM_DEFINE_CREATOR(CompactDownwardOrItem, QObject::tr("Abstract"), QObject::tr("OR Node (Downward)"));

} // namespace NeuroGui
