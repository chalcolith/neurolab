#ifndef NEURONETWORKITEM_H
#define NEURONETWORKITEM_H

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

#include "neurogui_global.h"
#include "neuroitem.h"
#include "../neurolib/neuronet.h"

namespace NeuroGui
{

    /// Base class for items that deal with underlying network automaton cells.
    class NEUROGUISHARED_EXPORT NeuroNetworkItem
        : public NeuroItem
    {
        Q_OBJECT

    public:
        typedef NeuroLib::NeuroCell::Index Index;
        typedef NeuroLib::NeuroCell::Value Value;

    protected:
        Property<NeuroNetworkItem, QVariant::Bool, bool, bool> _frozen_property;
        Property<NeuroNetworkItem, QVariant::Double, double, Value> _value_property;
        Property<NeuroNetworkItem, QVariant::Int, int, int> _persist_property;

    public:
        explicit NeuroNetworkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~NeuroNetworkItem();

        virtual QString dataValue() const;

        virtual Value outputValue() const = 0;
        virtual void setOutputValue(const Value &) = 0;

        int persist() const;
        void setPersist(const int & p);

        bool frozen() const;
        void setFrozen(const bool & f);

        virtual QList<Index> allCells() const = 0;

        virtual QList<Index> getIncomingCellsFor(const NeuroItem *item) const = 0;
        virtual QList<Index> getOutgoingCellsFor(const NeuroItem *item) const = 0;

        virtual void addEdges(NeuroItem *);
        virtual void removeEdges(NeuroItem *);

        virtual void onDetach(NeuroItem *item);

    public slots:
        virtual void reset();
        virtual void cleanup();

        /// Toggles the output value of the item between 0 and 1.
        virtual void toggleActivated();

        /// Toggles whether or not the item is frozen (i.e. whether or not its output value will change during a time step).
        virtual void toggleFrozen();

    protected:
        virtual void buildActionMenu(LabScene *, const QPointF &, QMenu &);

        virtual void setPenProperties(QPen &pen) const;

        /// \return A pointer to the neural network cell's previous and current state.
        const NeuroLib::NeuroNet::ASYNC_STATE *getCell(const Index & index) const;

        /// \return A pointer to the neural network cell's previous and current state.
        NeuroLib::NeuroNet::ASYNC_STATE *getCell(const Index & index);
    };

} // namespace NeuroGui

#endif
