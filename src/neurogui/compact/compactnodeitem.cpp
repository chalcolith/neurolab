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

#include "compactnodeitem.h"
#include "../labnetwork.h"

using namespace NeuroLib;

namespace NeuroGui
{

    CompactNodeItem::CompactNodeItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : CompactItem(network, scenePos, context), MixinRemember(this),
        _direction(DOWNWARD), _tipLinkItem(0), _frontwardTipCell(-1), _backwardTipCell(-1)
    {
        this->_value_property.setEditable(false);

        Q_ASSERT(network);
        Q_ASSERT(network->neuronet());

        if (context == NeuroItem::CREATE_UI)
        {
            _frontwardTipCell = network->neuronet()->addNode(NeuroCell(NeuroCell::NODE));
            _backwardTipCell = network->neuronet()->addNode(NeuroCell(NeuroCell::NODE));
        }
    }

    CompactNodeItem::~CompactNodeItem()
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        if (_ui_delete)
        {
            if (_frontwardTipCell != -1)
                network()->neuronet()->removeNode(_frontwardTipCell);
            if (_backwardTipCell != -1)
                network()->neuronet()->removeNode(_backwardTipCell);
        }
    }

    NeuroCell::Value CompactNodeItem::outputValue() const
    {
        const NeuroNet::ASYNC_STATE *frontwardCell = getCell(_frontwardTipCell);
        const NeuroNet::ASYNC_STATE *backwardCell = getCell(_backwardTipCell);

        if (frontwardCell && backwardCell)
            return qMax(frontwardCell->current().outputValue(), backwardCell->current().outputValue());

        return 0;
    }

    void CompactNodeItem::setOutputValue(const NeuroLib::NeuroCell::Value & val)
    {
        NeuroNet::ASYNC_STATE *frontwardCell = getCell(_frontwardTipCell);
        if (frontwardCell)
        {
            frontwardCell->current().setOutputValue(val);
            frontwardCell->former().setOutputValue(val);
        }

        NeuroNet::ASYNC_STATE *backwardCell = getCell(_backwardTipCell);
        if (backwardCell)
        {
            backwardCell->current().setOutputValue(val);
            backwardCell->former().setOutputValue(val);
        }
    }

    bool CompactNodeItem::posOnTip(const QPointF &p) const
    {
        if (_direction == UPWARD)
            return p.y() > 0;
        else
            return p.y() <= 0;
    }

    bool CompactNodeItem::scenePosOnTip(const QPointF &p) const
    {
        return posOnTip(mapFromScene(p));
    }

    void CompactNodeItem::onAttachedBy(NeuroItem *item)
    {
        CompactItem::onAttachedBy(item);

        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        if (link)
            MixinRemember::onAttachedBy(link);
    }

    void CompactNodeItem::onDetach(NeuroItem *item)
    {
        if (item == _tipLinkItem)
        {
            _tipLinkItem = 0;
        }
        else if (_baseLinkItems.contains(item))
        {
            _baseLinkItems.removeAll(item);
        }

        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        if (link)
            MixinRemember::onDetach(link);

        CompactItem::onDetach(item);
    }

    void CompactNodeItem::adjustLinks()
    {
        MixinRemember::adjustLinks();
    }

    void CompactNodeItem::setPenProperties(QPen &pen) const
    {
        CompactItem::setPenProperties(pen);

        if (pen.width() == NORMAL_LINE_WIDTH)
            pen.setWidth(pen.width() * 2);
    }

    void CompactNodeItem::setBrushProperties(QBrush &brush) const
    {
        CompactItem::setBrushProperties(brush);
        brush.setStyle(Qt::SolidPattern);
    }

    void CompactNodeItem::postLoad()
    {
        QVector2D center(scenePos());
        rememberItems(connections(), center);
    }

    void CompactNodeItem::writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const
    {
        CompactItem::writeClipboard(ds, id_map);
        MixinRemember::writeClipboard(ds, id_map);

        ds << static_cast<qint32>(_direction);

        if (_tipLinkItem && id_map.contains(_tipLinkItem->id()))
            ds << static_cast<qint32>(id_map[_tipLinkItem->id()]);
        else
            ds << static_cast<qint32>(0);

        ds << static_cast<qint32>(_baseLinkItems.size());
        foreach (NeuroItem *ni, _baseLinkItems)
        {
            if (ni && id_map.contains(ni->id()))
                ds << static_cast<qint32>(id_map[ni->id()]);
            else
                ds << static_cast<qint32>(0);
        }
    }

    void CompactNodeItem::readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> &id_map)
    {
        CompactItem::readClipboard(ds, id_map);
        MixinRemember::readClipboard(ds, id_map);

        qint32 dir;
        ds >> dir;
        _direction = static_cast<Direction>(dir);

        qint32 tip_id;
        ds >> tip_id;
        if (tip_id && id_map.contains(tip_id))
            _tipLinkItem = id_map[tip_id];

        qint32 num;
        ds >> num;
        _baseLinkItems.clear();
        for (qint32 i = 0; i < num; ++i)
        {
            qint32 id;
            ds >> id;
            if (id && id_map.contains(id))
                _baseLinkItems.append(id_map[id]);
        }
    }

    void CompactNodeItem::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        CompactItem::writeBinary(ds, file_version);

        ds << static_cast<quint8>(_direction);
        ds << static_cast<quint32>(_frontwardTipCell);
        ds << static_cast<quint32>(_backwardTipCell);
    }

    void CompactNodeItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        CompactItem::readBinary(ds, file_version);

        quint8 dir;
        ds >> dir;
        _direction = static_cast<Direction>(dir);

        quint32 index;
        ds >> index;
        _frontwardTipCell = static_cast<NeuroCell::Index>(index);

        ds >> index;
        _backwardTipCell = static_cast<NeuroCell::Index>(index);
    }

    void CompactNodeItem::writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        CompactItem::writePointerIds(ds, file_version);

        QList<NeuroItem *> items;
        items.append(_tipLinkItem);
        items.append(_baseLinkItems);

        ds << static_cast<qint32>(items.size());
        foreach (NeuroItem *ni, items)
        {
            ds << (ni ? static_cast<IdType>(ni->id()) : static_cast<IdType>(0));
        }
    }

    void CompactNodeItem::readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        CompactItem::readPointerIds(ds, file_version);

        qint32 num;
        ds >> num;

        _baseLinkItems.clear();
        for (int i = 0; i < num; ++i)
        {
            IdType id;
            ds >> id;

            if (i == 0)
                _tipLinkItem = reinterpret_cast<NeuroItem *>(id);
            else
                _baseLinkItems.append(reinterpret_cast<NeuroItem *>(id));
        }
    }

    void CompactNodeItem::idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap)
    {
        CompactItem::idsToPointers(idMap);

        IdType wanted_id = reinterpret_cast<IdType>(_tipLinkItem);
        _tipLinkItem = idMap[wanted_id]; // can be null

        QList<NeuroItem *> itemsToAdd;
        foreach (NeuroItem *ni, _baseLinkItems)
        {
            wanted_id = reinterpret_cast<IdType>(ni);
            NeuroItem *wanted_item = idMap[wanted_id];

            if (wanted_item)
                itemsToAdd.append(wanted_item);
            else
                throw Common::Exception(tr("Dangling node in compact node base: %1").arg(wanted_id));
        }

        _baseLinkItems = itemsToAdd;
    }

} // namespace NeuroGui
