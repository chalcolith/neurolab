#ifndef NEUROITEM_H
#define NEUROITEM_H

#include <QGraphicsItem>
#include <QBrush>
#include <QPen>

namespace NeuroLab
{
    
    class NeuroItem 
        : public QGraphicsItem
    {
    public:
        NeuroItem();
        
    protected:
        bool _in_hover;
        
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    };
    
} // namespace NeuroLab

#endif // NEUROITEM_H
