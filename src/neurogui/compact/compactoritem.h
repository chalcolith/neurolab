#ifndef COMPACTORITEM_H
#define COMPACTORITEM_H

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

#include "../neurogui_global.h"
#include "compactnodeitem.h"
#include "../../neurolib/neurocell.h"

namespace NeuroGui
{

    class NEUROGUISHARED_EXPORT CompactOrItem
        : public CompactNodeItem
    {
        Q_OBJECT

    protected:
        bool _shortcut;
        QSet<NeuroItem *> _shortcutItems;

        NeuroLib::NeuroNet _futureNetwork;

    public:
        explicit CompactOrItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~CompactOrItem();

        bool shortcut() const { return _shortcut; }

        virtual NeuroLib::NeuroCell::Index getIncomingCellFor(const NeuroItem *) const;
        virtual NeuroLib::NeuroCell::Index getOutgoingCellFor(const NeuroItem *) const;

        virtual QPointF targetPointFor(const NeuroItem *item) const;

    public slots:
        void postStep();

    protected:
        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *) const;

        virtual void onAttachedBy(NeuroItem *);
        virtual void onDetach(NeuroItem *);

        virtual void addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const;
        virtual void setBrushProperties(QBrush &brush) const;

        virtual void adjustLinks();
        virtual QVector2D getAttachPos(MixinArrow *link, const QVector2D &);

        virtual void writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const;
        virtual void readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> &id_map);
        virtual void writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const;
        virtual void readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version);

        virtual void writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const;
        virtual void readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version);
        virtual void idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap);

    private:
        qreal getRadius() const { return NeuroItem::NODE_WIDTH + 0.25f; }
        qreal getTip() const { return (getRadius() * 0.2f) * (_direction == DOWNWARD ? -1 : 1); }
    }; // class CompactOrItem


    //

    class NEUROGUISHARED_EXPORT CompactUpwardOrItem
        : public CompactOrItem
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

    public:
        explicit CompactUpwardOrItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
            : CompactOrItem(network, scenePos, context)
        {
            setDirection(UPWARD);
        }

        virtual ~CompactUpwardOrItem() {}
    };

    class NEUROGUISHARED_EXPORT CompactDownwardOrItem
        : public CompactOrItem
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

    public:
        explicit CompactDownwardOrItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
            : CompactOrItem(network, scenePos, context)
        {
            setDirection(DOWNWARD);
        }

        virtual ~CompactDownwardOrItem() {}
    };

} // namespace NeuroGui

#endif
