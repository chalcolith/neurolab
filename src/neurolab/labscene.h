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
        NeuroItem *_itemUnderMouse;
        
        QPointF _lastMousePos;
                
    public:
        LabScene(LabNetwork *_network);
        virtual ~LabScene();
        
        LabNetwork *network() { return _network; }
        const QPointF & lastMousePos() const { return _lastMousePos; }
        
        NeuroItem *itemUnderMouse() const { return _itemUnderMouse; }
        void setItemUnderMouse(NeuroItem *item) { _itemUnderMouse = item; }
                
    public slots:
        void newItem(const QString & typeName, const QPointF & scenePos);
        
    protected:
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    };
    
} // namespace NeuroLab

#endif // LABSCENE_H
