#ifndef COMAPCTANDITEM_H
#define COMPACTANDITEM_H

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
#include "compactnodeitem.h"

namespace NeuroGui
{

    /// Compact/Abstract AND nodes.
    class NEUROGUISHARED_EXPORT CompactAndItem
        : public CompactNodeItem
    {
        Q_OBJECT

    protected:
        bool _sequence;

        Property<CompactAndItem, QVariant::Bool, bool, bool> _sequence_property;

    public:
        /// Constructor.
        /// \see NeuroGui::NeuroItem::NeuroItem()
        explicit CompactAndItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~CompactAndItem();

        bool sequence() const { return _sequence; }
        void setSequence(const bool & seq) { _sequence = seq; }

        virtual NeuroLib::NeuroCell::Index getIncomingCellFor(const NeuroItem *) const;
        virtual NeuroLib::NeuroCell::Index getOutgoingCellFor(const NeuroItem *) const;

    protected:
        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *);
        virtual QVector2D getAttachPos(const QVector2D &dirTo);

        virtual void onAttachedBy(NeuroItem *);
        virtual void onDetach(NeuroItem *item);

        virtual void addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const;

    private:
        qreal getRadius() const { return NeuroItem::NODE_WIDTH; }
        qreal getTip() const { return (getRadius() / 3.0f) * (_direction == DOWNWARD ? -1 : 1); }

        void adjustNodeThreshold();
    }; // class CompactAndItem


    //

    class NEUROGUISHARED_EXPORT CompactUpwardAndItem
        : public CompactAndItem
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

    public:
        explicit CompactUpwardAndItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
            : CompactAndItem(network, scenePos, context)
        {
            setDirection(UPWARD);
        }

        virtual ~CompactUpwardAndItem() {}
    }; // class CompactUpwardAndItem

    class NEUROGUISHARED_EXPORT CompactDownwardAndItem
        : public CompactAndItem
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

    public:
        explicit CompactDownwardAndItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
            : CompactAndItem(network, scenePos, context)
        {
            setDirection(DOWNWARD);
        }

        virtual ~CompactDownwardAndItem() {}
    }; // class CompactDownwardAndItem

} // namespace NeuroGui

#endif
