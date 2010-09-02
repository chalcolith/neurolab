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

#include "labscene.h"
#include "labnetwork.h"
#include "neuroitem.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../neurolib/neuronet.h"

#include <QGraphicsSceneMouseEvent>
#include <QVector2D>

using namespace NeuroLib;

namespace NeuroGui
{

    LabScene::LabScene(LabNetwork *_network)
        : QGraphicsScene(_network), _network(_network), _itemUnderMouse(0), _mouseIsDown(false)
    {
        connect(this, SIGNAL(selectionChanged()), _network, SLOT(selectionChanged()), Qt::UniqueConnection);
        connect(this, SIGNAL(itemCreated(NeuroItem*)), MainWindow::instance(), SLOT(createdItem(NeuroItem*)), Qt::UniqueConnection);
    }

    LabScene::~LabScene()
    {
    }

    void LabScene::newItem(const QString & typeName, const QPointF & scenePos)
    {
        if (!_network)
            return;

        NeuroItem *item = NeuroItem::create(typeName, this, scenePos, NeuroItem::CREATE_UI);
        if (item)
        {
            addItem(item);
            item->updateShape();

            QPointF movePos(scenePos);
            item->handleMove(scenePos, movePos);

            clearSelection();

            _network->setChanged(true);
            emit itemCreated(item);
        }
    }

    void LabScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        QGraphicsScene::contextMenuEvent(event);

        if (!event->isAccepted())
        {
            QMenu menu;

            NeuroItem::buildNewMenu(this, itemUnderMouse(), event->scenePos(), menu);
            menu.addSeparator();
            NeuroItem::buildActionMenu(this, itemUnderMouse(), event->scenePos(), menu);

            QAction *action = menu.exec(event->screenPos());

            // check for the need to create a new item
            if (action)
            {
                QString typeName = action->data().toString();

                if (!typeName.isNull() && !typeName.isEmpty())
                {
                    newItem(typeName, event->scenePos());
                }
            }
        }
    }

    void LabScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        _lastMousePos = event->scenePos();
        QGraphicsScene::mouseMoveEvent(event);
    }

    void LabScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        _mouseIsDown = true;
        _lastMousePos = event->scenePos();

        if (event->button() == Qt::RightButton && _itemUnderMouse)
        {
            if ((event->modifiers() & Qt::ControlModifier) != 0)
            {
                _itemUnderMouse->setSelected(!_itemUnderMouse->isSelected());
                return;
            }
            else
            {
                clearSelection();
                _itemUnderMouse->setSelected(true);
                update();
                return;
            }
        }
        else if (event->button() == Qt::LeftButton && !_itemUnderMouse && _network)
        {

        }

        QGraphicsScene::mousePressEvent(event);
    }

    void LabScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        _mouseIsDown = false;
        _lastMousePos = event->scenePos();
        QGraphicsScene::mouseReleaseEvent(event);
    }

} // namespace NeuroGui
