#ifndef LABSCENE_H
#define LABSCENE_H

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

#include <QGraphicsScene>
#include <QStack>

namespace NeuroGui
{

    class LabNetwork;
    class NeuroItem;

    /// A derived class from QGraphicsScene that handles displaying the neural network items.
    class NEUROGUISHARED_EXPORT LabScene
        : public QGraphicsScene
    {
        Q_OBJECT

        LabNetwork *_network;
        NeuroItem *_itemUnderMouse;

        bool _mouseIsDown;
        bool _moveOnly;
        QPointF _lastMousePos;

    public:
        /// Constructor.
        /// \param _network The LabNetwork to display.
        explicit LabScene(LabNetwork *_network);
        virtual ~LabScene();

        /// \return The LabNetwork that this scene displays.
        LabNetwork *network() { return _network; }

        bool mouseIsDown() const { return _mouseIsDown; }
        bool moveOnly() const { return _moveOnly; }

        /// \return The position of the mouse (in scene coordinates).
        const QPointF & lastMousePos() const { return _lastMousePos; }

        /// \return A pointer to the topmose item (if any) currently under the mouse.
        NeuroItem *itemUnderMouse() const { return _itemUnderMouse; }

        /// Used by items to set themselves as being the item under the mouse.
        void setItemUnderMouse(NeuroItem *item) { _itemUnderMouse = item; }

    public slots:
        /// Creates a new item with the given type name.
        /// \param typeName The C++ type name of the item to create.
        /// \param scenePos The position (in scene coordinates) to position the newly-created item.
        /// \see NeuroItem
        void newItem(const QString & typeName, const QPointF & scenePos);

    signals:
        void itemCreated(NeuroItem *item);

    protected:
        virtual void keyReleaseEvent(QKeyEvent *event);
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    };

} // namespace NeuroGui

#endif // LABSCENE_H
