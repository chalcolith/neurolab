#ifndef COMPACTITEM_H
#define COMPACTITEM_H

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
#include "../neuroitem.h"
#include "../../neurolib/neurocell.h"
#include "../../neurolib/neuronet.h"

namespace NeuroGui
{

    /// Base class for Compact/Abstract items.
    class NEUROGUISHARED_EXPORT CompactItem
        : public NeuroItem
    {
        Q_OBJECT

    protected:
        QList<NeuroLib::NeuroCell::NeuroIndex> _upward_cells, _downward_cells;

        Property<CompactItem, QVariant::Double, double, NeuroLib::NeuroCell::NeuroValue> _upward_output_property;
        Property<CompactItem, QVariant::Double, double, NeuroLib::NeuroCell::NeuroValue> _downward_output_property;

    public:
        /// Constructor.
        /// \see NeuroGui::NeuroItem::NeuroItem()
        explicit CompactItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);

        /// Destructor.
        virtual ~CompactItem();

        /// Compact/Abstract items are all bidirectional.
        virtual bool isBidirectional() const { return true; }

        /// Chain of automaton cells that make up the item's upward direction.
        QList<NeuroLib::NeuroCell::NeuroIndex> upwardCells() { return _upward_cells; }

        /// Chain of automaton cells that make up the item's downward direction.
        QList<NeuroLib::NeuroCell::NeuroIndex> downwardCells() { return _downward_cells; }

        /// Output value of the upward chain.
        /// \see CompactItem::upwardCells()
        NeuroLib::NeuroCell::NeuroValue upwardOutputValue() const { return outputValue(_upward_cells); }

        /// Set the output value of the upward chain.
        /// \note Sets the output value of the last cell in the chain to <tt>value</tt>, and the rest to 1.
        /// \see CompactItem::upwardCells()
        void setUpwardOutputValue(const NeuroLib::NeuroCell::NeuroValue & value) { setOutputValue(_upward_cells, value); }

        /// Output value of the downward chain.
        /// \see CompactItem::downwardCells()
        NeuroLib::NeuroCell::NeuroValue downwardOutputValue() const { return outputValue(_downward_cells); }

        /// Set the output value of the downward chain.
        /// \note Sets the output value of the last cell in the chain to <tt>value</tt>, and the rest to 1.
        /// \see CompactItem::downwardCells()
        void setDownwardOutputValue(const NeuroLib::NeuroCell::NeuroValue & value) { setOutputValue(_downward_cells, value); }

    public slots:
        /// Resets the item's output values to 0.
        virtual void reset();

    private:
        NeuroLib::NeuroCell::NeuroValue outputValue(const QList<NeuroLib::NeuroCell::NeuroIndex> & cells) const;
        void setOutputValue(QList<NeuroLib::NeuroCell::NeuroIndex> & cells, const NeuroLib::NeuroCell::NeuroValue & value);

    protected:
        /// Adds an incoming link.
        /// \note For Compact/Abstract items this will also add an outgoing link.
        virtual bool addIncoming(NeuroItem *linkItem);

        /// Remove an incoming link.
        /// \note For Compact/Abstract items this will also remove the corresponding outgoing link.
        virtual void removeIncoming(NeuroItem *linkItem);

        /// Add an outgoing link.
        /// \note For Compact/Abstract items this will also add an incoming link.
        virtual bool addOutgoing(NeuroItem *linkItem);

        /// Remove an outgoing link.
        /// \note For Compact/Abstract items this will also remove the corresponding incoming link.
        virtual void removeOutgoing(NeuroItem *linkItem);

        /// Utility function for getting the actual automaton cell.
        const NeuroLib::NeuroNet::ASYNC_STATE *getCell(const NeuroLib::NeuroCell::NeuroIndex & index) const;

        /// Utility function for getting the actual automaton cell.
        NeuroLib::NeuroNet::ASYNC_STATE *getCell(const NeuroLib::NeuroCell::NeuroIndex & index);

        virtual void writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const;
        virtual void readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version);
    }; // class CompactItem

} // namespace NeuroGui

#endif
