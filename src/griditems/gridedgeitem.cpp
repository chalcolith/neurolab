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

#include "gridedgeitem.h"
#include "neurogriditem.h"
#include "../neurogui/labview.h"
#include "../neurogui/labscene.h"
#include "../neurogui/labnetwork.h"

#include <QScrollBar>

using namespace NeuroGui;

namespace GridItems
{

    NEUROITEM_DEFINE_RESTRICTED_PLUGIN_CREATOR(GridEdgeItem, QObject::tr("Grid Items"), QObject::tr("Edge Connector"), ":/griditems/icons/grid_connector.png", GridItems::VERSION(), NeuroGridItem)

    static const int GAP = 2;
    static const int LONG = NeuroItem::NODE_WIDTH/2;
    static const int SHORT = NeuroItem::NODE_WIDTH/3;

    GridEdgeItem::GridEdgeItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroNetworkItem(network, scenePos, context), MixinRemember(this),
          _vertical(false), _adjustLinksNextUpdate(true)
    {
        if (context == NeuroItem::CREATE_UI)
        {
            // should we be horizontal or vertical?
            const QPointF mousePos = scenePos;
            const QRectF sceneRect = network->view()->rect();

            qreal min_x = 1.0e9;
            if (mousePos.x() - sceneRect.left() < min_x)
                min_x = mousePos.x() - sceneRect.left();
            if (sceneRect.right() - mousePos.x() < min_x)
                min_x = sceneRect.right() - mousePos.x();

            qreal min_y = 1.0e9;
            if (mousePos.y() - sceneRect.top() < min_y)
                min_y = mousePos.y() - sceneRect.top();
            if (sceneRect.bottom() - mousePos.y() < min_y)
                min_y = sceneRect.bottom() - mousePos.y();

            _vertical = min_y < min_x;

            QPointF movePos = scenePos;
            if (_vertical)
                movePos.setY((sceneRect.top() + sceneRect.bottom()) / 2.0);
            else
                movePos.setX((sceneRect.left() + sceneRect.right()) / 2.0);
            setPos(movePos);
        }

        if (network->view())
        {
            connect(network->view(), SIGNAL(viewResized()), this, SLOT(viewResized()));
        }
    }

    GridEdgeItem::~GridEdgeItem()
    {
    }

    void GridEdgeItem::viewResized()
    {
        _adjustLinksNextUpdate = true;
        setChanged(true);
    }

    bool GridEdgeItem::handleMove(const QPointF & mousePos, QPointF & movePos)
    {
        movePos = scenePos();

        if (_vertical)
            movePos.setX(mousePos.x());
        else
            movePos.setY(mousePos.y());

        return NeuroNetworkItem::handleMove(mousePos, movePos);
    }

    bool GridEdgeItem::canBeAttachedBy(const QPointF &, NeuroItem *item) const
    {
        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        return link != 0;
    }

    bool GridEdgeItem::canBeAttachedToTwice(NeuroItem *) const
    {
        return true;
    }

    QVector2D GridEdgeItem::getAttachPos(MixinArrow *link, const QVector2D & pos)
    {
        // all vectors are in my frame

        QVector2D myPoint1(mapFromScene(_point1.toPointF()));
        QVector2D myPoint2(mapFromScene(_point2.toPointF()));

        QVector2D fromPoint1 = pos - myPoint1;
        QVector2D fromPoint2 = pos - myPoint2;

        if (fromPoint1.lengthSquared() < fromPoint2.lengthSquared())
        {
            return myPoint1 + fromPoint1.normalized() * (LONG/2);
        }
        else
        {
            return myPoint2 + fromPoint2.normalized() * (LONG/2);
        }
    }

    void GridEdgeItem::onAttachedBy(NeuroItem *item)
    {
        NeuroNetworkItem::onAttachedBy(item);

        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        if (link)
        {
            MixinRemember::onAttachedBy(link);

            if (link->frontLinkTarget() == this && link->backLinkTarget() == this)
            {
                _connections1.insert(item);
                _connections2.insert(item);
            }
            else
            {
                bool attached = true;
                QVector2D attachPt;
                if (_incomingAttachments.contains(link))
                    attachPt = QVector2D(mapToScene(_incomingAttachments[link].toPointF()));
                else if (_outgoingAttachments.contains(link))
                    attachPt = QVector2D(mapToScene(_outgoingAttachments[link].toPointF()));
                else
                    attached = false;

                if (attached)
                {
                    qreal dist1 = (_point1 - attachPt).lengthSquared();
                    qreal dist2 = (_point2 - attachPt).lengthSquared();

                    if (dist1 < dist2)
                        _connections1.insert(item);
                    else
                        _connections2.insert(item);
                }
            }
        }
    }

    void GridEdgeItem::onDetach(NeuroItem *item)
    {
        _connections1.remove(item);
        _connections2.remove(item);

        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        if (link)
            MixinRemember::onDetach(link);

        NeuroNetworkItem::onDetach(item);
    }

    QPointF GridEdgeItem::targetPointFor(const NeuroItem *item, bool front) const
    {
        const MixinArrow *link = dynamic_cast<const MixinArrow *>(item);
        if (link)
        {
            QPointF connect_pos = front ? link->line().p2() : link->line().p1();

            QVector2D to_pt1 = _point1 - QVector2D(connect_pos);
            QVector2D to_pt2 = _point2 - QVector2D(connect_pos);

            if (to_pt1.lengthSquared() < to_pt2.lengthSquared())
                return _point1.toPointF();
            else
                return _point2.toPointF();
        }
        else
        {
            return scenePos();
        }
    }

    void GridEdgeItem::addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const
    {
        NeuroNetworkItem::addToShape(drawPath, texts);

        // don't bother when we are not visible
        if (this->scene() != this->network()->scene())
            return;

        const QRectF sceneRect = this->network()->view()->sceneRect();
        const QRect viewRect = this->network()->view()->rect();

        // workaround for a bug in Qt where the scene rect leaves space for scroll bars
        qreal horiz_fudge = static_cast<qreal>(viewRect.width()) - sceneRect.width() - 1;
        if (this->network()->view()->verticalScrollBar()->isVisible())
            horiz_fudge -= this->network()->view()->verticalScrollBar()->width();

        qreal vert_fudge = static_cast<qreal>(viewRect.height()) - sceneRect.height() - 1;
        if (this->network()->view()->horizontalScrollBar()->isVisible())
            vert_fudge -= this->network()->view()->horizontalScrollBar()->height();

        if (_vertical)
        {
            QVector2D sceneTop(scenePos().x(), sceneRect.top() + GAP);
            QVector2D sceneBottom(scenePos().x(), sceneRect.bottom() + vert_fudge - GAP);
            QVector2D newCenter = (sceneTop + sceneBottom) * 0.5;

            GridEdgeItem *gei = const_cast<GridEdgeItem *>(this);
            gei->setPos(newCenter.toPointF());

            // top
            QPointF top = mapFromScene(scenePos().x(), sceneRect.top() + GAP);
            QRectF bound(top.x() - SHORT/2, top.y() - LONG/2, SHORT, LONG);

            drawPath.moveTo(top);
            drawPath.arcTo(bound, 180, 180);
            drawPath.closeSubpath();

            // bottom
            QPointF bot = mapFromScene(scenePos().x(), sceneRect.bottom() + vert_fudge - GAP);

            bound = QRectF(bot.x() - SHORT/2, bot.y() - LONG/2, SHORT, LONG);

            drawPath.moveTo(bot);
            drawPath.arcTo(bound, 0, 180);
            drawPath.closeSubpath();

            _point1 = QVector2D(mapToScene(top));
            _point2 = QVector2D(mapToScene(bot));
        }
        else
        {
            QVector2D sceneTop(sceneRect.left() + GAP, scenePos().y());
            QVector2D sceneBottom(sceneRect.right() + horiz_fudge - GAP, scenePos().y());
            QVector2D newCenter = (sceneTop + sceneBottom) * 0.5;

            GridEdgeItem *gei = const_cast<GridEdgeItem *>(this);
            gei->setPos(newCenter.toPointF());

            // left
            QPointF left = mapFromScene(sceneRect.left() + GAP, scenePos().y());
            QRectF bound(left.x() - LONG/2, left.y() - SHORT/2, LONG, SHORT);

            drawPath.moveTo(left);
            drawPath.arcTo(bound, 270, 180);
            drawPath.closeSubpath();

            // right
            QPointF right = mapFromScene(sceneRect.right() + horiz_fudge - GAP, scenePos().y());
            bound = QRectF(right.x() - LONG/2, right.y() - SHORT/2, LONG, SHORT);

            drawPath.moveTo(right);
            drawPath.arcTo(bound, 90, 180);
            drawPath.closeSubpath();

            _point1 = QVector2D(mapToScene(left));
            _point2 = QVector2D(mapToScene(right));
        }

        if (_adjustLinksNextUpdate)
        {
            GridEdgeItem *thisItem = const_cast<GridEdgeItem *>(this);
            thisItem->adjustLinks();
            _adjustLinksNextUpdate = false;
        }
    }

    bool GridEdgeItem::isConnectedToTop(const NeuroItem *item) const
    {
        return _vertical && _connections1.contains(const_cast<NeuroItem *>(item));
    }

    bool GridEdgeItem::isConnectedToBottom(const NeuroItem *item) const
    {
        return _vertical && _connections2.contains(const_cast<NeuroItem *>(item));
    }

    bool GridEdgeItem::isConnectedToLeft(const NeuroItem *item) const
    {
        return !_vertical && _connections1.contains(const_cast<NeuroItem *>(item));
    }

    bool GridEdgeItem::isConnectedToRight(const NeuroItem *item) const
    {
        return !_vertical && _connections2.contains(const_cast<NeuroItem *>(item));
    }

    void GridEdgeItem::adjustLinks()
    {
        MixinRemember::adjustLinks();
    }

    void GridEdgeItem::adjustLink(MixinArrow *link, QSet<MixinArrow *> &alreadyAdjusted)
    {
        LabScene *lab_scene = dynamic_cast<LabScene *>(this->scene());
        Q_ASSERT(lab_scene);

        bool frontLink = link->frontLinkTarget() == this;
        bool backLink = link->backLinkTarget() == this;

        bool rememberFront = _incomingAttachments.contains(link);
        bool rememberBack = _outgoingAttachments.contains(link);

        QLineF link_line = link->line();
        QVector2D front(this->mapFromScene(link_line.p2())); // link's front in my frame
        QVector2D back(this->mapFromScene(link_line.p1())); // link's back in my frame

        QVector2D toPos = QVector2D(this->mapFromScene(lab_scene->lastMousePos())); // in my frame

        if (frontLink)
        {
            if (rememberFront)
                toPos = _incomingAttachments[link];

            front = getAttachPos(link, toPos);
            _incomingAttachments[link] = front;
        }

        if (backLink)
        {
            if (rememberBack)
                toPos = _outgoingAttachments[link];

            back = getAttachPos(link, toPos);
            _outgoingAttachments[link] = back;
        }

        if (frontLink && backLink)
        {
            if (_vertical)
            {
                back.setX(0);
                front.setX(0);
            }
            else
            {
                back.setY(0);
                front.setY(0);
            }
        }

        link->setLine(this->mapToScene(back.toPointF()), this->mapToScene(front.toPointF()));
        alreadyAdjusted.insert(link);
    }

    void GridEdgeItem::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        NeuroNetworkItem::writeBinary(ds, file_version);

        ds << _vertical;
        ds << _point1;
        ds << _point2;
    }

    void GridEdgeItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        NeuroNetworkItem::readBinary(ds, file_version);

        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_12)
        {

        }

        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_10)
        {
            ds >> _vertical;
            ds >> _point1;
            ds >> _point2;
        }
    }

    void GridEdgeItem::writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        NeuroNetworkItem::writePointerIds(ds, file_version);

        quint32 num = _connections1.size();
        ds << num;

        foreach (const NeuroItem *item, _connections1)
        {
            IdType id = item->id();
            ds << id;
        }

        num = _connections2.size();
        ds << num;

        foreach (const NeuroItem *item, _connections2)
        {
            IdType id = item->id();
            ds << id;
        }
    }

    void GridEdgeItem::readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        NeuroNetworkItem::readPointerIds(ds, file_version);

        quint32 num;

        _connections1.clear();
        ds >> num;
        for (quint32 i = 0; i < num; ++i)
        {
            IdType id;
            ds >> id;
            _connections1.insert(reinterpret_cast<NeuroItem *>(id));
        }

        _connections2.clear();
        ds >> num;
        for (quint32 i = 0; i < num; ++i)
        {
            IdType id;
            ds >> id;
            _connections2.insert(reinterpret_cast<NeuroItem *>(id));
        }
    }

    void GridEdgeItem::idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap)
    {
        NeuroNetworkItem::idsToPointers(idMap);

        QSet<NeuroItem *> toAdd;
        foreach (NeuroItem *item, _connections1)
        {
            IdType wanted_id = static_cast<NeuroItem::IdType>(reinterpret_cast<quint64>(item));
            NeuroItem *wanted_item = idMap[wanted_id];
            if (wanted_item)
                toAdd.insert(wanted_item);
            else
                throw Common::FileFormatError(tr("Dangling connection node in file: %1").arg(wanted_id));
        }
        _connections1 = toAdd;

        toAdd.clear();
        foreach (NeuroItem *item, _connections2)
        {
            IdType wanted_id = static_cast<NeuroItem::IdType>(reinterpret_cast<quint64>(item));
            NeuroItem *wanted_item = idMap[wanted_id];
            if (wanted_item)
                toAdd.insert(wanted_item);
            else
                throw Common::FileFormatError(tr("Dangling connection node in file: %1").arg(wanted_id));
        }
        _connections2 = toAdd;
    }

    void GridEdgeItem::postLoad()
    {
        QVector2D center(scenePos());
        rememberItems(connections(), center);
    }

} // namespace GridItems
