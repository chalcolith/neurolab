#ifndef NEUROGRIDITEM_H
#define NEUROGRIDITEM_H

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

#include "griditems_global.h"
#include "../neurogui/subnetwork/subnetworkitem.h"

namespace GridItems
{

    class GRIDITEMSSHARED_EXPORT NeuroGridItem
        : public NeuroGui::SubNetworkItem
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

        Property<NeuroGridItem, QVariant::Int, qint32, qint32> _horizontal_property;
        Property<NeuroGridItem, QVariant::Int, qint32, qint32> _vertical_property;

        qint32 _num_horiz;
        qint32 _num_vert;

    public:
        NeuroGridItem(NeuroGui::LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~NeuroGridItem();

        qint32 horizontalCols() const { return _num_horiz; }
        void setHorizontalCols(const qint32 & num) { _num_horiz = num; }

        qint32 verticalRows() const { return _num_vert; }
        void setVerticalRows(const qint32 & num) { _num_vert = num; }

    protected:
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;

        virtual bool canCreateNewItem(const QString &, const QPointF &) const;
        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *);

        virtual void writeBinary(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version) const;
        virtual void readBinary(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version);
    };

}

#endif // NEUROGRIDITEM_H
