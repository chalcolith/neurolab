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

#include "subconnectionitem.h"
#include "subnetworkitem.h"
#include "../labexception.h"
#include "../labnetwork.h"
#include "../labscene.h"
#include "../narrow/neuronarrowitem.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace NeuroGui
{

    NEUROITEM_DEFINE_CREATOR(SubConnectionItem, QObject::tr("Debug|Subnetwork Connection"));

    SubConnectionItem::SubConnectionItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroItem(network, scenePos, context), MixinArrow(this),
          _direction(INCOMING), _parentSubnetworkItem(0), _governingItem(0),
          _value_property(this, &SubConnectionItem::outputValue, &SubConnectionItem::setOutputValue, tr("Output Value"), tr("The output value of the corresponding item in the outer network."), false)
    {
        setInitialPosAndDir(QVector2D(scenePos), QVector2D(-10, 1));
    }

    SubConnectionItem::SubConnectionItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context,
                                         SubNetworkItem *parent, NeuroItem *governing, quint32 direction,
                                         const QVector2D & initialPos, const QVector2D & initialDir)
        : NeuroItem(network, scenePos, context), MixinArrow(this),
         _direction(direction), _parentSubnetworkItem(parent), _governingItem(0),
         _value_property(this, &SubConnectionItem::outputValue, &SubConnectionItem::setOutputValue, tr("Output Value"), tr("The output value of the corresponding item in the outer network."), false)
    {
        setInitialPosAndDir(initialPos, initialDir);
        setGoverningItem(governing);
    }

    SubConnectionItem::~SubConnectionItem()
    {
        setFrontLinkTarget(0);
        setGoverningItem(0);
    }

    NeuroLib::NeuroCell::NeuroValue SubConnectionItem::outputValue() const
    {
        if (_governingItem)
        {
            const NeuroNarrowItem *narrow = dynamic_cast<const NeuroNarrowItem *>(_governingItem);
            return narrow ? narrow->outputValue() : 0;
        }

        return 0;
    }

    void SubConnectionItem::setOutputValue(const NeuroLib::NeuroCell::NeuroValue & val)
    {
        if (_governingItem)
        {
            NeuroNarrowItem *narrow = dynamic_cast<NeuroNarrowItem *>(_governingItem);
            if (narrow)
                narrow->setOutputValue(val);
        }
    }

    void SubConnectionItem::setInitialPosAndDir(const QVector2D & initialPos, const QVector2D & initialDir)
    {
        _initialPos = initialPos;
        _initialDir = initialDir.normalized();

        QPointF back = _initialPos.toPointF();
        QPointF front = (_initialPos + _initialDir * 50).toPointF();

        setLine(back, front);
    }

    void SubConnectionItem::setGoverningItem(NeuroItem *item)
    {
        NeuroItem *oldFrontLinkTarget = 0, *oldBackLinkTarget = 0;

        // disconnect from old item
        if (_governingItem)
        {
            disconnect(_governingItem, SIGNAL(labelChanged(QString)), this, SLOT(setLabel(QString)));

            // disconnect network nodes
            if ((oldFrontLinkTarget = _frontLinkTarget))
                setFrontLinkTarget(0);

            if ((oldBackLinkTarget = _backLinkTarget))
                setBackLinkTarget(0);
        }

        // set new governing item and connect
        _governingItem = item;
        if (_governingItem)
        {
            setLabel(_governingItem->label());
            connect(_governingItem, SIGNAL(labelChanged(QString)), this, SLOT(setLabel(QString)));
        }

        if (oldFrontLinkTarget)
            setFrontLinkTarget(oldFrontLinkTarget);
        if (oldBackLinkTarget)
            setBackLinkTarget(oldBackLinkTarget);
    }

    bool SubConnectionItem::canAttachTo(const QPointF &pos, NeuroItem *item)
    {
        if (_governingItem)
        {
            // set the dragFront flag on any arrows correctly
            MixinArrow *arrow = dynamic_cast<MixinArrow *>(_governingItem);
            if (arrow)
                arrow->setDragFront(arrow->frontLinkTarget() == _parentSubnetworkItem);

            return _governingItem->canAttachTo(pos, item);
        }

        return false;
    }

    void SubConnectionItem::onAttachTo(NeuroItem *item)
    {
        setFrontLinkTarget(item);
    }

    bool SubConnectionItem::handleMove(const QPointF &mousePos, QPointF &movePos)
    {
        // break links
        if (_frontLinkTarget && !_frontLinkTarget->contains(_frontLinkTarget->mapFromScene(mousePos)))
        {
            setFrontLinkTarget(0);
        }

        // attach
        return NeuroItem::handleMove(mousePos, movePos);
    }

    void SubConnectionItem::setFrontLinkTarget(NeuroItem *linkTarget)
    {
        if (_direction & INCOMING)
        {
            // disconnect old target
            if (_frontLinkTarget)
            {
                if (_governingItem)
                    _frontLinkTarget->removeIncoming(_governingItem);
            }

            // set new target and connect
            _frontLinkTarget = linkTarget;
            if (_frontLinkTarget)
            {
                addOutgoing(_frontLinkTarget);
                if (_governingItem)
                    _frontLinkTarget->addIncoming(_governingItem);
            }
        }

        if (_direction & OUTGOING)
        {
            // disconnect old target
            if (_frontLinkTarget)
            {
                if (_governingItem)
                    _frontLinkTarget->removeOutgoing(_governingItem);
            }

            // set new target and connect
            _frontLinkTarget = linkTarget;
            if (_frontLinkTarget)
            {
                addIncoming(_frontLinkTarget);
                if (_governingItem)
                    _frontLinkTarget->addOutgoing(_governingItem);
            }
        }
    }

    void SubConnectionItem::setBackLinkTarget(NeuroItem *linkTarget)
    {
        if (linkTarget)
            throw new LabException(tr("You cannot set the back link target on a sub-connection item."));
        else
            _backLinkTarget = linkTarget;
    }

    QVariant SubConnectionItem::itemChange(GraphicsItemChange change, const QVariant & value)
    {
        LabScene *labScene = dynamic_cast<LabScene *>(scene());

        switch (change)
        {
        case QGraphicsItem::ItemPositionChange:
            if (!_settingLine && labScene && !labScene->moveOnly() && dynamic_cast<SubConnectionItem *>(labScene->itemUnderMouse()) == this)
                return changePos(labScene, value, true, false);

        default:
            break;
        }

        return _settingLine ? value : NeuroItem::itemChange(change, value);
    }

    void SubConnectionItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroItem::addToShape(drawPath, texts);

        // calculate line
        QVector2D myPos(scenePos());   // in scene

        QVector2D myFront(_line.p2()); // in my frame; note the _line not line()
        QVector2D myBack(_line.p1());  // in my frame

        c1 = myBack + _initialDir * (myFront - myBack).length() * 0.33;
        c2 = myBack + (myFront - myBack) * 0.66;

        if (_frontLinkTarget)
        {
            QVector2D center(_frontLinkTarget->scenePos());
            QVector2D toFront = QVector2D(line().p2()) - center;
            c2 = (center + toFront * 3) - myPos;
        }

        QPointF front = myFront.toPointF();
        QPointF back = myBack.toPointF();

        drawPath.moveTo(back);
        drawPath.cubicTo(c1.toPointF(), c2.toPointF(), front);

        // draw wavy line in back (at 90 degrees to initial angle)
        qreal angle = ::atan2(_initialDir.y(), _initialDir.x());
        qreal back_angle = angle + M_PI/2;
        QVector2D newDir(::cos(back_angle), ::sin(back_angle));
        QVector2D a = myBack + newDir * 20;
        QVector2D b = myBack - newDir * 20;

        drawWavyLine(drawPath, a, b, back_angle - M_PI/4, back_angle + M_PI/4);

        // draw arrow in front or back depending on direction
        if (_direction & INCOMING)
        {
            addPoint(drawPath, front, (myFront - c2).normalized(), ELLIPSE_WIDTH);
        }

        if (_direction & OUTGOING)
        {
            addPoint(drawPath, back, (myBack - c1).normalized(), ELLIPSE_WIDTH);
        }
    }

    static const int NUM_STEPS = 6;

    void SubConnectionItem::drawWavyLine(QPainterPath & drawPath, const QVector2D & a, const QVector2D & b, const qreal & angle1, const qreal & angle2) const
    {
        QVector2D a2b = b - a;
        qreal len = a2b.length() / NUM_STEPS;

        QVector2D step1(::cos(angle1), ::sin(angle1));
        step1 *= len;

        QVector2D step2(::cos(angle2), ::sin(angle2));
        step2 *= len;

        QVector2D center = (a + b) * 0.5;

        QVector2D cur = center;
        drawPath.moveTo(cur.toPointF());
        for (int i = 0; i < NUM_STEPS/2; ++i)
        {
            cur = cur + step1;
            drawPath.lineTo(cur.toPointF());
            cur = cur + step2;
            drawPath.lineTo(cur.toPointF());
        }

        cur = center;
        drawPath.moveTo(cur.toPointF());
        for (int i = 0; i < NUM_STEPS/2; ++i)
        {
            cur = cur - step2;
            drawPath.lineTo(cur.toPointF());
            cur = cur - step1;
            drawPath.lineTo(cur.toPointF());
        }
    }

    void SubConnectionItem::setPenProperties(QPen &pen) const
    {
        if (_governingItem)
        {
            _governingItem->setPenProperties(pen);

            if (shouldHighlight())
                pen.setWidth(NeuroItem::HOVER_LINE_WIDTH);
            else
                pen.setWidth(NeuroItem::NORMAL_LINE_WIDTH);
        }
    }

    void SubConnectionItem::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        NeuroItem::writeBinary(ds, file_version);
        MixinArrow::writeBinary(ds, file_version);

        ds << _direction;
        ds << _initialPos;
        ds << _initialDir;
    }

    void SubConnectionItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        NeuroItem::readBinary(ds, file_version);
        MixinArrow::readBinary(ds, file_version);

        ds >> _direction;
        ds >> _initialPos;
        ds >> _initialDir;
    }

    void SubConnectionItem::writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        NeuroItem::writePointerIds(ds, file_version);
        MixinArrow::writePointerIds(ds, file_version);

        NeuroItem::IdType id = _governingItem ? _governingItem->id() : 0;
        ds << id;

        id = _parentSubnetworkItem->id();
        ds << id;
    }

    void SubConnectionItem::readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        NeuroItem::readPointerIds(ds, file_version);
        MixinArrow::readPointerIds(ds, file_version);

        NeuroItem::IdType id;
        ds >> id;
        if (id)
            _governingItem = reinterpret_cast<NeuroItem *>(id);
        else
            _governingItem = 0;

        ds >> id;
        if (id)
            _parentSubnetworkItem = reinterpret_cast<SubNetworkItem *>(id);
        else
            _parentSubnetworkItem = 0;
    }

    void SubConnectionItem::idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> & idMap)
    {
        NeuroItem::idsToPointers(idMap);
        MixinArrow::idsToPointers(idMap);

        NeuroItem::IdType governingId = reinterpret_cast<NeuroItem::IdType>(_governingItem);
        NeuroItem *wanted_item = idMap[governingId];

        if (governingId && wanted_item)
        {
            _governingItem = wanted_item;
        }
        else if (governingId)
        {
            throw new LabException(tr("Subconnection item in file has dangling governing item: %1").arg(governingId));
        }

        NeuroItem::IdType parentId = reinterpret_cast<NeuroItem::IdType>(_parentSubnetworkItem);
        SubNetworkItem *wanted_parent = dynamic_cast<SubNetworkItem *>(idMap[parentId]);

        if (parentId && wanted_parent)
        {
            _parentSubnetworkItem = wanted_parent;
        }
        else
        {
            throw new LabException(tr("Subconnection item in file has dangling parent item: %1").arg(parentId));
        }
    }

} // namespace NeuroGui
