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

#include "subconnectionitem.h"
#include "subnetworkitem.h"
#include "../labnetwork.h"
#include "../labscene.h"

#define _USE_MATH_DEFINES
#include <cmath>

using namespace NeuroLib;

namespace NeuroGui
{

    NEUROITEM_DEFINE_CREATOR(SubConnectionItem, QString("__INTERNAL__"), QString("SubConnectionItem"));

    SubConnectionItem::SubConnectionItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroNetworkItem(network, scenePos, context), MixinArrow(this),
          _direction(INCOMING), _parentSubnetworkItem(0), _governingItem(0),
          _value_property(this, &SubConnectionItem::outputValue, &SubConnectionItem::setOutputValue,
                          tr("Output Value"), tr("The output value of the corresponding item in the outer network."), false)
    {
        setInitialPosAndDir(QVector2D(scenePos), QVector2D(-10, 1));
    }

    SubConnectionItem::SubConnectionItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context,
                                         SubNetworkItem *parent, NeuroItem *governing, const Directions & direction,
                                         const QVector2D & initialPos, const QVector2D & initialDir)
        : NeuroNetworkItem(network, scenePos, context), MixinArrow(this),
         _direction(direction), _parentSubnetworkItem(parent), _governingItem(0),
         _value_property(this, &SubConnectionItem::outputValue, &SubConnectionItem::setOutputValue,
                         tr("Output Value"), tr("The output value of the corresponding item in the outer network."), false)
    {
        setInitialPosAndDir(initialPos, initialDir);
        setGoverningItem(governing);
    }

    SubConnectionItem::~SubConnectionItem()
    {
        if (_ui_delete)
        {
            setGoverningItem(0);
        }
    }

    NeuroLib::NeuroCell::Value SubConnectionItem::outputValue() const
    {
        Q_ASSERT(_governingItem);
        const NeuroNetworkItem *item = dynamic_cast<NeuroNetworkItem *>(_governingItem);
        return item ? item->outputValue() : 0;
    }

    void SubConnectionItem::setOutputValue(const NeuroLib::NeuroCell::Value & val)
    {
        Q_ASSERT(_governingItem);
        NeuroNetworkItem *item = dynamic_cast<NeuroNetworkItem *>(_governingItem);
        if (item) item->setOutputValue(val);
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
        // disconnect from old item
        if (_governingItem)
        {
            disconnect(_governingItem, SIGNAL(labelChanged(QString)), this, SLOT(setLabel(QString)));

            // disconnect network nodes
            if (_frontLinkTarget)
                _frontLinkTarget->onDetach(this);

            if (_backLinkTarget)
                _backLinkTarget->onDetach(this);
        }

        // set new governing item and connect
        _governingItem = item;
        if (_governingItem)
        {
            setLabel(_governingItem->label());
            connect(_governingItem, SIGNAL(labelChanged(QString)), this, SLOT(setLabel(QString)));
        }

        if (_frontLinkTarget)
            _frontLinkTarget->onAttachedBy(this);
        if (_backLinkTarget)
            _backLinkTarget->onAttachedBy(this);
    }

    bool SubConnectionItem::canAttachTo(const QPointF &pos, NeuroItem *item)
    {
        if (_dragFront && _governingItem)
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
        NeuroNetworkItem::onAttachTo(item);
    }

    NeuroCell::Index SubConnectionItem::getIncomingCellFor(const NeuroItem *item) const
    {
        Q_ASSERT(_governingItem);

        if (item == _parentSubnetworkItem)
        {
            NeuroNetworkItem *netFront = dynamic_cast<NeuroNetworkItem *>(_frontLinkTarget);
            return netFront ? netFront->getIncomingCellFor(this) : -1;
        }
        else
        {
            NeuroNetworkItem *netGov = dynamic_cast<NeuroNetworkItem *>(_governingItem);
            return netGov ? netGov->getIncomingCellFor(_parentSubnetworkItem) : -1;
        }
    }

    NeuroCell::Index SubConnectionItem::getOutgoingCellFor(const NeuroItem *item) const
    {
        Q_ASSERT(_governingItem);

        if (item == _parentSubnetworkItem)
        {
            NeuroNetworkItem *netFront = dynamic_cast<NeuroNetworkItem *>(_frontLinkTarget);
            return netFront ? netFront->getOutgoingCellFor(this) : -1;
        }
        else
        {
            NeuroNetworkItem *netGov = dynamic_cast<NeuroNetworkItem *>(_governingItem);
            return netGov ? netGov->getOutgoingCellFor(_parentSubnetworkItem) : -1;
        }
    }

    bool SubConnectionItem::handleMove(const QPointF &mousePos, QPointF &movePos)
    {
        // move the line
        MixinArrow::changePos(movePos, true, false);

        // break links
        MixinArrow::breakLinks(mousePos);

        // attach
        return NeuroNetworkItem::handleMove(mousePos, movePos);
    }

    void SubConnectionItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroNetworkItem::addToShape(drawPath, texts);

        // calculate line
        QVector2D myPos(scenePos());   // in scene

        QVector2D myFront(_line.p2()); // in my frame; note the _line not line()
        QVector2D myBack(_line.p1());  // in my frame

        c1 = myBack + _initialDir * (myFront - myBack).length() * 0.33;
        c2 = myBack + (myFront - myBack) * 0.66;

        if (_frontLinkTarget)
        {
            QVector2D center(_frontLinkTarget->targetPointFor(this));
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
        if (_direction.testFlag(INCOMING))
        {
            addPoint(drawPath, front, (myFront - c2).normalized(), ELLIPSE_WIDTH);
        }

        if (_direction.testFlag(OUTGOING))
        {
            addPoint(drawPath, back, (myBack - c1).normalized(), ELLIPSE_WIDTH);
        }
    }

    static const int NUM_STEPS = 6;

    void SubConnectionItem::drawWavyLine(QPainterPath & drawPath,
                                         const QVector2D & a, const QVector2D & b,
                                         const qreal & angle1, const qreal & angle2) const
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
        const MixinArrow *link = dynamic_cast<const MixinArrow *>(_governingItem);

        if (link)
        {
            link->setPenGradient(pen, _line);
        }
        else if (_governingItem)
        {
            _governingItem->setPenProperties(pen);
        }

        if (shouldHighlight())
            pen.setWidth(NeuroItem::HOVER_LINE_WIDTH);
        else
            pen.setWidth(NeuroItem::NORMAL_LINE_WIDTH);
    }

    void SubConnectionItem::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        NeuroNetworkItem::writeBinary(ds, file_version);
        MixinArrow::writeBinary(ds, file_version);

        ds << static_cast<qint32>(_direction);
        ds << _initialPos;
        ds << _initialDir;
    }

    void SubConnectionItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        NeuroNetworkItem::readBinary(ds, file_version);
        MixinArrow::readBinary(ds, file_version);

        qint32 flag;
        ds >> flag; _direction = Directions(flag);
        ds >> _initialPos;
        ds >> _initialDir;
    }

    void SubConnectionItem::writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        NeuroNetworkItem::writePointerIds(ds, file_version);
        MixinArrow::writePointerIds(ds, file_version);

        NeuroNetworkItem::IdType id = _governingItem ? _governingItem->id() : 0;
        ds << id;

        id = _parentSubnetworkItem->id();
        ds << id;
    }

    void SubConnectionItem::readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        NeuroNetworkItem::readPointerIds(ds, file_version);
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
        NeuroNetworkItem::idsToPointers(idMap);
        MixinArrow::idsToPointers(idMap);

        NeuroItem::IdType governingId = reinterpret_cast<NeuroItem::IdType>(_governingItem);
        NeuroItem *wanted_item = idMap[governingId];

        if (governingId && wanted_item)
        {
            _governingItem = wanted_item;
        }
        else if (governingId)
        {
            throw Exception(tr("Subconnection item in file has dangling governing item: %1").arg(governingId));
        }

        NeuroItem::IdType parentId = reinterpret_cast<NeuroItem::IdType>(_parentSubnetworkItem);
        SubNetworkItem *wanted_parent = dynamic_cast<SubNetworkItem *>(idMap[parentId]);

        if (parentId && wanted_parent)
        {
            _parentSubnetworkItem = wanted_parent;
        }
        else
        {
            throw Exception(tr("Subconnection item in file has dangling parent item: %1").arg(parentId));
        }
    }

} // namespace NeuroGui
