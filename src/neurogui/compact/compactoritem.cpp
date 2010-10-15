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

#include "compactoritem.h"
#include "../labnetwork.h"
#include "../labscene.h"

using namespace NeuroLib;

namespace NeuroGui
{

    CompactOrItem::CompactOrItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : CompactNodeItem(network, scenePos, context),
        _shortcut(false)
        //_shortcut_property(this, &CompactOrItem::shortcut, &CompactOrItem::setShortcut,
        //                   tr("Shortcut"), tr("Whether or not this OR node can be used for choosing alternatives."))
    {
        Q_ASSERT(network);

        //_shortcut_property.setEditable(false);

        connect(network, SIGNAL(preStep()), this, SLOT(preStep()));
        connect(network, SIGNAL(postStep()), this, SLOT(postStep()));
    }

    CompactOrItem::~CompactOrItem()
    {
        Q_ASSERT(network());

        disconnect(network(), SIGNAL(preStep()), this, SLOT(preStep()));
        disconnect(network(), SIGNAL(postStep()), this, SLOT(postStep()));
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

    void CompactOrItem::preStep()
    {
    }

    void CompactOrItem::postStep()
    {
    }

    bool CompactOrItem::canBeAttachedBy(const QPointF & pos, NeuroItem *item)
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

        // TODO: set up shortcut stuff

        addEdges(item);
    }

    void CompactOrItem::onDetach(NeuroItem *item)
    {
        // TODO: clean up shortcut stuff

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
            y = -1;
        else
            y = 1;

        if (posOnTip(pos.toPointF()))
        {
            qreal radius = getRadius();
            if (qAbs(pos.x()) > radius/3.0f)
                x = qBound(-1.0, pos.x(), 1.0) * 2.0f / 3.0f;
        }

        return QVector2D(x, y);
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


    //

    NEUROITEM_DEFINE_CREATOR(CompactUpwardOrItem, QObject::tr("Abstract"), QObject::tr("OR Node (Upward)"));
    NEUROITEM_DEFINE_CREATOR(CompactDownwardOrItem, QObject::tr("Abstract"), QObject::tr("OR Node (Downward)"));

} // namespace NeuroGui
