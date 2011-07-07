#ifndef MULTILINK_H
#define MULTILINK_H

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
#include "multiitem.h"
#include "../neurogui/mixins/mixinarrow.h"

namespace GridItems
{

    class GRIDITEMSSHARED_EXPORT MultiLink
        : public MultiItem, public NeuroGui::MixinArrow
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

        QList<QList<Index> > _frontward_lines, _backward_lines;
        Property<MultiLink, QVariant::Int, int, int> _width_property;
        Property<MultiLink, QVariant::Int, int, int> _length_property;
        Property<MultiLink, QVariant::Double, double, NeuroLib::NeuroCell::Value> _weight_property;

    public:
        MultiLink(NeuroGui::LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~MultiLink();

        int width() const { return _frontward_lines.size(); }
        void setWidth(const int &);

        int length() const { return _frontward_lines.front().size(); }
        void setLength(const int &);

        virtual Value weight() const;
        virtual void setWeight(const Value &);

        virtual Value outputValue() const;
        virtual void setOutputValue(const Value &);

        virtual QList<Index> getIncomingCellsFor(const NeuroItem *item) const;
        virtual QList<Index> getOutgoingCellsFor(const NeuroItem *item) const;
        virtual QList<Index> allCells() const;

        virtual bool canAttachTo(const QPointF &, NeuroItem *) const;
        virtual void onAttachTo(NeuroItem *item);
        virtual void onDetach(NeuroItem *item);

        virtual bool handleMove(const QPointF &mousePos, QPointF &movePos);
        virtual void addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const;
        virtual void setPenProperties(QPen &pen) const;
        virtual void setPenGradient(QPen &, const QLineF &) const;

    protected:
        virtual void writeBinary(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version) const;
        virtual void readBinary(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version);
        virtual void writePointerIds(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version) const;
        virtual void readPointerIds(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version);
        virtual void idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap);
    };

} // namespace GridItems

#endif // MULTILINK_H
