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
        : QGraphicsScene(0), _network(_network), _itemUnderMouse(0), _selectedItem(0)
    {
    }
    
    LabScene::~LabScene()
    {
    }
        
    void LabScene::setSelectedItem(NeuroItem *item)
    {
        NeuroItem *oldSelected = _selectedItem;
        _selectedItem = item;
        
        if (oldSelected)
            oldSelected->update();
        
        MainWindow::instance()->setPropertyObject(_selectedItem);
        
        if (_selectedItem)
        {
            _selectedItem->updateProperties();
            _selectedItem->update();
        }
    }
        
    void LabScene::deleteSelectedItem()
    {
        if (_selectedItem)
        {
            this->removeItem(_selectedItem);
            delete _selectedItem;
            _selectedItem = 0;
        }
    }
    
    void LabScene::labelSelectedItem(const QString & s)
    {
        if (_selectedItem)
        {
            _selectedItem->setLabel(s);
            _selectedItem->update();
        }
    }

    void LabScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        editInfo.scenePos = event->scenePos();
        
        if (event->buttons() & Qt::LeftButton)
        {
            if (editInfo.movingItem)
            {
                editInfo.movingItem = 0;
                return;
            }
            else if (mousePressPickupNode(event))
            {
                setSelectedItem(editInfo.movingItem);
                return;
            }
            else
            {
                setSelectedItem(0);
                MainWindow::instance()->setPropertyObject(_network);
            }
        }
        else if (event->buttons() & Qt::RightButton)
        {
            editInfo.movingItem = 0;
            setSelectedItem(_itemUnderMouse);
            MainWindow::instance()->ui()->menuItem->exec(event->screenPos());

            return;
        }

        QGraphicsScene::mousePressEvent(event);
    }

    bool LabScene::mousePressPickupNode(QGraphicsSceneMouseEvent *)
    {
        NeuroItem *item = _itemUnderMouse;

        if (item && item->handlePickup(editInfo))
        {
            item->bringToFront();
            editInfo.movingItem = item;

            return true;
        }

        return false;
    }

    void LabScene::newItem(const QString & typeName)
    {
        setSelectedItem(0);
        editInfo.movingItem = 0;

        NeuroItem *item = NeuroItem::create(typeName, this, editInfo.scenePos);
        if (item)
        {
            addItem(item);
            item->buildShape();
            
            editInfo.linkFront = false;
            item->handleMove(editInfo);
            
            if (!dynamic_cast<NeuroNodeItem *>(item))
                editInfo.movingItem = item;
            
            editInfo.linkFront = true;
            
            setSelectedItem(item);
        }
    }

    void LabScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        editInfo.scenePos = event->scenePos();
        editInfo.movingItem = 0;

        QGraphicsScene::mouseReleaseEvent(event);
    }

    void LabScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        editInfo.scenePos = event->scenePos();

        if (editInfo.movingItem)
        {
            editInfo.movingItem->handleMove(editInfo);

            if (_network)
                _network->changed();

            return;
        }

        QGraphicsScene::mouseMoveEvent(event);
    }

} // namespace NeuroLab
