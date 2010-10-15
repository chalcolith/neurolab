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

using namespace NeuroLib;

namespace NeuroGui
{

    /// Compact/Abstract AND nodes.
    class NEUROGUISHARED_EXPORT CompactAndItem
        : public CompactNodeItem
    {
        Q_OBJECT

    protected:
        bool _sequential;
        qint32 _delay;

        Property<CompactAndItem, QVariant::Bool, bool, bool> _sequential_property;
        Property<CompactAndItem, QVariant::Int, qint32, qint32> _delay_property;

        QList< QList<NeuroCell::Index> > _frontwardDelayLines, _backwardDelayLines;

    public:
        /// Constructor.
        /// \see NeuroGui::NeuroItem::NeuroItem()
        explicit CompactAndItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~CompactAndItem();

        bool sequential() const { return _sequential; }
        void setSequential(const bool & seq);

        qint32 delay() const { return _delay; }
        void setDelay(const qint32 & d);

        virtual NeuroLib::NeuroCell::Index getIncomingCellFor(const NeuroItem *) const;
        virtual NeuroLib::NeuroCell::Index getOutgoingCellFor(const NeuroItem *) const;

    protected:
        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *);

        virtual void onAttachedBy(NeuroItem *);
        virtual void onDetach(NeuroItem *item);

        virtual void addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const;
        virtual void adjustLinks();
        virtual QVector2D getAttachPos(const QVector2D &);

        void writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const;
        void readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version);

    private:
        qreal getRadius() const { return NeuroItem::NODE_WIDTH + 0.25f; }
        qreal getTip() const { return (getRadius() / 2.5f) * (_direction == DOWNWARD ? -1 : 1); }

        void adjustNodeThreshold();

        void buildDelayLines();
        QList<NeuroCell::Index> buildDelayLine(int len, NeuroCell::Index in, NeuroCell::Index out);

        void teardownDelayLines();
        void clearDelayLines(QList< QList<NeuroCell::Index> > & delayLines);
        void clearDelayLine(QList<NeuroCell::Index> & delayLine);

        void writeDelayLines(QDataStream &ds, const NeuroLabFileVersion &file_version,
                             const QList< QList<NeuroCell::Index> > & delayLines) const;
        void readDelayLines(QDataStream &ds, const NeuroLabFileVersion &file_version, QList< QList<NeuroCell::Index> > & delayLines);
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
