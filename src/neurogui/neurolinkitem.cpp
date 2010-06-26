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

#include "neurolinkitem.h"
#include "labnetwork.h"
#include "labscene.h"
#include "mainwindow.h"
#include "../neurolib/neuronet.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QPainterPathStroker>
#include <QPolygonF>
#include <QVector2D>

#include <QtProperty>

#define _USE_MATH_DEFINES
#include <cmath>

using namespace NeuroLib;

namespace NeuroLab
{

    //////////////////////////////////////////////////////////////////

    NeuroLinkItem::NeuroLinkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroNarrowItem(network, scenePos, context),
        _weight_property(this, &NeuroLinkItem::weight, &NeuroLinkItem::setWeight,
                         tr("Weight"), tr("The output weight of the link.")),
        _frontLinkTarget(0), _backLinkTarget(0), dragFront(false), settingLine(false)
    {
        Q_ASSERT(network != 0);
    }

    NeuroLinkItem::~NeuroLinkItem()
    {
        _frontLinkTarget = 0;
        _backLinkTarget = 0;
    }

    NeuroCell::NeuroValue NeuroLinkItem::weight() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell();
        return cell ? cell->current().weight() : 0;
    }

    void NeuroLinkItem::setWeight(const NeuroLib::NeuroCell::NeuroValue & value)
    {
        NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            cell->current().setWeight(value);
            cell->former().setWeight(value);
        }
    }

    QLineF NeuroLinkItem::line() const
    {
        QVector2D center(pos());
        QVector2D p1 = QVector2D(_line.p1()) + center;
        QVector2D p2 = QVector2D(_line.p2()) + center;

        return QLineF(p1.toPointF(), p2.toPointF());
    }

    void NeuroLinkItem::setLine(const QLineF & l)
    {
        settingLine = true;
        prepareGeometryChange();

        QVector2D p1(l.p1());
        QVector2D p2(l.p2());
        QVector2D center = (p1 + p2) * 0.5f;

        _line = QLineF((p1 - center).toPointF(), (p2 - center).toPointF());
        setPos(center.toPointF());

        updateShape();
        adjustLinks();

        settingLine = false;
    }

    void NeuroLinkItem::setLine(const qreal & x1, const qreal & y1, const qreal & x2, const qreal & y2)
    {
        setLine(QLineF(x1, y1, x2, y2));
    }

    void NeuroLinkItem::setLine(const QPointF & p1, const QPointF & p2)
    {
        setLine(QLineF(p1, p2));
    }

    bool NeuroLinkItem::addIncoming(NeuroItem *linkItem)
    {
        return NeuroNarrowItem::addIncoming(linkItem);
    }

    bool NeuroLinkItem::removeIncoming(NeuroItem *linkItem)
    {
        if (linkItem && linkItem == _backLinkTarget)
            setBackLinkTarget(0);
        return NeuroNarrowItem::removeIncoming(linkItem);
    }

    bool NeuroLinkItem::addOutgoing(NeuroItem *linkItem)
    {
        return NeuroNarrowItem::addOutgoing(linkItem);
    }

    bool NeuroLinkItem::removeOutgoing(NeuroItem *linkItem)
    {
        if (linkItem && linkItem == _frontLinkTarget)
            setFrontLinkTarget(0);
        return NeuroNarrowItem::removeOutgoing(linkItem);
    }

    void NeuroLinkItem::setFrontLinkTarget(NeuroItem *linkTarget)
    {
        if (_frontLinkTarget)
            NeuroNarrowItem::removeOutgoing(_frontLinkTarget);

        if (linkTarget)
            addOutgoing(linkTarget);

        _frontLinkTarget = linkTarget;

        updateShape();
    }

    void NeuroLinkItem::setBackLinkTarget(NeuroItem *linkTarget)
    {
        if (_backLinkTarget)
            NeuroNarrowItem::removeIncoming(_backLinkTarget);

        if (linkTarget)
            addIncoming(linkTarget);

        _backLinkTarget = linkTarget;

        updateShape();
    }

    void NeuroLinkItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroNarrowItem::addToShape(drawPath, texts);

        QVector2D myPos(pos());

        // in my frame
        QVector2D myFront(_line.p2());
        QVector2D myBack(_line.p1());

        // calculate control points
        c1 = myBack + (myFront - myBack) * 0.33;
        c2 = myBack + (myFront - myBack) * 0.66;

        if (_backLinkTarget)
        {
            QVector2D center(_backLinkTarget->pos());
            QVector2D toBack = QVector2D(line().p1()) - center;
            c1 = (center + toBack * 3) - myPos;
        }

        if (_frontLinkTarget && !dynamic_cast<NeuroLinkItem *>(_frontLinkTarget))
        {
            QVector2D center(_frontLinkTarget->pos());
            QVector2D toFront = QVector2D(line().p2()) - center;
            c2 = (center + toFront * 3) - myPos;
        }

        QPointF front = myFront.toPointF();
        QPointF back = myBack.toPointF();

        drawPath.moveTo(back);
        drawPath.cubicTo(c1.toPointF(), c2.toPointF(), front);
    }

    void NeuroLinkItem::setPenProperties(QPen & pen) const
    {
        NeuroNarrowItem::setPenProperties(pen);

        const NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            qreal weight = qBound(0.1f, qAbs(cell->current().weight()), 1.0f);
            pen.setColor(lerp(Qt::lightGray, pen.color(), weight));
        }
    }

    void NeuroLinkItem::setBrushProperties(QBrush & brush) const
    {
        NeuroNarrowItem::setBrushProperties(brush);
        brush.setStyle(Qt::NoBrush);
    }

    void NeuroLinkItem::adjustLinks()
    {
        QVector2D front(line().p2());
        QVector2D back(line().p1());

        QVector2D center = (back + front) * 0.5f;

        for (QListIterator<NeuroItem *> i(incoming()); i.hasNext(); i.next())
        {
            NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(i.peekNext());
            if (link)
                link->setLine(link->line().p1(), center.toPointF());
        }
    }

    bool NeuroLinkItem::canAttachTo(const QPointF & pos, NeuroItem *item)
    {
        // cannot link the back of a link to another link
        if (dynamic_cast<NeuroLinkItem *>(item) && !dragFront)
            return false;
        return NeuroNarrowItem::canAttachTo(pos, item);
    }

    bool NeuroLinkItem::canBeAttachedBy(const QPointF &, NeuroItem *item)
    {
        // can only be attached to by an inhibitory link
        return dynamic_cast<NeuroInhibitoryLinkItem *>(item);
    }

    void NeuroLinkItem::attachTo(NeuroItem *item)
    {
        if (dragFront)
            setFrontLinkTarget(item);
        else
            setBackLinkTarget(item);
    }

    bool NeuroLinkItem::handleMove(const QPointF & mousePos, QPointF & movePos)
    {
        // break links
        NeuroItem *linkedItem = dragFront ? _frontLinkTarget : _backLinkTarget;
        if (linkedItem && !linkedItem->contains(linkedItem->mapFromScene(mousePos)))
        {
            if (dragFront)
                setFrontLinkTarget(0);
            else
                setBackLinkTarget(0);
        }

        // attach
        return NeuroNarrowItem::handleMove(mousePos, movePos);
    }

    QVariant NeuroLinkItem::itemChange(GraphicsItemChange change, const QVariant & value)
    {
        LabScene *labScene;

        switch (change)
        {
        case QGraphicsItem::ItemPositionChange:
            if ((labScene = dynamic_cast<LabScene *>(scene()))
                && labScene->selectedItems().size() <= 1
                && !settingLine)
            {
                // adjust position
                QVector2D front(line().p2());
                QVector2D back(line().p1());
                QVector2D mousePos(labScene->lastMousePos());

                qreal distFront = (mousePos - front).lengthSquared();
                qreal distBack = (mousePos - back).lengthSquared();

                dragFront = distFront < distBack;
                if (dragFront)
                    front = mousePos;
                else
                    back = mousePos;

                QVector2D newCenter = (front + back) * 0.5;

                // set new position and line
                setPos(newCenter.toPointF());
                _line = QLineF((back - newCenter).toPointF(), (front - newCenter).toPointF());

                // adjust if we're linked
                if (_frontLinkTarget)
                    _frontLinkTarget->adjustLinks();
                if (_backLinkTarget)
                    _backLinkTarget->adjustLinks();

                // handle move
                QPointF movePos(scenePos());

                handleMove(mousePos.toPointF(), movePos);
                adjustLinks();
                updateShape();

                return QVariant(movePos);
            }
            break;

        default:
            break;
        }

        return NeuroNarrowItem::itemChange(change, value);
    }

    void NeuroLinkItem::writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const
    {
        NeuroNarrowItem::writeClipboard(ds, id_map);
        ds << _line;
    }

    void NeuroLinkItem::readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> &id_map)
    {
        NeuroNarrowItem::readClipboard(ds, id_map);
        ds >> _line;
    }

    void NeuroLinkItem::writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const
    {
        NeuroNarrowItem::writeBinary(ds, file_version);
        ds << _line;
    }

    void NeuroLinkItem::readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version)
    {
        NeuroNarrowItem::readBinary(ds, file_version);

        if (file_version.neurolab_version >= NeuroLab::NEUROLAB_FILE_VERSION_OLD)
        {
            QLineF l;
            ds >> l;
            setLine(l);
        }
        else
        {
            throw new Automata::FileFormatError();
        }
    }

    void NeuroLinkItem::writePointerIds(QDataStream & ds, const NeuroLabFileVersion & file_version) const
    {
        NeuroNarrowItem::writePointerIds(ds, file_version);

        IdType id = _frontLinkTarget ? _frontLinkTarget->id() : 0;
        ds << id;

        id = _backLinkTarget ? _backLinkTarget->id() : 0;
        ds << id;
    }

    void NeuroLinkItem::readPointerIds(QDataStream & ds, const NeuroLabFileVersion & file_version)
    {
        NeuroNarrowItem::readPointerIds(ds, file_version);

        setFrontLinkTarget(0);
        setBackLinkTarget(0);

        if (file_version.neurolab_version >= NeuroLab::NEUROLAB_FILE_VERSION_1)
        {
            IdType id;

            ds >> id;
            if (id)
                _frontLinkTarget = reinterpret_cast<NeuroItem *>(id);

            ds >> id;
            if (id)
                _backLinkTarget = reinterpret_cast<NeuroItem *>(id);
        }
        else if (file_version.neurolab_version >= NeuroLab::NEUROLAB_FILE_VERSION_OLD)
        {
            qint64 id64;
            IdType id;

            ds >> id64; id = static_cast<IdType>(id64);
            if (id)
                _frontLinkTarget = reinterpret_cast<NeuroItem *>(id);

            ds >> id64; id = static_cast<IdType>(id64);
            if (id)
                _backLinkTarget = reinterpret_cast<NeuroItem *>(id);
        }
    }

    void NeuroLinkItem::idsToPointers(QGraphicsScene *sc)
    {
        NeuroNarrowItem::idsToPointers(sc);

        IdType frontId = reinterpret_cast<IdType>(_frontLinkTarget);
        IdType backId = reinterpret_cast<IdType>(_backLinkTarget);

        for (QListIterator<QGraphicsItem *> i(sc->items()); i.hasNext(); )
        {
            NeuroItem *item = dynamic_cast<NeuroItem *>(i.next());
            if (item)
            {
                if (item->id() == frontId)
                    _frontLinkTarget = item;
                else if (item->id() == backId)
                    _backLinkTarget = item;
            }
        }
    }


    //////////////////////////////////////////////////////////////////

    NEUROITEM_DEFINE_CREATOR(NeuroExcitoryLinkItem, QObject::tr("Narrow|Excitory Link"));

    NeuroExcitoryLinkItem::NeuroExcitoryLinkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroLinkItem(network, scenePos, context)
    {
        Q_ASSERT(network != 0 && network->neuronet() != 0);

        if (context == CREATE_UI)
        {
            NeuroCell::NeuroIndex index = network->neuronet()->addNode(NeuroCell(NeuroCell::EXCITORY_LINK));
            setCellIndex(index);
        }

        setLine(scenePos.x(), scenePos.y(), scenePos.x()+NeuroNarrowItem::NODE_WIDTH*2, scenePos.y()-NeuroNarrowItem::NODE_WIDTH*2);
    }

    NeuroExcitoryLinkItem::~NeuroExcitoryLinkItem()
    {
    }

    void NeuroExcitoryLinkItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroLinkItem::addToShape(drawPath, texts);

        QVector2D center(pos());
        QVector2D front(line().p2());
        QVector2D fromFront((center + c2) - front);
        double angle = ::atan2(fromFront.y(), fromFront.x());

        double langle = angle + (20.0 * M_PI / 180.0);
        QVector2D left(::cos(langle), ::sin(langle));

        QVector2D end = front + left * ELLIPSE_WIDTH;

        drawPath.moveTo((front - center).toPointF());
        drawPath.lineTo((end - center).toPointF());

        double rangle = angle - (20.0 * M_PI / 180.0);
        QVector2D right(::cos(rangle), ::sin(rangle));

        end = front + right * ELLIPSE_WIDTH;

        drawPath.moveTo((front - center).toPointF());
        drawPath.lineTo((end - center).toPointF());
    }

    bool NeuroExcitoryLinkItem::canAttachTo(const QPointF & pos, NeuroItem *item)
    {
        // can not attach to a link at all
        return NeuroLinkItem::canAttachTo(pos, item) && !dynamic_cast<NeuroLinkItem *>(item);
    }


    //////////////////////////////////////////////////////////////////

    NEUROITEM_DEFINE_CREATOR(NeuroInhibitoryLinkItem, QObject::tr("Narrow|Inhibitory Link"));

    NeuroInhibitoryLinkItem::NeuroInhibitoryLinkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroLinkItem(network, scenePos, context)
    {
        Q_ASSERT(network != 0 && network->neuronet() != 0);

        if (context == CREATE_UI)
        {
            NeuroCell::NeuroIndex index = network->neuronet()->addNode(NeuroCell(NeuroCell::INHIBITORY_LINK, -100000000));
            setCellIndex(index);
        }

        setLine(scenePos.x(), scenePos.y(), scenePos.x()+NeuroNarrowItem::NODE_WIDTH*2, scenePos.y()-NeuroNarrowItem::NODE_WIDTH*2);
    }

    NeuroInhibitoryLinkItem::~NeuroInhibitoryLinkItem()
    {
    }

    void NeuroInhibitoryLinkItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroLinkItem::addToShape(drawPath, texts);

        QVector2D center(pos());
        QVector2D front(line().p2());

        drawPath.addEllipse((front-center).toPointF(), ELLIPSE_WIDTH/2, ELLIPSE_WIDTH/2);
    }

} // namespace NeuroLab
