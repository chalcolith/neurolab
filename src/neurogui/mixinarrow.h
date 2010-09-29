#ifndef MIXINARROW_H
#define MIXINARROW_H

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

#include <QLineF>
#include <QVector2D>
#include <QVariant>
#include <QPainterPath>

class QGraphicsScene;

namespace NeuroGui
{

    class LabScene;

    /// Mixin class for drawing arrows.
    class NEUROGUISHARED_EXPORT MixinArrow
    {
        NeuroItem *_self;

    protected:
        QLineF _line;
        mutable QVector2D c1, c2;

        NeuroItem *_frontLinkTarget, *_backLinkTarget;
        bool _dragFront, _settingLine;

    public:
        /// You must call this in derived classes' initialization, in order to correctly set the self pointer.
        explicit MixinArrow(NeuroItem *self);
        virtual ~MixinArrow();

        /// The link's back (\c p1) and front (\c p2) position, in scene coordinates.
        QLineF line() const;

        /// Sets the link's back (\c p1) and front (\c p2) positions.
        /// You can also specify a center point.
        void setLine(const QLineF & l, const QPointF *const c = 0);

        /// Sets the link's back (\c p1) and front (\c p2) positions.
        void setLine(const qreal & x1, const qreal & y1, const qreal & x2, const qreal & y2);

        /// Sets the link's back (\c p1) and front (\c p2) positions.
        void setLine(const QPointF & p1, const QPointF & p2);

        /// If the link is being moved, returns whether or not the front end of the node is being dragged.
        bool dragFront() const { return _dragFront; }

        /// The item that the front of the link is currently attached to.
        /// \see line()
        /// \see setFrontLinkTarget()
        NeuroItem *frontLinkTarget() { return _frontLinkTarget; }

        /// Sets the item that the front of the link is currently attached to.
        /// \see line()
        /// \see frontLinkTarget()
        void setFrontLinkTarget(NeuroItem *linkTarget);

        /// The item that the back of the link is currently attached to.
        /// \see line()
        /// \see setBackLinkTarget()
        NeuroItem *backLinkTarget() { return _backLinkTarget; }

        /// Sets the item that the back of the link is currently attached to.
        /// \see line()
        /// \see backLinkTarget()
        void setBackLinkTarget(NeuroItem *linkTarget);

    protected:
        /// Draws the arrow's shaft.
        void addLine(QPainterPath & drawPath) const;

        /// Draws an arrowhead.
        void addPoint(QPainterPath & drawPath, const QPointF & pos, const QVector2D & dir, const qreal & len) const;

        /// Handles dragging one end of the arrow.
        QVariant changePos(LabScene *labScene, const QVariant & value, bool canDragFront = true, bool canDragBack = true);

        void writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const;
        void readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version);

        void writePointerIds(QDataStream & ds, const NeuroLabFileVersion & file_version) const;
        void readPointerIds(QDataStream & ds, const NeuroLabFileVersion & file_version);
        void idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> & idMap);
    }; // class MixinArrow

} // namespace NeuroGui

#endif // MIXINARROW_H
