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
#include "mixinarrow.h"

#include <QVector2D>

namespace NeuroGui
{

    /// A link item that represents a one-directional link in Narrow notation.
    class NEUROGUISHARED_EXPORT NeuroLinkItem
        : public NeuroNarrowItem, public MixinArrow
    {
        Q_OBJECT

    protected:
        Property<NeuroLinkItem, QVariant::Double, double, NeuroLib::NeuroCell::NeuroValue> _weight_property;
        Property<NeuroLinkItem, QVariant::Int, int, int> _length_property;

    public:
        /// Constructor.
        /// \param network The network this item is a part of.
        /// \param scenePos The scene position at which to create the item.
        /// \param context The context in which the item is being created.
        explicit NeuroLinkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~NeuroLinkItem();

        NeuroLib::NeuroCell::NeuroValue weight() const;
        void setWeight(const NeuroLib::NeuroCell::NeuroValue & value);

        int length() const { return _cellIndices.size(); }
        void setLength(const int & value);

        /// Adds an incoming item.  Overrides the default to create an edge in the neural network.
        virtual bool addIncoming(NeuroItem *linkItem);
        /// Removes an incoming item.  Overrides the default to remove an edge in the neural network.
        virtual bool removeIncoming(NeuroItem *linkItem);

        virtual bool addOutgoing(NeuroItem *linkItem);
        virtual bool removeOutgoing(NeuroItem *linkItem);

        virtual bool canAttachTo(const QPointF &, NeuroItem *);
        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *);

        virtual void attachTo(NeuroItem *);
        virtual bool handleMove(const QPointF & mousePos, QPointF & movePos);
        virtual void adjustLinks();

        virtual void writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const;
        virtual void readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> & id_map);

        virtual void writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const;
        virtual void readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version);

        virtual void writePointerIds(QDataStream & ds, const NeuroLabFileVersion & file_version) const;
        virtual void readPointerIds(QDataStream & ds, const NeuroLabFileVersion & file_version);

    protected:
        /// Adds the link's shape to the drawing painter path.
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;

        virtual void setPenProperties(QPen & pen) const;
        virtual void setBrushProperties(QBrush & brush) const;

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        virtual void idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> & idMap);

        virtual void addNewCell() = 0;
    };


    /// An excitory link in narrow notation.
    class NEUROGUISHARED_EXPORT NeuroExcitoryLinkItem
        : public NeuroLinkItem
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

    public:
        explicit NeuroExcitoryLinkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~NeuroExcitoryLinkItem();

        virtual bool canAttachTo(const QPointF &, NeuroItem *);

        virtual QString uiName() const { return tr("Excitory Link"); }

    protected:
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;
        virtual void addNewCell();
    };


    /// An inhibitory link in narrow notation.
    class NEUROGUISHARED_EXPORT NeuroInhibitoryLinkItem
        : public NeuroLinkItem
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

    public:
        explicit NeuroInhibitoryLinkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~NeuroInhibitoryLinkItem();

        virtual QString uiName() const { return tr("Inhibitory Link"); }

    protected:
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;
        virtual void addNewCell();
    };

} // namespace NeuroGui

#endif // NEUROLINKITEM_H
