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
        static int NEXT_ID;
        
    protected:
        int _id;
        bool _in_hover;
        QList<NeuroLinkItem *> _incoming;
        QList<NeuroLinkItem *> _outgoing;
        
    public:
        enum NODE_TYPE
        {
            NEURO_NULL = 0,
            NEURO_NODE,
            NEURO_EXCITORY_LINK,
            NEURO_INHIBITORY_LINK
        };
        
        static const QColor NORMAL_LINE_COLOR;
        static const QColor UNLINKED_LINE_COLOR;
        static const QColor BACKGROUND_COLOR;
        
        static const int NORMAL_LINE_WIDTH;
        static const int HOVER_LINE_WIDTH;
        
        static const int NODE_WIDTH;
        static const int ELLIPSE_WIDTH;
        
        NeuroItem();
        virtual ~NeuroItem();
        
        int id() { return _id; }        
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
        
        virtual void idsToPointers(QGraphicsScene *);

    protected:
        virtual bool shouldHighlight() const;
        
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        
        virtual void writeBinary(QDataStream &) const = 0;
        virtual void readBinary(QDataStream &) = 0;
        
        virtual void writePointerIds(QDataStream &) const;
        virtual void readPointerIds(QDataStream &);
        
        virtual void idsToPointersAux(QList<NeuroLinkItem *> list, QGraphicsScene *sc);
        
        friend QDataStream & operator<< (QDataStream &, const NeuroItem &);
        friend QDataStream & operator>> (QDataStream &, NeuroItem &);
    };
    
    extern QDataStream & operator<< (QDataStream &, const NeuroItem &);
    extern QDataStream & operator>> (QDataStream &, NeuroItem &);
    
} // namespace NeuroLab

#endif // NEUROITEM_H
