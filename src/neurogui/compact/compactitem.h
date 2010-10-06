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
        CompactItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~CompactItem();

        virtual bool isBidirectional() const { return true; }

        QList<NeuroLib::NeuroCell::NeuroIndex> upwardCells() { return _upward_cells; }
        QList<NeuroLib::NeuroCell::NeuroIndex> downwardCells() { return _downward_cells; }

        NeuroLib::NeuroCell::NeuroValue upwardOutputValue() const { return outputValue(_upward_cells); }
        void setUpwardOutputValue(const NeuroLib::NeuroCell::NeuroValue & value) { setOutputValue(_upward_cells, value); }

        NeuroLib::NeuroCell::NeuroValue downwardOutputValue() const { return outputValue(_downward_cells); }
        void setDownwardOutputValue(const NeuroLib::NeuroCell::NeuroValue & value) { setOutputValue(_downward_cells, value); }

        virtual bool addIncoming(NeuroItem *linkItem);
        virtual bool removeIncoming(NeuroItem *linkItem);
        virtual bool addOutgoing(NeuroItem *linkItem);
        virtual bool removeOutgoing(NeuroItem *linkItem);

    private:
        NeuroLib::NeuroCell::NeuroValue outputValue(const QList<NeuroLib::NeuroCell::NeuroIndex> & cells) const;
        void setOutputValue(QList<NeuroLib::NeuroCell::NeuroIndex> & cells, const NeuroLib::NeuroCell::NeuroValue & value);

    protected:
        const NeuroLib::NeuroNet::ASYNC_STATE *getCell(const NeuroLib::NeuroCell::NeuroIndex & index) const;
        NeuroLib::NeuroNet::ASYNC_STATE *getCell(const NeuroLib::NeuroCell::NeuroIndex & index);

        virtual void writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const;
        virtual void readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version);
    }; // class CompactItem

} // namespace NeuroGui

#endif
