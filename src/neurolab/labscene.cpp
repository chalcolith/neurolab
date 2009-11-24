#include "labscene.h"

#include <QGraphicsSceneMouseEvent>
#include <cmath>

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
    
    static qreal mag(const QPointF & p)
    {
        return ::sqrt(p.x()*p.x() + p.y()*p.y());
    }
    
    bool LabScene::mousePressPickupNode(QGraphicsSceneMouseEvent *event)
    {
        QGraphicsItem *item = itemAt(event->pos());
        
        if (item)
        {
            NeuroNodeItem *nodeItem = dynamic_cast<NeuroNodeItem *>(item);
            if (nodeItem)
            {
                movingNode = nodeItem;
                return true;
            }
            
            NeuroLinkItem *linkItem = dynamic_cast<NeuroLinkItem *>(item);
            if (linkItem)
            {
                movingLink = linkItem;
                
                qreal frontDist = mag(event->pos() - linkItem->line().p2());
                qreal backDist = mag(event->pos() - linkItem->line().p1());
                
                linkFront = frontDist < backDist;
                
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
        movingNode = 0;
        movingLink = 0;
        
        QGraphicsScene::mouseReleaseEvent(event);
    }
    
    void LabScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        if (movingNode)
        {
            const QPointF & pos = event->scenePos();
            
            movingNode->setPos(pos.x(), pos.y());
        }
        else if (movingLink)
        {
            const QPointF & pos = event->scenePos();
            const QLineF & line = movingLink->line();
            
            if (linkFront)
                movingLink->setLine(line.x1(), line.y1(), pos.x(), pos.y());
            else
                movingLink->setLine(pos.x(), pos.y(), line.x2(), line.y2());
        }
        
        QGraphicsScene::mouseMoveEvent(event);
    }
    
} // namespace NeuroLab
