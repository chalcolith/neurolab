#ifndef GRIDEDGEITEM_H
#define GRIDEDGEITEM_H

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
#include "../neurogui/neuronetworkitem.h"
#include "../neurogui/mixins/mixinremember.h"

#include <QList>

namespace GridItems
{

    class GRIDITEMSSHARED_EXPORT GridEdgeItem
        : public NeuroGui::NeuroNetworkItem, public NeuroGui::MixinRemember
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

        bool _vertical;

        mutable QVector2D _point1;
        mutable QVector2D _point2;

    public:
        GridEdgeItem(NeuroGui::LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~GridEdgeItem();

        virtual NeuroLib::NeuroCell::Value outputValue() const { return 0; }
        virtual void setOutputValue(const NeuroLib::NeuroCell::Value &) {}

        virtual bool handleMove(const QPointF & mousePos, QPointF & movePos);

        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *) const;
        virtual QVector2D getAttachPos(const QVector2D &pos);
        virtual void onAttachedBy(NeuroItem *item);
        virtual void onDetach(NeuroItem *item);

        virtual QPointF targetPointFor(const NeuroItem *) const;
        virtual void addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const;
        virtual void adjustLinks();

        bool isConnectedToTop(const NeuroItem *) const;
        bool isConnectedToBottom(const NeuroItem *) const;
        bool isConnectedToLeft(const NeuroItem *) const;
        bool isConnectedToRight(const NeuroItem *) const;

        virtual QList<Index> allCells() const { return QList<Index>(); }

    protected:
        virtual void writeBinary(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version) const;
        virtual void readBinary(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version);
        virtual void postLoad();

    private:
        bool isConnected(const NeuroItem *item, bool vertical, const QVector2D & pt1, const QVector2D & pt2) const;
    };

} // namespace GridItems

#endif // GRIDEDGEITEM_H
