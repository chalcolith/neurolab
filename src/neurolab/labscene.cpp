#include "labscene.h"

#include <QGraphicsSceneMouseEvent>
#include <QVector2D>

namespace NeuroLab
{
    
    LabScene::LabScene()
        : QGraphicsScene(0), movingNode(0), movingLink(0), linkFront(true)
    {
        mode.push(MODE_NONE);
    }
    
    LabScene::~LabScene()
    {
    }
    
    void LabScene::setMode(const Mode & m)
    {
        // transition from other modes
        switch (mode.top())
        {
        default:
            break;
        }
        
        // set new mode
        mode.top() = m;
    }
    
    const LabScene::Mode LabScene::getMode() const
    {
        return mode.top();
    }
    
    void LabScene::deleteSelectedItem()
    {
        QListIterator<QGraphicsItem *> i(this->selectedItems());
        while (i.hasNext())
        {
            QGraphicsItem *item = i.next();
            this->removeItem(item);
            delete item;
        }
    }

    void LabScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        movingNode = 0;
        movingLink = 0;
        
        if (event->buttons() == Qt::LeftButton)
        {
            // pick up node if we're on one
            if (!mousePressPickupNode(event))
            {
                switch (mode.top())
                {
                case MODE_ADD_NODE:
                    // if we're not on a node, make a new one
                    if (mousePressAddNode(event))
                        return;
                    break;
                    
                case MODE_ADD_E_LINK:                    
                    // start rubber-banding new link
                    if (mousePressAddLink(event, new NeuroExcitoryLinkItem()))
                        return;
                    break;
                    
                case MODE_ADD_I_LINK:
                    if (mousePressAddLink(event, new NeuroInhibitoryLinkItem()))
                        return;
                    break;
                    
                default:
                    break;
                }
            }
        }
        else if (event->buttons() == Qt::RightButton)
        {
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
    
    bool LabScene::mousePressAddNode(QGraphicsSceneMouseEvent *event)
    {
        NeuroNodeItem *item = new NeuroNodeItem();
        const QPointF & pos = event->scenePos();
        
        item->setPos(pos.x(), pos.y());
        
        addItem(item);
        movingNode = item;
        
        return true;
    }
    
    bool LabScene::mousePressAddLink(QGraphicsSceneMouseEvent *event, NeuroLinkItem *linkItem)
    {
        const QPointF & pos = event->scenePos();
        linkItem->setLine(pos.x(), pos.y(), pos.x()+20, pos.y()-20);
        
        addItem(linkItem);
        movingLink = linkItem;
        linkFront = true;
        
        return true;
    }
    
    void LabScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        if (movingNode)
            movingNode->setInHover(false);
        movingNode = 0;
        
        if (movingLink)
            movingLink->setInHover(false);
        movingLink = 0;
        
        QGraphicsScene::mouseReleaseEvent(event);
    }
    
    void LabScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        if (movingNode)
        {
            const QPointF & pos = event->scenePos();
            
            movingNode->setPos(pos.x(), pos.y());
            movingNode->adjustLinks();
        }
        else if (movingLink)
        {
            QPointF pos = event->scenePos();
            
            // check for break/form link
            if (!mouseMoveHandleLinks(event, pos, movingLink))
            {
                // update line            
                const QLineF & line = movingLink->line();
                QPointF front = line.p2();
                QPointF back = line.p1();            
                QPointF & endToMove = linkFront ? front : back;
                
                endToMove = pos;
                
                movingLink->setLine(back.x(), back.y(), front.x(), front.y());
                
                if (movingLink->frontLinkTarget())
                    movingLink->frontLinkTarget()->adjustLinks();
                if (movingLink->backLinkTarget())
                    movingLink->backLinkTarget()->adjustLinks();
                                
                movingLink->adjustLinks();
            }
        }
        
        QGraphicsScene::mouseMoveEvent(event);
    }
    
    bool LabScene::mouseMoveHandleLinks(QGraphicsSceneMouseEvent *event, QPointF & scenePos, NeuroLinkItem *link)
    {
        // get topmost item that is not the link itself
        NeuroItem *itemAtPos = 0;
        QListIterator<QGraphicsItem *> i(this->items(scenePos, Qt::IntersectsItemShape, Qt::DescendingOrder));
        while (i.hasNext())
        {
            itemAtPos = dynamic_cast<NeuroItem *>(i.next());
            if (itemAtPos && dynamic_cast<NeuroLinkItem *>(itemAtPos) != link)
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
        if (itemAtPos && canLink(link, itemAtPos))
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
