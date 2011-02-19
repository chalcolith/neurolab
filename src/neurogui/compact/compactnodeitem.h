#ifndef COMPACTNODEITEM_H

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
#include "compactitem.h"
#include "../mixins/mixinremember.h"

namespace NeuroGui
{

    /// Base class for Compact/Abstract AND and OR nodes.
    class NEUROGUISHARED_EXPORT CompactNodeItem
        : public CompactItem, public MixinRemember
    {
        Q_OBJECT

    public:
        enum Direction
        {
            UPWARD = 0,
            DOWNWARD = 1
        };

    protected:
        Direction _direction;

        NeuroItem *_tipLinkItem;
        QList<NeuroItem *> _baseLinkItems;

        NeuroLib::NeuroCell::Index _frontwardTipCell, _backwardTipCell;

    public:
        /// Constructor.
        /// \see NeuroGui::NeuroItem::NeuroItem()
        explicit CompactNodeItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context);

        /// Destructor.
        virtual ~CompactNodeItem();

        /// What direction the multiple-link side of the node is facing.
        /// \see CompactNodeItem::setDirection()
        Direction direction() const { return _direction; }

        /// Set the direction of the node.
        /// \see CompactNodeItem::direction()
        void setDirection(const Direction & dir) { _direction = dir; }

        virtual NeuroLib::NeuroCell::Value outputValue() const;
        virtual void setOutputValue(const NeuroLib::NeuroCell::Value &);

        virtual bool canCutAndPaste() const { return true; }

    protected:
        virtual void onAttachedBy(NeuroItem *);
        virtual void onDetach(NeuroItem *);
        virtual void adjustLinks();

        virtual void setPenProperties(QPen &pen) const;
        virtual void setBrushProperties(QBrush &brush) const;

        virtual void postLoad();

        bool posOnTip(const QPointF & p) const;
        bool scenePosOnTip(const QPointF & p) const;

        virtual void writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const;
        virtual void readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> &id_map);

        virtual void writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const;
        virtual void readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version);

        virtual void writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const;
        virtual void readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version);
        virtual void idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap);
    }; // class CompactNodeItem

} // namespace NeuroGui

#endif
