#include "labscene.h"
#include "labnetwork.h"
#include "neuroitem.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../neurolib/neuronet.h"

#include <QGraphicsSceneMouseEvent>
#include <QVector2D>

using namespace NeuroLib;

namespace NeuroLab
{

    LabScene::LabScene(LabNetwork *_network)
        : QGraphicsScene(_network), _network(_network), _itemUnderMouse(0)
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

        NeuroItem *item = NeuroItem::create(typeName, this, scenePos);
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
        _lastMousePos = event->scenePos();

        if (event->button() == Qt::RightButton && _itemUnderMouse)
        {
            clearSelection();
            _itemUnderMouse->setSelected(true);
        }

        QGraphicsScene::mousePressEvent(event);
    }

    void LabScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        _lastMousePos = event->scenePos();
        QGraphicsScene::mouseReleaseEvent(event);
    }

} // namespace NeuroLab
