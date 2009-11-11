#ifndef NEUROLINKITEM_H
#define NEUROLINKITEM_H

#include "neuroitem.h"

namespace NeuroLab
{
    
    class NeuroLinkItem 
            : public NeuroItem
    {
    protected:
        QLineF _line;
        
    public:
        NeuroLinkItem();
        virtual ~NeuroLinkItem();
        
        QLineF line() const { return _line; }
        void setLine(const QLineF & l);
        void setLine(const qreal & x1, const qreal & y1, const qreal & x2, const qreal & y2);
        
        virtual QRectF boundingRect() const;
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        
    private:
        void updatePos();
    };
    
    
    class NeuroExcitoryLinkItem
            : public NeuroLinkItem
    {
    public:
        NeuroExcitoryLinkItem();
        virtual ~NeuroExcitoryLinkItem();        

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    };
    
    
    class NeuroInhibitoryLinkItem
            : public NeuroLinkItem
    {
    public:
        NeuroInhibitoryLinkItem();
        virtual ~NeuroInhibitoryLinkItem();

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    };
    
} // namespace NeuroLab

#endif // NEUROLINKITEM_H
