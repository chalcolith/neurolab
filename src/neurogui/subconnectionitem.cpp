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
#include "labexception.h"
#include "labnetwork.h"
#include "labscene.h"
#include "neuronarrowitem.h"

#include <cmath>

namespace NeuroGui
{

    NEUROITEM_DEFINE_CREATOR(SubConnectionItem, QObject::tr("Debug|Subnetwork Connection"));

    SubConnectionItem::SubConnectionItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroItem(network, scenePos, context), MixinArrow(this),
          _direction(NONE), _governingItem(0),
          _value_property(this, &SubConnectionItem::outputValue, &SubConnectionItem::setOutputValue, tr("Output Value"), tr("The output value of the corresponding item in the outer network."))
    {
    }

    SubConnectionItem::~SubConnectionItem()
    {
    }

    NeuroLib::NeuroCell::NeuroValue SubConnectionItem::outputValue() const
    {
        if ((_direction & INCOMING) && (_direction & OUTGOING))
        {
            throw new LabException(tr("You cannot take the value of a bidirectional subconnection node."));
        }
        else if (_governingItem)
        {
            const NeuroNarrowItem *narrow = dynamic_cast<const NeuroNarrowItem *>(_governingItem);
            return narrow ? narrow->outputValue() : 0;
        }

        return 0;
    }

    void SubConnectionItem::setOutputValue(const NeuroLib::NeuroCell::NeuroValue & val)
    {
        if ((_direction & INCOMING) && (_direction & OUTGOING))
        {
            throw new LabException(tr("You cannot set the value of a bidirectional subconnection node."));
        }
        else if (_governingItem)
        {
            NeuroNarrowItem *narrow = dynamic_cast<NeuroNarrowItem *>(_governingItem);
            if (narrow)
            {
                narrow->setOutputValue(val);
                return;
            }
        }
    }

    void SubConnectionItem::setInitialPosAndDir(const QVector2D & initialPos, const QVector2D & initialDir)
    {
        _initialPos = initialPos;
        _initialDir = initialDir.normalized();

        QPointF back = _initialPos.toPointF();
        QPointF front = (_initialPos + _initialDir * 30).toPointF();

        setLine(back, front);
    }

    void SubConnectionItem::itemChange(GraphicsItemChange change, const QVariant & value)
    {
        LabScene *labScene = dynamic_cast<LabScene *>(scene());

        switch (change)
        {
        case QGraphicsItem::ItemPositionChange:
            if (!_settingLine && labScene && dynamic_cast<SubConnectionItem *>(labScene->itemUnderMouse()) == this)
                return changePos(labScene, value, true, false);

        default:
            break;
        }

        return _settingLine ? value : NeuroNarrowItem::itemChange(change, value);
    }

    void SubConnectionItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroItem::addToShape(drawPath, texts);

        // calculate line
        QVector2D myFront(_line.p1());
        QVector2D myBack(_line.p2());

        c2 = myBack + (myFront - myBack) * 0.66;
        c1 = myBack + _initialDir * c2.length();

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
        angle += M_PI/2;
        QVector2D newDir(::cos(angle), ::sin(angle));
        QVector2D a = myBack + newDir * 20;
        QVector2D b = myBack - newDir * 20;

        drawWavyLine(drawPath, a, b);

        // draw arrow in front or back depending on direction
    }

    void SubConnectionItem::drawWavyLine(QPainterPath & drawPath, const QVector2D & a, const QVector2D & b)
    {
    }

} // namespace NeuroGui
