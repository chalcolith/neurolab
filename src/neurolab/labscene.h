#ifndef LABSCENE_H
#define LABSCENE_H

#include "neuronodeitem.h"
#include "neurolinkitem.h"

#include <QGraphicsScene>
#include <QStack>

namespace NeuroLab
{
    
    class LabScene 
        : public QGraphicsScene
    {
        NeuroNodeItem *movingNode;
        NeuroLinkItem *movingLink;
        bool linkFront;
                
    public:
        LabScene();
        virtual ~LabScene();
        
        enum Mode
        {
            MODE_NONE = 0,
            MODE_ADD_NODE,
            MODE_ADD_E_LINK,
            MODE_ADD_I_LINK,
            MODE_MOVE_NODE,
            MODE_MOVE_LINK,

            MODE_NUM_MODES
        };
        
        void setMode(const Mode &);
        const Mode getMode() const;
        
        void deleteSelectedItem();
                
    protected:
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *);
                
    private:
        QStack<Mode> mode;
        
        bool mousePressPickupNode(QGraphicsSceneMouseEvent *event);
        bool mousePressAddNode(QGraphicsSceneMouseEvent *event);
        bool mousePressAddLink(QGraphicsSceneMouseEvent *event, NeuroLinkItem *linkItem);
        
        bool mouseMoveHandleLinks(QGraphicsSceneMouseEvent *event, QPointF & scenePos, NeuroLinkItem *link);
        bool canLink(NeuroItem *moving, NeuroItem *fixed);
    };
    
} // namespace NeuroLab

#endif // LABSCENE_H
