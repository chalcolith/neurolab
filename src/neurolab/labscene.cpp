#include "labscene.h"

#include <QGraphicsSceneMouseEvent>

namespace NeuroLab
{
    
    LabScene::LabScene()
        : QGraphicsScene(0), movingNode(0), movingLink(0)
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
    
    bool LabScene::mousePressPickupNode(QGraphicsSceneMouseEvent *event)
    {
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
        
        return true;
    }
    
    void LabScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        switch (mode.top())
        {
        case MODE_ADD_NODE:
            movingNode = 0;
            break;
        case MODE_ADD_E_LINK:
        case MODE_ADD_I_LINK:
            movingLink = 0;
            break;
        default:
            break;
        }
        
        QGraphicsScene::mouseReleaseEvent(event);
    }
    
    void LabScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        switch (mode.top())
        {
        case MODE_ADD_NODE:
            if (movingNode)
            {
                const QPointF & pos = event->scenePos();
                
                movingNode->setPos(pos.x(), pos.y());
            }
            break;
            case MODE_ADD_E_LINK:
            case MODE_ADD_I_LINK:
            if (movingLink)
            {
                const QPointF & pos = event->scenePos();
                const QLineF & line = movingLink->line();
                
                movingLink->setLine(line.x1(), line.y1(), pos.x(), pos.y());
            }
            break;
            default:
            break;
        }
        
        QGraphicsScene::mouseMoveEvent(event);
    }
        
} // namespace NeuroLab
