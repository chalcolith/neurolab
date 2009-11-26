#ifndef NEUROLINKITEM_H
#define NEUROLINKITEM_H

#include "neuroitem.h"

#include <QPainterPath>
#include <QList>

namespace NeuroLab
{
    
    class NeuroLinkItem
        : public NeuroItem
    {
    protected:
        QLineF _line;
        
        NeuroItem *_frontLinkTarget, *_backLinkTarget;
        
    public:
        NeuroLinkItem();
        virtual ~NeuroLinkItem();
        
        QLineF line() const { return _line; }
        void setLine(const QLineF & l);
        void setLine(const qreal & x1, const qreal & y1, const qreal & x2, const qreal & y2);
        void setLine(const QPointF & p1, const QPointF & p2);
        
        NeuroItem *frontLinkTarget() { return _frontLinkTarget; }
        void setFrontLinkTarget(NeuroItem *linkTarget);
        
        NeuroItem *backLinkTarget() { return _backLinkTarget; }
        void setBackLinkTarget(NeuroItem *linkTarget);
        
        virtual QRectF boundingRect() const;
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        virtual QPainterPath shape() const;
        
        virtual void adjustIncomingLinks();
        
    private:
        void updatePos();
        
    protected:
        void setPenWidth(QPen & pen);
        void paintBackLink(QPainter *painter);
    };
    
    
    class NeuroExcitoryLinkItem
        : public NeuroLinkItem
    {
    public:
        NeuroExcitoryLinkItem();
        virtual ~NeuroExcitoryLinkItem();        

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        virtual QPainterPath shape() const;
    };
    
    
    class NeuroInhibitoryLinkItem
        : public NeuroLinkItem
    {
    public:
        NeuroInhibitoryLinkItem();
        virtual ~NeuroInhibitoryLinkItem();

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        virtual QPainterPath shape() const;
    };
    
} // namespace NeuroLab

#endif // NEUROLINKITEM_H
