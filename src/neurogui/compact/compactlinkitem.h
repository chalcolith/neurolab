#ifndef COMPACTLINKITEM_H
#define COMPACTLINKITEM_H

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
#include "compactitem.h"
#include "../mixins/mixinarrow.h"

namespace NeuroGui
{

    /// A Compact/Abstract bidirectional link.
    class NEUROGUISHARED_EXPORT CompactLinkItem
        : public CompactItem, public MixinArrow
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

        QList<NeuroLib::NeuroCell::Index> _frontward_cells; ///< These go out the front.
        QList<NeuroLib::NeuroCell::Index> _backward_cells; ///< These go out the back.

        Property<CompactLinkItem, QVariant::Int, int, int> _length_property;
        Property<CompactLinkItem, QVariant::Double, double, NeuroLib::NeuroCell::Value> _weight_property;

        Property<CompactLinkItem, QVariant::Double, double, NeuroLib::NeuroCell::Value> _frontward_output_property;
        Property<CompactLinkItem, QVariant::Double, double, NeuroLib::NeuroCell::Value> _backward_output_property;

    public:
        /// Constructor.
        /// \see NeuroGui::NeuroItem::NeuroItem()
        explicit CompactLinkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);

        /// Destructor.
        virtual ~CompactLinkItem();

        virtual QString uiName() const { return tr("Abstract Link"); }

        /// The length of the link (i.e. how many time steps it takes to transmit its activation).
        /// \see CompactLinkItem::setLength()
        int length() const { return _frontward_cells.size(); }

        /// Set the length of the link.
        /// \see CompactLinkItem::length()
        void setLength(const int & value);

        /// The weight of the link (multiplied by its input activation to produce its output value).
        /// \see CompactLinkItem::setWeight()
        NeuroLib::NeuroCell::Value weight() const;

        /// Set the weight of the link.
        /// \see CompactLinkItem::weight()
        void setWeight(const NeuroLib::NeuroCell::Value & value);

        virtual NeuroLib::NeuroCell::Value outputValue() const { return qMax(frontwardOutputValue(), backwardOutputValue()); }
        virtual void setOutputValue(const NeuroLib::NeuroCell::Value & val) { setFrontwardOutputValue(val); setBackwardOutputValue(val); }

        /// Output value of the frontward chain.
        /// \see CompactItem::upwardCells()
        NeuroLib::NeuroCell::Value frontwardOutputValue() const { return outputValueAux(_frontward_cells); }

        /// Set the output value of the frontward chain.
        /// \note Sets the output value of the last cell in the chain to <tt>value</tt>, and the rest to 1.
        /// \see CompactItem::upwardCells()
        void setFrontwardOutputValue(const NeuroLib::NeuroCell::Value & value) { setOutputValueAux(_frontward_cells, value); }

        /// Output value of the backward chain.
        /// \see CompactItem::downwardCells()
        NeuroLib::NeuroCell::Value backwardOutputValue() const { return outputValueAux(_backward_cells); }

        /// Set the output value of the backward chain.
        /// \note Sets the output value of the last cell in the chain to <tt>value</tt>, and the rest to 1.
        /// \see CompactItem::downwardCells()
        void setBackwardOutputValue(const NeuroLib::NeuroCell::Value & value) { setOutputValueAux(_backward_cells, value); }

        virtual NeuroLib::NeuroCell::Index getIncomingCellFor(const NeuroItem *) const;
        virtual NeuroLib::NeuroCell::Index getOutgoingCellFor(const NeuroItem *) const;

    public slots:
        virtual void reset();

    protected:
        virtual bool handleMove(const QPointF &mousePos, QPointF &movePos);

        virtual bool canAttachTo(const QPointF &, NeuroItem *);
        virtual void onAttachTo(NeuroItem *);
        virtual void onDetach(NeuroItem *);

        virtual void addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const;
        virtual void setPenProperties(QPen &pen) const;

        virtual void writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const;
        virtual void readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version);
        virtual void writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const;
        virtual void readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version);
        virtual void idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap);

    private:
        void addNewCell(bool frontwards);
        void setWeightAux(QList<NeuroLib::NeuroCell::Index> & cells, const NeuroLib::NeuroCell::Value &value);

        NeuroLib::NeuroCell::Value outputValueAux(const QList<NeuroLib::NeuroCell::Index> & cells) const;
        void setOutputValueAux(QList<NeuroLib::NeuroCell::Index> & cells, const NeuroLib::NeuroCell::Value & value);
    }; // class CompactLinkItem

} // namespace NeuroGui

#endif
