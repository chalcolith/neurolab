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

namespace NeuroGui
{

    //////////////////////////////////////////////////////////////////

    NeuroLinkItem::NeuroLinkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroNarrowItem(network, scenePos, context), MixinArrow(this),
        _weight_property(this, &NeuroLinkItem::weight, &NeuroLinkItem::setWeight,
                         tr("Weight"), tr("The output weight of the link."))
    {
        Q_ASSERT(network != 0);
    }

    NeuroLinkItem::~NeuroLinkItem()
    {
    }

    NeuroCell::NeuroValue NeuroLinkItem::weight() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
        return cell ? cell->current().weight() : 0;
    }

    void NeuroLinkItem::setWeight(const NeuroLib::NeuroCell::NeuroValue & value)
    {
        NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
        if (cell)
        {
            cell->current().setWeight(value);
            cell->former().setWeight(value);
        }
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

    void NeuroLinkItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroNarrowItem::addToShape(drawPath, texts);
        addArrow(drawPath);
    }

    void NeuroLinkItem::setPenProperties(QPen & pen) const
    {
        NeuroNarrowItem::setPenProperties(pen);

        const NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
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
        QVector2D center = QVector2D(scenePos()) + ((c1 + c2) * 0.5f);

        for (QListIterator<NeuroItem *> i(incoming()); i.hasNext(); )
        {
            NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(i.next());
            if (link)
                link->setLine(link->line().p1(), center.toPointF());
        }
    }

    bool NeuroLinkItem::canAttachTo(const QPointF &, NeuroItem *item)
    {
        // cannot link the back of a link to another link
        if (dynamic_cast<NeuroLinkItem *>(item) && !_dragFront)
            return false;

        if (_dragFront && outgoing().contains(item))
            return false;
        else if (!_dragFront && incoming().contains(item))
            return false;

        return true;
    }

    bool NeuroLinkItem::canBeAttachedBy(const QPointF &, NeuroItem *item)
    {
        // can only be attached to by an inhibitory link
        return dynamic_cast<NeuroInhibitoryLinkItem *>(item);
    }

    void NeuroLinkItem::attachTo(NeuroItem *item)
    {
        if (_dragFront)
            setFrontLinkTarget(item);
        else
            setBackLinkTarget(item);
    }

    bool NeuroLinkItem::handleMove(const QPointF & mousePos, QPointF & movePos)
    {
        // break links
        NeuroItem *linkedItem = _dragFront ? _frontLinkTarget : _backLinkTarget;
        if (linkedItem && !linkedItem->contains(linkedItem->mapFromScene(mousePos)))
        {
            if (_dragFront)
                setFrontLinkTarget(0);
            else
                setBackLinkTarget(0);
        }

        // attach
        return NeuroNarrowItem::handleMove(mousePos, movePos);
    }

    QVariant NeuroLinkItem::itemChange(GraphicsItemChange change, const QVariant & value)
    {
        LabScene *labScene = dynamic_cast<LabScene *>(scene());

        switch (change)
        {
        case QGraphicsItem::ItemPositionChange:
            if (labScene && dynamic_cast<NeuroLinkItem *>(labScene->itemUnderMouse()) == this)
                return changePos(labScene, value);

        default:
            break;
        }

        return _settingLine ? value : NeuroNarrowItem::itemChange(change, value);
    }

    void NeuroLinkItem::writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const
    {
        NeuroNarrowItem::writeClipboard(ds, id_map);
        ds << _line;

        if (_frontLinkTarget && id_map[_frontLinkTarget->id()])
            ds << static_cast<qint32>(id_map[_frontLinkTarget->id()]);
        else
            ds << static_cast<qint32>(0);

        if (_backLinkTarget && id_map[_backLinkTarget->id()])
            ds << static_cast<qint32>(id_map[_backLinkTarget->id()]);
        else
            ds << static_cast<qint32>(0);
    }

    void NeuroLinkItem::readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> & id_map)
    {
        NeuroNarrowItem::readClipboard(ds, id_map);
        ds >> _line;

        qint32 id;

        // front link target
        ds >> id;
        if (id && id_map[id])
            setFrontLinkTarget(id_map[id]);

        // back link target
        ds >> id;
        if (id && id_map[id])
            setBackLinkTarget(id_map[id]);
    }

    void NeuroLinkItem::writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const
    {
        NeuroNarrowItem::writeBinary(ds, file_version);
        ds << _line;
    }

    void NeuroLinkItem::readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version)
    {
        NeuroNarrowItem::readBinary(ds, file_version);

        // if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_OLD)
        {
            ds >> _line;
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

        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_1)
        {
            IdType id;

            ds >> id;
            if (id)
                _frontLinkTarget = reinterpret_cast<NeuroItem *>(id);

            ds >> id;
            if (id)
                _backLinkTarget = reinterpret_cast<NeuroItem *>(id);
        }
        else // if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_OLD)
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

        bool foundFront = false;
        bool foundBack = false;

        IdType frontId = reinterpret_cast<IdType>(_frontLinkTarget);
        IdType backId = reinterpret_cast<IdType>(_backLinkTarget);

        for (QListIterator<QGraphicsItem *> i(sc->items()); i.hasNext(); )
        {
            NeuroItem *item = dynamic_cast<NeuroItem *>(i.next());
            if (item)
            {
                if (item->id() == frontId)
                {
                    _frontLinkTarget = item;
                    foundFront = true;
                }

                if (item->id() == backId)
                {
                    _backLinkTarget = item;
                    foundBack = true;
                }
            }
        }

        if (!foundFront && frontId != 0)
            throw Automata::Exception(tr("Link in file has dangling ID: %1").arg(frontId));
        if (!foundBack && backId != 0)
            throw Automata::Exception(tr("Link in file has dangling ID: %2").arg(backId));
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
            _cellIndices.clear();
            _cellIndices.append(index);
        }

        setLine(scenePos.x(), scenePos.y(), scenePos.x()+NeuroNarrowItem::NODE_WIDTH*2, scenePos.y()-NeuroNarrowItem::NODE_WIDTH*2);
    }

    NeuroExcitoryLinkItem::~NeuroExcitoryLinkItem()
    {
    }

    void NeuroExcitoryLinkItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroLinkItem::addToShape(drawPath, texts);

        QVector2D front(_line.p2());
        QVector2D fromFront(c2 - front);

        double angle = ::atan2(fromFront.y(), fromFront.x());

        double langle = angle + (20.0 * M_PI / 180.0);
        QVector2D left(::cos(langle), ::sin(langle));

        QVector2D end = front + left * ELLIPSE_WIDTH;

        drawPath.moveTo(front.toPointF());
        drawPath.lineTo(end.toPointF());

        double rangle = angle - (20.0 * M_PI / 180.0);
        QVector2D right(::cos(rangle), ::sin(rangle));

        end = front + right * ELLIPSE_WIDTH;

        drawPath.moveTo(front.toPointF());
        drawPath.lineTo(end.toPointF());
    }

    bool NeuroExcitoryLinkItem::canAttachTo(const QPointF & pos, NeuroItem *item)
    {
        // can only attach to narrow items
        NeuroNarrowItem *narrow = dynamic_cast<NeuroNarrowItem *>(item);
        if (!narrow)
            return false;

        // cannot attach to a link at all
        NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(item);
        if (link)
            return false;

        // can not attach to a link at all
        return NeuroLinkItem::canAttachTo(pos, item);
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
            _cellIndices.clear();
            _cellIndices.append(index);
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

} // namespace NeuroGui
