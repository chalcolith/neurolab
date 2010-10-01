#ifndef SUBCONNECTIONITEM_H
#define SUBCONNECTIONITEM_H

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
#include "../mixins/mixinarrow.h"

#include "../../neurolib/neuronet.h"

#include <QList>

namespace NeuroGui
{

    class NEUROGUISHARED_EXPORT SubConnectionItem
        : public NeuroItem, public MixinArrow
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

    public:
        enum Direction
        {
            NONE     = 0,
            INCOMING = 1 << 0,
            OUTGOING = 1 << 1,
            BOTH     = INCOMING | OUTGOING
        };

    private:
        quint32 _direction;
        QVector2D _initialPos, _initialDir;

        NeuroItem *_governingItem;
        Property<SubConnectionItem, QVariant::Double, double, NeuroLib::NeuroCell::NeuroValue> _value_property;

    public:
        explicit SubConnectionItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~SubConnectionItem();

        virtual QString uiName() const { return tr("Subnetwork Connection"); }

        NeuroLib::NeuroCell::NeuroValue outputValue() const;
        void setOutputValue(const NeuroLib::NeuroCell::NeuroValue &);

        const quint32 & direction() const { return _direction; }
        void setDirection(const quint32 & direction) { _direction = direction; }

        NeuroItem *governingItem() const { return _governingItem; }
        void setGoverningItem(NeuroItem *item) { _governingItem = item; }

        void setInitialPosAndDir(const QVector2D & initialPos, const QVector2D & initialDir);

    protected:
        virtual bool canCutAndPaste() const { return false; }

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;

        virtual void writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const;
        virtual void readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version);

        virtual void writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const;
        virtual void readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version);
        virtual void idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> & idMap);

    private:
        void drawWavyLine(QPainterPath & drawPath, const QVector2D & a, const QVector2D & b, const qreal & angle1, const qreal & angle2) const;
    };

} // namespace NeuroGui

#endif // SUBCONNECTIONITEM_H
