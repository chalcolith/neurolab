#ifndef LABSCENE_H
#define LABSCENE_H

#include "neuroitem.h"

#include <QGraphicsScene>
#include <QStack>

namespace NeuroLab
{
    
    class LabNetwork;
    class NeuroNodeItem;
    class NeuroLinkItem;

    class LabScene 
        : public QGraphicsScene
    {
        Q_OBJECT
        
        LabNetwork *_network;

        NeuroItem::EditInfo editInfo;        
        NeuroItem *_itemUnderMouse;
        NeuroItem *_selectedItem;
                
    public:
        LabScene(LabNetwork *_network);
        virtual ~LabScene();
        
        LabNetwork *network() { return _network; }
        
        NeuroItem *itemUnderMouse() const { return _itemUnderMouse; }
        void setItemUnderMouse(NeuroItem *item) { _itemUnderMouse = item; }
        
        NeuroItem *selectedItem() const { return _selectedItem; }
        void setSelectedItem(NeuroItem *item);
        
        void deleteSelectedItem();
        void labelSelectedItem(const QString & s);
        
    public slots:
        void newItem(const QString & typeName);

    protected:
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *);
                
    private:
        bool mousePressPickupNode(QGraphicsSceneMouseEvent *event);
    };
    
} // namespace NeuroLab

#endif // LABSCENE_H
