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

#include "neurogui_global.h"
#include "neuroitem.h"
#include "../neurolib/neuronet.h"

namespace NeuroLab
{

    /// Base class for items in Narrow notation.  These include nodes and one-directional links.
    class NEUROGUISHARED_EXPORT NeuroNarrowItem
        : public NeuroItem
    {
        Q_OBJECT

        Property<NeuroNarrowItem, QVariant::Double, double, NeuroLib::NeuroCell::NeuroValue> _value_property;

        NeuroLib::NeuroCell::NeuroIndex _cellIndex; ///< The index of the neural network cell that underlies this item.

    public:
        NeuroNarrowItem(LabNetwork *network, const QPointF & scenePos);
        virtual ~NeuroNarrowItem();

        virtual QString dataValue() const { return QString::number(outputValue()); }

        NeuroLib::NeuroCell::NeuroValue outputValue() const;
        void setOutputValue(const NeuroLib::NeuroCell::NeuroValue & value);

        virtual bool addIncoming(NeuroItem *linkItem);
        virtual bool removeIncoming(NeuroItem *linkItem);

        virtual void setPenProperties(QPen & pen) const;

        virtual void writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const;
        virtual void readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version);

        virtual void writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const;
        virtual void readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> &id_map);

    public slots:
        /// Resets the item.  If it is not frozen, sets the output value to zero.
        virtual void reset();

    protected:
        NeuroLib::NeuroCell::NeuroIndex cellIndex() const { return _cellIndex; }
        void setCellIndex(const NeuroLib::NeuroCell::NeuroIndex & index) { _cellIndex = index; }

        /// \return A pointer to the neural network cell's previous and current state.
        const NeuroLib::NeuroNet::ASYNC_STATE *getCell() const;

        /// \return A pointer to the neural network cell's previous and current state.
        NeuroLib::NeuroNet::ASYNC_STATE *getCell();
    };

}

#endif // NEURONARROWITEM_H
