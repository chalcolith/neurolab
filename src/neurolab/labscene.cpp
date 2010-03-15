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
        : QGraphicsScene(0), _network(_network), _itemUnderMouse(0)
    {
    }
    
    LabScene::~LabScene()
    {
    }
        
    void LabScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
    {
        QGraphicsScene::contextMenuEvent(event);
        if (!event->isAccepted())
        {
            MainWindow::instance()->ui()->menuItem->exec(event->screenPos());
        }
    }
        
    void LabScene::newItem(const QString & typeName, const QPointF & scenePos)
    {
        NeuroItem *item = NeuroItem::create(typeName, this, scenePos);
        if (item)
        {
            addItem(item);
            item->buildShape();            
            item->setSelected(true);
        }
    }
    
} // namespace NeuroLab
