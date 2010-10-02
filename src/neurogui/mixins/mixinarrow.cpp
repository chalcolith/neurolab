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

#include "mixinarrow.h"
#include "../labexception.h"
#include "../neuroitem.h"
#include "../narrow/neurolinkitem.h"
#include "../labscene.h"

#include <cmath>

namespace NeuroGui
{

    MixinArrow::MixinArrow(NeuroItem *self)
        : _self(self),
        _frontLinkTarget(0), _backLinkTarget(0), _dragFront(false), _settingLine(false)
    {
        Q_ASSERT(self);
    }

    MixinArrow::~MixinArrow()
    {
        _frontLinkTarget = 0;
        _backLinkTarget = 0;
    }

    QLineF MixinArrow::line() const
    {
        QVector2D center(_self->scenePos());
        QVector2D p1 = QVector2D(_line.p1()) + center;
        QVector2D p2 = QVector2D(_line.p2()) + center;

        return QLineF(p1.toPointF(), p2.toPointF());
    }

    void MixinArrow::setLine(const QLineF & l, const QPointF *const c)
    {
        _settingLine = true;
        _self->prepGeomChange();

        QVector2D p1(l.p1());
        QVector2D p2(l.p2());
        QVector2D center = c ? QVector2D(*c) : ((p1 + p2) * 0.5f);

        _line = QLineF((p1 - center).toPointF(), (p2 - center).toPointF());
        _self->setPos(center.toPointF());

        _self->updateShape();
        _self->adjustLinks();

        _settingLine = false;
    }

    void MixinArrow::setLine(const qreal & x1, const qreal & y1, const qreal & x2, const qreal & y2)
    {
        setLine(QLineF(x1, y1, x2, y2));
    }

    void MixinArrow::setLine(const QPointF & p1, const QPointF & p2)
    {
        setLine(QLineF(p1, p2));
    }

    void MixinArrow::setFrontLinkTarget(NeuroItem *linkTarget)
    {
        NeuroItem *frontTarget = _frontLinkTarget;
        _frontLinkTarget = 0;

        if (frontTarget)
            _self->removeOutgoing(frontTarget);

        if (linkTarget)
            _self->addOutgoing(linkTarget);

        _frontLinkTarget = linkTarget;

        _self->updateShape();
    }

    void MixinArrow::setBackLinkTarget(NeuroItem *linkTarget)
    {
        NeuroItem *backTarget = _backLinkTarget;
        _backLinkTarget = 0;

        if (backTarget)
            _self->removeIncoming(backTarget);

        if (linkTarget)
            _self->addIncoming(linkTarget);

        _backLinkTarget = linkTarget;

        _self->updateShape();
    }

    void MixinArrow::addLine(QPainterPath & drawPath) const
    {
        QVector2D myPos(_self->scenePos());

        // in my frame
        QVector2D myFront(_line.p2());
        QVector2D myBack(_line.p1());

        // calculate control points
        if (_frontLinkTarget && _frontLinkTarget == _backLinkTarget)
        {
            QVector2D toFront = myFront - myBack;
            QVector2D toBack = myBack - myFront;

            c1 = toBack;
            c2 = toFront;
        }
        else
        {
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
                QVector2D center(_frontLinkTarget->scenePos());
                QVector2D toFront = QVector2D(line().p2()) - center;
                c2 = (center + toFront * 3) - myPos;
            }
        }

        QPointF front = myFront.toPointF();
        QPointF back = myBack.toPointF();

        drawPath.moveTo(back);
        drawPath.cubicTo(c1.toPointF(), c2.toPointF(), front);
    }

    void MixinArrow::addPoint(QPainterPath & drawPath, const QPointF & pos, const QVector2D & dir, const qreal & len) const
    {
        QVector2D reverse_dir = -dir;

        qreal reverse_angle = ::atan2(reverse_dir.y(), reverse_dir.x());
        qreal left_angle = reverse_angle + 30.0 * M_PI / 180.0;
        qreal right_angle = reverse_angle - 30.0 * M_PI / 180.0;

        QVector2D barb1(::cos(left_angle), ::sin(left_angle));
        barb1 *= len;

        drawPath.moveTo(pos);
        drawPath.lineTo((QVector2D(pos) + barb1).toPointF());

        QVector2D barb2(::cos(right_angle), ::sin(right_angle));
        barb2 *= len;

        drawPath.moveTo(pos);
        drawPath.lineTo((QVector2D(pos) + barb2).toPointF());
    }

    QVariant MixinArrow::changePos(LabScene *labScene, const QVariant & value, bool canDragFront, bool canDragBack)
    {
        if (!_settingLine && labScene
            && labScene->selectedItems().size() <= 1
            && labScene->mouseIsDown())
        {
            _self->prepGeomChange();

            // adjust position
            QVector2D oldCenter(_self->scenePos());

            QVector2D front = QVector2D(_line.p2()) + oldCenter;
            QVector2D back = QVector2D(_line.p1()) + oldCenter;
            QVector2D mousePos(labScene->lastMousePos());

            qreal distFront = (mousePos - front).lengthSquared();
            qreal distBack = (mousePos - back).lengthSquared();

            _dragFront = distFront < distBack;
            if (_dragFront)
            {
                if (canDragFront)
                    front = mousePos;
                else
                    return value;
            }
            else if (!_dragFront)
            {
                if (canDragBack)
                    back = mousePos;
                else
                    return value;
            }

            QVector2D newCenter = (front + back) * 0.5f;

            // set new position and line
            _settingLine = true;
            _line = QLineF((back - newCenter).toPointF(), (front - newCenter).toPointF());
            _self->setPos(newCenter.toPointF());
            _settingLine = false;

            // adjust if we're linked
            if (_frontLinkTarget && _dragFront)
                _frontLinkTarget->adjustLinks();
            if (_backLinkTarget && !_dragFront)
                _backLinkTarget->adjustLinks();

            // handle move
            QPointF movePos(_self->scenePos());
            _self->handleMove(mousePos.toPointF(), movePos);

            _self->adjustLinks();
            _self->updateShape();

            return QVariant(_self->scenePos());
        }

        return value;
    }

    void MixinArrow::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        ds << _line;
    }

    void MixinArrow::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        ds >> _line;
    }

    void MixinArrow::writePointerIds(QDataStream &ds, const NeuroLabFileVersion &) const
    {
        NeuroItem::IdType id = _frontLinkTarget ? _frontLinkTarget->id() : 0;
        ds << id;

        id = _backLinkTarget ? _backLinkTarget->id() : 0;
        ds << id;
    }

    void MixinArrow::readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        setFrontLinkTarget(0);
        setBackLinkTarget(0);

        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_1)
        {
            NeuroItem::IdType id;

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
            NeuroItem::IdType id;

            ds >> id64; id = static_cast<NeuroItem::IdType>(id64);
            if (id)
                _frontLinkTarget = reinterpret_cast<NeuroItem *>(id);

            ds >> id64; id = static_cast<NeuroItem::IdType>(id64);
            if (id)
                _backLinkTarget = reinterpret_cast<NeuroItem *>(id);
        }
    }

    void MixinArrow::idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> & idMap)
    {
        NeuroItem::IdType frontId = reinterpret_cast<NeuroItem::IdType>(_frontLinkTarget);
        NeuroItem *frontItem = idMap[frontId];

        if (frontId && frontItem)
        {
            _frontLinkTarget = frontItem;
        }
        else if (frontId)
        {
            throw LabException(QObject::tr("Link in file has dangling ID: %1").arg(frontId));
        }

        NeuroItem::IdType backId = reinterpret_cast<NeuroItem::IdType>(_backLinkTarget);
        NeuroItem *backItem = idMap[backId];

        if (backId && backItem)
        {
            _backLinkTarget = backItem;
        }
        else if (backId)
        {
            throw LabException(QObject::tr("Link in file has dangling ID: %2").arg(backId));
        }
    }

} // namespace NeuroGui
