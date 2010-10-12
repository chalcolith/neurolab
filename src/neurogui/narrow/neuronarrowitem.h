#ifndef NEURONARROWITEM_H
#define NEURONARROWITEM_H

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

namespace NeuroGui
{

    /// Base class for items in Narrow notation.  These include nodes and one-directional links.
    class NEUROGUISHARED_EXPORT NeuroNarrowItem
        : public NeuroNetworkItem
    {
        Q_OBJECT

    protected:
        QList<NeuroLib::NeuroCell::Index> _cellIndices;

    public:
        /// Constructor.
        /// \see NeuroGui::NeuroItem::NeuroItem()
        explicit NeuroNarrowItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~NeuroNarrowItem();

        /// The indices of the automaton cells that are controlled by this item.
        /// For narrow items, which are unidirectional, the first index is the incoming, and the last is the outgoing.
        QList<NeuroLib::NeuroCell::Index> & cellIndices() { return _cellIndices; }

        /// The output value of the item.
        /// \see NeuroNarrowItem::setOutputValue()
        virtual NeuroLib::NeuroCell::Value outputValue() const;

        /// Set the output value of the item.
        /// \see NeuroNarrowItem::outputValue()
        virtual void setOutputValue(const NeuroLib::NeuroCell::Value & value);

        virtual bool canCutAndPaste() const { return true; }
        virtual void setPenProperties(QPen & pen) const;

        virtual void writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const;
        virtual void readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version);

        virtual void writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const;
        virtual void readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> & id_map);

    public slots:
        virtual void reset();
    };

}

#endif // NEURONARROWITEM_H
