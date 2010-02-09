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
        Q_OBJECT
        
        LabNetwork *_network;

        NeuroNodeItem *movingNode;
        NeuroLinkItem *movingLink;
        bool linkFront;
        
        NeuroItem *_selectedItem;
        QPointF _lastMousePos;
                
    public:
        LabScene(LabNetwork *_network);
        virtual ~LabScene();
        
        LabNetwork *network() { return _network; }
        NeuroItem *selectedItem() const { return _selectedItem; }
        void setSelectedItem(NeuroItem *item) { if (_selectedItem) _selectedItem->update(); _selectedItem = item; }
        
        void deleteSelectedItem();
        void labelSelectedItem(const QString & s);
        
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
        
        bool addNode(const QPointF & scenePos, NeuroNodeItem *nodeItem);
        bool addLink(const QPointF & scenePos, NeuroLinkItem *linkItem);
        
        bool mouseMoveHandleNode(const QPointF & scenePos, NeuroNodeItem *node);
        bool mouseMoveHandleLinks(const QPointF & scenePos, NeuroLinkItem *link);
        bool canLink(NeuroItem *moving, NeuroItem *fixed);
    };
    
} // namespace NeuroLab

#endif // LABSCENE_H
