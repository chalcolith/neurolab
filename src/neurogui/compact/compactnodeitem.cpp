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

#include "compactnodeitem.h"
#include "../labnetwork.h"

using namespace NeuroLib;

namespace NeuroGui
{

    CompactNodeItem::CompactNodeItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : CompactItem(network, scenePos, context), MixinRemember(this),
        _tipLinkItem(0), _frontwardTipCell(-1), _backwardTipCell(-1),
        _direction_property(this, &CompactNodeItem::direction, &CompactNodeItem::setDirection,
                            tr("Direction"), tr("The direction in which the multiple-link side of the node is facing."))
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
            return p.y() <= 0;
        else
            return p.y() > 0;
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
        pen.setWidth(pen.width() * 2);
    }

    void CompactNodeItem::setBrushProperties(QBrush &brush) const
    {
        CompactItem::setBrushProperties(brush);
        brush.setStyle(Qt::NoBrush);
    }

    void CompactNodeItem::postLoad()
    {
        QVector2D center(scenePos());
        rememberItems(connections(), center);
    }

} // namespace NeuroGui
