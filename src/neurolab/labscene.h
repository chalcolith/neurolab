#ifndef LABSCENE_H
#define LABSCENE_H

#include "neuronodeitem.h"
#include "neurolinkitem.h"

#include <QGraphicsScene>
#include <QStack>

namespace NeuroLab
{
    
    class LabNetwork;

    class LabScene 
        : public QGraphicsScene
    {
        LabNetwork *_network;

        NeuroNodeItem *movingNode;
        NeuroLinkItem *movingLink;
        bool linkFront;
        
        QGraphicsItem *_itemToSelect;
        QPointF _lastMousePos;
                
    public:
        LabScene(LabNetwork *_network);
        virtual ~LabScene();
        
        const QGraphicsItem *itemToSelect() const { return _itemToSelect; }
        void deleteSelectedItem();
        
        enum ItemType
        {
            NO_ITEM = 0,
            NODE_ITEM,
            EXCITORY_LINK_ITEM,
            INHIBITORY_LINK_ITEM,

            NUM_ITEM_TYPES
        };

    public slots:
        void newItem(const ItemType type);

    protected:
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *);
                
    private:
        bool mousePressPickupNode(QGraphicsSceneMouseEvent *event);
        
        bool addNode(const QPointF & scenePos);
        bool addLink(const QPointF & scenePos, NeuroLinkItem *linkItem);
        
        bool mouseMoveHandleNode(const QPointF & scenePos, NeuroNodeItem *node);
        bool mouseMoveHandleLinks(const QPointF & scenePos, NeuroLinkItem *link);
        bool canLink(NeuroItem *moving, NeuroItem *fixed);
    };
    
} // namespace NeuroLab

#endif // LABSCENE_H
