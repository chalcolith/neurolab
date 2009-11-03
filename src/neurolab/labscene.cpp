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
            switch (mode.top())
            {
            case MODE_ADD_NODE:
                // pick up node if we're on one
                // if we're not on a node, make a new one
                {
                    NeuroNodeItem *item = new NeuroNodeItem();
                    const QRectF & ir = item->rect();
                    const QPointF & pos = event->scenePos();
                    
                    item->setPos(pos.x() - ir.width()/2, pos.y() - ir.height()/2);
                    
                    addItem(item);
                    movingNode = item;
                }
                return;
                
            case MODE_ADD_E_LINK:
                // pick up link end if we're on it
                
                // otherwise, start rubber-banding new link
                {
                    const QPointF & pos = event->scenePos();
                    NeuroExcitoryLinkItem *item = new NeuroExcitoryLinkItem();
                    item->setLine(pos.x(), pos.y(), pos.x()+20, pos.y()-20);
                    
                    addItem(item);
                    movingLink = item;
                }
                return;
                
            case MODE_ADD_I_LINK:
                {
                    const QPointF & pos = event->scenePos();
                    NeuroInhibitoryLinkItem *item = new NeuroInhibitoryLinkItem();
                    item->setLine(pos.x(), pos.y(), pos.x()+20, pos.y()-20);
                    
                    addItem(item);
                    movingLink = item;
                }
                return;
                
            default:
                break;
            }
        }
        else if (event->buttons() == Qt::RightButton)
        {
        }

        QGraphicsScene::mousePressEvent(event);
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
                const QRectF & ir = movingNode->rect();
                const QPointF & pos = event->scenePos();
                
                movingNode->setPos(pos.x() - ir.width()/2, pos.y() - ir.height()/2);
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
