#ifndef NEUROITEM_H
#define NEUROITEM_H

#include <QGraphicsItem>
#include <QColor>
#include <QList>

namespace NeuroLab
{
    
    class NeuroLinkItem;
    
    class NeuroItem 
        : public QGraphicsItem
    {
    protected:
        bool _in_hover;
        QList<NeuroLinkItem *> _incoming;
        
        static const QColor NORMAL_LINE_COLOR;
        static const QColor UNLINKED_LINE_COLOR;
        static const QColor BACKGROUND_COLOR;
        
        static const int NORMAL_LINE_WIDTH;
        static const int HOVER_LINE_WIDTH;
        
        static const int NODE_WIDTH;
        static const int ELLIPSE_WIDTH;
        
    public:
        NeuroItem();
        
        void addIncoming(NeuroLinkItem *linkItem);
        void removeIncoming(NeuroLinkItem *linkItem);
                
        virtual void adjustIncomingLinks() = 0;
        
    protected:
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    };
    
} // namespace NeuroLab

#endif // NEUROITEM_H
