#ifndef SUBCONNECTIONITEM_H
#define SUBCONNECTIONITEM_H

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

#include "../neurogui_global.h"
#include "../neuronetworkitem.h"
#include "../mixins/mixinarrow.h"

#include "../../neurolib/neuronet.h"

#include <QList>
#include <QFlags>

namespace NeuroGui
{

    class SubNetworkItem;

    /// An item that transfers activation from links in an outer network into a subnetwork.
    class NEUROGUISHARED_EXPORT SubConnectionItem
        : public NeuroNetworkItem, public MixinArrow
    {
        Q_OBJECT

    public:
        /// Which direction activation can be transferred to or from the outer network.
        enum Direction
        {
            /// The item does not transfer activation.
            NONE     = 0,

            /// The item transfers activation from the outer network to the inner one.
            INCOMING = 1 << 0,

            /// The item transfers activation from the inner network to the outer one.
            OUTGOING = 1 << 1,

            /// The item is bidirectional.
            BOTH     = INCOMING | OUTGOING
        };

        /// Type-safe flag type for directions.
        Q_DECLARE_FLAGS(Directions, Direction)

    private:
        Directions _direction;
        QVector2D _initialPos, _initialDir;

        SubNetworkItem *_parentSubnetworkItem;
        NeuroItem *_governingItem;
        Property<SubConnectionItem, QVariant::Double, double, NeuroLib::NeuroCell::Value> _value_property;

    public:
        /// Creator.
        /// \see NeuroGui::NeuroItem::NeuroItem()
        explicit SubConnectionItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);

        /// Creator
        /// \param network The network this item is part of.
        /// \param scenePos The position in the current scene at which to create the item.
        /// \param context The context in which the item is being created.
        /// \param parent The subnetwork this item is part of.
        /// \param governing The item in the outer network that corresponds to this item.
        /// \param direction Direction to transfer activation.
        /// \param initialPos The initial position of the link.
        /// \param initialDir The initial direction the link should point.
        explicit SubConnectionItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context,
                                   SubNetworkItem *parent, NeuroItem *governing, const Directions & direction,
                                   const QVector2D & initialPos, const QVector2D & initialDir);
        virtual ~SubConnectionItem();

        virtual QString uiName() const { return tr("Subnetwork Connection"); }

        /// The output value of the governing item.
        /// \see SubConnectionItem::setOutputValue()
        /// \see SubConnectionItem::governingItem()
        virtual NeuroLib::NeuroCell::Value outputValue() const;

        /// Set the output value of the governing item.
        /// \see SubConnectionItem::outputValue()
        /// \see SubConnectionItem::governingItem()
        virtual void setOutputValue(const NeuroLib::NeuroCell::Value &);

        /// The directions the item can transfer activation in.
        /// \see SubConnectionItem::setDirection()
        const Directions & direction() const { return _direction; }

        /// Set the directions the item can transfer activation in.
        /// \see SubConnectionItem::direction()
        void setDirection(const Directions & direction) { _direction = direction; }

        /// The subnetwork this node is in.
        SubNetworkItem *parentSubnetwork() const { return _parentSubnetworkItem; }

        /// The item in the outer network that this item corresponds to.
        /// \see SubConnectionItem::setGoverningItem()
        NeuroItem *governingItem() const { return _governingItem; }

        /// Set the item in the outer network that this item corresponds to.
        /// \see SubConnectionItem::governingItem()
        void setGoverningItem(NeuroItem *item);

        /// Set the initial position and direction for the item.
        void setInitialPosAndDir(const QVector2D & initialPos, const QVector2D & initialDir);

        virtual NeuroLib::NeuroCell::Index getIncomingCellFor(const NeuroItem *) const;
        virtual NeuroLib::NeuroCell::Index getOutgoingCellFor(const NeuroItem *) const;

    protected:
        virtual bool canAttachTo(const QPointF &, NeuroItem *);
        virtual void onAttachTo(NeuroItem *);
        virtual bool handleMove(const QPointF &mousePos, QPointF &movePos);

        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;
        virtual void setPenProperties(QPen &pen) const;

        virtual void writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const;
        virtual void readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version);

        virtual void writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const;
        virtual void readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version);
        virtual void idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> & idMap);

    private:
        void drawWavyLine(QPainterPath & drawPath, const QVector2D & a, const QVector2D & b, const qreal & angle1, const qreal & angle2) const;
    };

    Q_DECLARE_OPERATORS_FOR_FLAGS(SubConnectionItem::Directions)

} // namespace NeuroGui

#endif // SUBCONNECTIONITEM_H
