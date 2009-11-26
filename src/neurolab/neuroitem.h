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
        QList<NeuroLinkItem *> _outgoing;
        
        static const QColor NORMAL_LINE_COLOR;
        static const QColor UNLINKED_LINE_COLOR;
        static const QColor BACKGROUND_COLOR;
        
        static const int NORMAL_LINE_WIDTH;
        static const int HOVER_LINE_WIDTH;
        
        static const int NODE_WIDTH;
        static const int ELLIPSE_WIDTH;
        
    public:
        NeuroItem();
        virtual ~NeuroItem();
        
        bool inHover() const { return _in_hover; }
        void setInHover(bool ih) { _in_hover = ih; update(boundingRect()); }
        
        const QList<NeuroLinkItem *> incoming() const { return _incoming; }
        const QList<NeuroLinkItem *> outgoing() const { return _outgoing; }
        
        void bringToFront();
        
        void addIncoming(NeuroLinkItem *linkItem);
        void removeIncoming(NeuroLinkItem *linkItem);
        
        void addOutgoing(NeuroLinkItem *linkItem);
        void removeOutgoing(NeuroLinkItem *linkItem);
                
        virtual void adjustLinks() = 0;
        
    protected:
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    };
    
} // namespace NeuroLab

#endif // NEUROITEM_H
