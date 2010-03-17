#include "labscene.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "labnetwork.h"
#include "neuronodeitem.h"
#include "neurolinkitem.h"

#include "../neurolib/neuronet.h"

#include <QGraphicsSceneMouseEvent>
#include <QVector2D>

using namespace NeuroLib;

namespace NeuroLab
{

    LabScene::LabScene(LabNetwork *_network)
        : QGraphicsScene(_network), _network(_network), _itemUnderMouse(0)
    {
        connect(this, SIGNAL(selectionChanged()), _network, SLOT(selectionChanged()));
    }

    LabScene::~LabScene()
    {
    }

    void LabScene::newItem(const QString & typeName, const QPointF & scenePos)
    {
        NeuroItem *item = NeuroItem::create(typeName, this, scenePos);
        if (item)
        {
            addItem(item);
            item->buildShape();
            item->handleMove();

            clearSelection();
            //item->setSelected(true);
        }
    }

    void LabScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        QGraphicsScene::contextMenuEvent(event);
        if (!event->isAccepted())
        {
            MainWindow::instance()->ui()->menuItem->exec(event->screenPos());
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
