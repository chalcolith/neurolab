#ifndef NEUROLINKITEM_H
#define NEUROLINKITEM_H

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

#include "neurogui_global.h"
#include "neuronarrowitem.h"

#include <QVector2D>

namespace NeuroLab
{

    /// A link item that represents a one-directional link in Narrow notation.
    class NEUROGUISHARED_EXPORT NeuroLinkItem
        : public NeuroNarrowItem
    {
        Q_OBJECT

        Property<NeuroLinkItem, QVariant::Double, double, NeuroLib::NeuroCell::NeuroValue> _weight_property;

    protected:
        QLineF _line;
        mutable QVector2D c1, c2;

        NeuroItem *_frontLinkTarget, *_backLinkTarget;
        bool dragFront, settingLine;

    public:
        /// Constructor.
        /// \param network The network this item is a part of.
        /// \param scenePos The scene position at which to create the item.
        NeuroLinkItem(LabNetwork *network, const QPointF & scenePos);
        virtual ~NeuroLinkItem();

        NeuroLib::NeuroCell::NeuroValue weight() const;
        void setWeight(const NeuroLib::NeuroCell::NeuroValue & value);

        /// The link's back (\c p0) and front (\c p1) position, in scene coordinates.
        QLineF line() const { return _line; }

        /// Sets the link's back (\c p0) and front (\c p1) positions.
        void setLine(const QLineF & l);
        /// Sets the link's back (\c p0) and front (\c p1) positions.
        void setLine(const qreal & x1, const qreal & y1, const qreal & x2, const qreal & y2);
        /// Sets the link's back (\c p0) and front (\c p1) positions.
        void setLine(const QPointF & p1, const QPointF & p2);

        /// Adds an incoming item.  Overrides the default to create an edge in the neural network.
        virtual bool addIncoming(NeuroItem *linkItem);
        /// Removes an incoming item.  Overrides the default to remove an edge in the neural network.
        virtual bool removeIncoming(NeuroItem *linkItem);

        virtual bool addOutgoing(NeuroItem *linkItem);
        virtual bool removeOutgoing(NeuroItem *linkItem);

        /// The item that the front of the link is currently attached to.
        /// \see line()
        /// \see setFrontLinkTarget()
        NeuroItem *frontLinkTarget() { return _frontLinkTarget; }
        /// Sets the item that the front of the link is currently attached to.
        /// \see line()
        /// \see frontLinkTarget()
        void setFrontLinkTarget(NeuroItem *linkTarget);

        /// The item that the back of the link is currently attached to.
        /// \see line()
        /// \see setBackLinkTarget()
        NeuroItem *backLinkTarget() { return _backLinkTarget; }
        /// Sets the item that the back of the link is currently attached to.
        /// \see line()
        /// \see backLinkTarget()
        void setBackLinkTarget(NeuroItem *linkTarget);

        virtual bool canAttachTo(const QPointF &, NeuroItem *);
        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *);

        virtual void attachTo(NeuroItem *);
        virtual bool handleMove(const QPointF & mousePos, QPointF & movePos);
        virtual void adjustLinks();

        virtual void writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const;
        virtual void readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version);

        virtual void writePointerIds(QDataStream & ds, const NeuroLabFileVersion & file_version) const;
        virtual void readPointerIds(QDataStream & ds, const NeuroLabFileVersion & file_version);

    private:
        /// Updates the link's position to be halfway between its front and back points.
        void updatePos();

    protected:
        /// Adds the link's shape to the drawing painter path.
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;

        virtual bool canCreateNewOnMe(const QString &, const QPointF &) const { return false; }

        virtual void setPenProperties(QPen & pen) const;
        virtual void setBrushProperties(QBrush & brush) const;

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        virtual void idsToPointers(QGraphicsScene *);
    };


    /// An excitory link in narrow notation.
    class NEUROGUISHARED_EXPORT NeuroExcitoryLinkItem
        : public NeuroLinkItem
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

    public:
        NeuroExcitoryLinkItem(LabNetwork *network, const QPointF & scenePos);
        virtual ~NeuroExcitoryLinkItem();

        virtual bool canAttachTo(const QPointF &, NeuroItem *);

        virtual QString uiName() const { return tr("Excitory Link"); }

    protected:
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;
    };


    /// An inhibitory link in narrow notation.
    class NEUROGUISHARED_EXPORT NeuroInhibitoryLinkItem
        : public NeuroLinkItem
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

    public:
        NeuroInhibitoryLinkItem(LabNetwork *network, const QPointF & scenePos);
        virtual ~NeuroInhibitoryLinkItem();

        virtual QString uiName() const { return tr("Inhibitory Link"); }

    protected:
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;
    };

} // namespace NeuroLab

#endif // NEUROLINKITEM_H
