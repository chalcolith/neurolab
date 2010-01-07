#include "labscene.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "labnetwork.h"

#include <QGraphicsSceneMouseEvent>
#include <QVector2D>

namespace NeuroLab
{
    
    LabScene::LabScene(LabNetwork *_network)
        : QGraphicsScene(0), _network(_network), movingNode(0), movingLink(0), linkFront(true), _itemToSelect(0)
    {
    }
    
    LabScene::~LabScene()
    {
    }
        
    void LabScene::deleteSelectedItem()
    {
        if (_itemToSelect)
        {
            this->removeItem(_itemToSelect);
            delete _itemToSelect;
            _itemToSelect = 0;
        }
        
        for (QListIterator<QGraphicsItem *> i(this->selectedItems()); i.hasNext(); i.next())
        {
            QGraphicsItem *item = i.peekNext();
            this->removeItem(item);
            delete item;
        }
    }

    void LabScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        _lastMousePos = event->scenePos();

        movingNode = 0;
        movingLink = 0;
        
        if (event->buttons() & Qt::LeftButton)
        {
            // pick up node if we're on one
            if (mousePressPickupNode(event))
                return;
        }
        else if (event->buttons() & Qt::RightButton)
        {
            QGraphicsItem *item = this->itemAt(event->scenePos());
            _itemToSelect = item;

            MainWindow::instance()->ui()->menuItem->exec(event->screenPos());

            _itemToSelect = 0;
            return;
        }

        QGraphicsScene::mousePressEvent(event);
    }
    
    bool LabScene::mousePressPickupNode(QGraphicsSceneMouseEvent *event)
    {
        QGraphicsItem *item = this->itemAt(event->scenePos());
        
        if (item)
        {
            NeuroNodeItem *nodeItem = dynamic_cast<NeuroNodeItem *>(item);
            if (nodeItem)
            {
                nodeItem->bringToFront();
                movingNode = nodeItem;
                return true;
            }
            
            NeuroLinkItem *linkItem = dynamic_cast<NeuroLinkItem *>(item);
            if (linkItem)
            {
                linkItem->bringToFront();
                movingLink = linkItem;
                
                qreal front_dist = (QVector2D(event->scenePos()) - QVector2D(linkItem->line().p2())).lengthSquared();
                qreal back_dist = (QVector2D(event->scenePos()) - QVector2D(linkItem->line().p1())).lengthSquared();
                
                linkFront = front_dist < back_dist;
                
                return true;
            }
        }
        
        return false;
    }
    
    void LabScene::newItem(ItemType type)
    {
        switch (type)
        {
        case NODE_ITEM:
            addNode(_lastMousePos);
            break;
        case EXCITORY_LINK_ITEM:
            addLink(_lastMousePos, new NeuroExcitoryLinkItem());
            break;
        case INHIBITORY_LINK_ITEM:
            addLink(_lastMousePos, new NeuroInhibitoryLinkItem());
            break;
        }
    }

    bool LabScene::addNode(const QPointF & pos)
    {
        NeuroNodeItem *item = new NeuroNodeItem();
        
        item->setPos(pos.x(), pos.y());
        
        addItem(item);
        movingNode = item;

        mouseMoveHandleNode(pos, movingNode);

        // don't go into move mode anymore
        movingNode = 0;

        return true;
    }

    bool LabScene::addLink(const QPointF & pos, NeuroLinkItem *linkItem)
    {
        linkItem->setLine(pos.x(), pos.y(), pos.x()+NeuroItem::NODE_WIDTH*2, pos.y()-NeuroItem::NODE_WIDTH*2);
        
        addItem(linkItem);
        movingLink = linkItem;
        linkFront = true;
        
        // link back end if we're on a node
        linkFront = false;
        mouseMoveHandleLinks(pos, movingLink);
        linkFront = true;
        
        // don't go into move mode
        movingLink = 0;

        return true;
    }
    
    void LabScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        _lastMousePos = event->scenePos();

        movingNode = 0;
        movingLink = 0;
        
        QGraphicsScene::mouseReleaseEvent(event);
    }
    
    void LabScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        _lastMousePos = event->scenePos();

        if (movingNode)
        {
            QPointF pos = event->scenePos();
            
            if (!mouseMoveHandleNode(pos, movingNode))
            {
                movingNode->setPos(pos.x(), pos.y());
                movingNode->adjustLinks();
            }
            
            if (_network)
                _network->changed();

            return;
        }
        else if (movingLink)
        {
            QPointF pos = event->scenePos();
            
            // check for break/form link
            if (!mouseMoveHandleLinks(pos, movingLink))
            {
                // update line            
                const QLineF & line = movingLink->line();
                QPointF front = line.p2();
                QPointF back = line.p1();            
                QPointF & endToMove = linkFront ? front : back;
                
                endToMove = pos;
                
                movingLink->setLine(back.x(), back.y(), front.x(), front.y());                
                movingLink->adjustLinks();
                
                if (movingLink->frontLinkTarget())
                    movingLink->frontLinkTarget()->adjustLinks();
                else if (movingLink->backLinkTarget())
                    movingLink->backLinkTarget()->adjustLinks();
            }

            if (_network)
                _network->changed();
            
            return;
        }
        
        QGraphicsScene::mouseMoveEvent(event);
    }
    
    bool LabScene::mouseMoveHandleNode(const QPointF & scenePos, NeuroNodeItem *node)
    {
        // get topmost link
        NeuroLinkItem *itemAtPos = 0;
        bool front = false;
        for (QListIterator<QGraphicsItem *> i(node->collidingItems(Qt::IntersectsItemShape)); i.hasNext(); i.next())
        {
            itemAtPos = dynamic_cast<NeuroLinkItem *>(i.peekNext());
            if (itemAtPos && !(itemAtPos->frontLinkTarget() == node || itemAtPos->backLinkTarget() == node))
            {
                // figure out which end to link
                qreal frontDist = (QVector2D(scenePos) - QVector2D(itemAtPos->line().p2())).lengthSquared();
                qreal backDist = (QVector2D(scenePos) - QVector2D(itemAtPos->line().p1())).lengthSquared();
                front = frontDist < backDist;

                // don't attach to a link that's already attached to something                
                if (!(front ? itemAtPos->frontLinkTarget() : itemAtPos->backLinkTarget()) && canLink(itemAtPos, node))
                    break;
            }

            itemAtPos = 0;
        }
        
        // connect to link
        if (itemAtPos)
        {
            if (front)
                itemAtPos->setFrontLinkTarget(node);
            else
                itemAtPos->setBackLinkTarget(node);
            
            node->adjustLinks();
            return true;
        }
        
        return false;
    }
    
    bool LabScene::mouseMoveHandleLinks(const QPointF & scenePos, NeuroLinkItem *link)
    {
        // get topmost item that is not the link itself
        NeuroItem *itemAtPos = 0;
        for (QListIterator<QGraphicsItem *> i(this->items(scenePos, Qt::IntersectsItemShape, Qt::DescendingOrder)); i.hasNext(); i.next())
        {
            itemAtPos = dynamic_cast<NeuroItem *>(i.peekNext());
            if (itemAtPos && dynamic_cast<NeuroLinkItem *>(itemAtPos) != link && canLink(link, itemAtPos))
                break;
            else
                itemAtPos = 0;
        }
        
        // break links
        NeuroItem *linkedItem = linkFront ? link->frontLinkTarget() : link->backLinkTarget();
        if (linkedItem)
        {
            if (itemAtPos != linkedItem)
            {
                if (linkFront)
                    link->setFrontLinkTarget(0);
                else
                    link->setBackLinkTarget(0);
            }
        }
        
        // form links
        if (itemAtPos)
        {
            if (linkFront)
                link->setFrontLinkTarget(itemAtPos);
            else
                link->setBackLinkTarget(itemAtPos);
            
            itemAtPos->adjustLinks();
            return true;
        }
        
        return false;
    }
    
    bool LabScene::canLink(NeuroItem *moving, NeuroItem *fixed)
    {
        // cannot link the back of a link to another link
        if (dynamic_cast<NeuroLinkItem *>(fixed) && !linkFront)
            return false;
        
        // cannot link an excitory link to another link
        if (dynamic_cast<NeuroExcitoryLinkItem*>(moving) && dynamic_cast<NeuroLinkItem *>(fixed))
            return false;
        
        return true;
    }
    
} // namespace NeuroLab
