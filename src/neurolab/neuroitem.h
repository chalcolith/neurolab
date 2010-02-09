#ifndef NEUROITEM_H
#define NEUROITEM_H

#include "../neurolib/neurocell.h"

#include <QGraphicsItem>
#include <QColor>
#include <QList>

namespace NeuroLab
{
    
    class LabNetwork;
    class NeuroLinkItem;
    
    class NeuroItem 
        : public QGraphicsItem
    {        
        static int NEXT_ID;
        
    protected:
        LabNetwork *_network;
        
        typedef qint64 IdType;
        
        IdType _id;
        bool _in_hover;
        QList<NeuroItem *> _incoming;
        QList<NeuroItem *> _outgoing;
        
        QString _label;
        QRectF  _labelRect;
        
        NeuroLib::NeuroCell::NeuroIndex _cellIndex;
        
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
        static const QColor ACTIVE_COLOR;
        
        static const int NORMAL_LINE_WIDTH;
        static const int HOVER_LINE_WIDTH;
        
        static const int NODE_WIDTH;
        static const int ELLIPSE_WIDTH;
        
        NeuroItem(LabNetwork *network, NeuroLib::NeuroCell::NeuroIndex cellIndex = -1);
        virtual ~NeuroItem();
        
        const QString & label() const { return _label; }
        void setLabel(const QString & s) { _label = s; _labelRect.setWidth(200); update(boundingRect()); }
        
        int id() { return _id; }        
        bool inHover() const { return _in_hover; }
        void setInHover(bool ih) { _in_hover = ih; update(boundingRect()); }
        
        const QList<NeuroItem *> incoming() const { return _incoming; }
        const QList<NeuroItem *> outgoing() const { return _outgoing; }
        
        void bringToFront();
        
        virtual void addIncoming(NeuroItem *linkItem);
        virtual void removeIncoming(NeuroItem *linkItem);
        
        virtual void addOutgoing(NeuroItem *linkItem);
        virtual void removeOutgoing(NeuroItem *linkItem);
        
        virtual void adjustLinks() = 0;
        
        virtual void idsToPointers(QGraphicsScene *);
        
        virtual QRectF boundingRect() const;
        virtual QPainterPath shape() const;

        virtual void reset();
        virtual void activate();
        virtual void deactivate();
        virtual void toggleFrozen();
                
    protected:
        NeuroLib::NeuroCell *getCell();
        
        virtual void setPenWidth(QPen & pen);
        virtual void setPenColor(QPen & pen);
        virtual void drawLabel(QPainter *painter, QPen & pen, QBrush & brush);
        
        virtual bool shouldHighlight() const;
        
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        
        virtual void writeBinary(QDataStream &) const = 0;
        virtual void readBinary(QDataStream &) = 0;
        
        virtual void writePointerIds(QDataStream &) const;
        virtual void readPointerIds(QDataStream &);
        
        virtual void idsToPointersAux(QList<NeuroItem *> & list, QGraphicsScene *sc);
        
        friend QDataStream & operator<< (QDataStream &, const NeuroItem &);
        friend QDataStream & operator>> (QDataStream &, NeuroItem &);
    };
    
    extern QDataStream & operator<< (QDataStream &, const NeuroItem &);
    extern QDataStream & operator>> (QDataStream &, NeuroItem &);
    
} // namespace NeuroLab

#endif // NEUROITEM_H
