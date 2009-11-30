#ifndef NEURONODEITEM_H
#define NEURONODEITEM_H

#include "neuroitem.h"

#include <QPainter>
#include <QPainterPath>

namespace NeuroLab
{
    
    class NeuroNodeItem
        : public NeuroItem
    {
        QRectF _rect;
        
    public:
        NeuroNodeItem();
        virtual ~NeuroNodeItem();
        
        const QRectF & rect() const { return _rect; }
        void setRect(const QRectF & r) { _rect = r; }
        
        virtual QRectF boundingRect() const;
        
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        virtual QPainterPath shape() const;
        
        virtual void adjustLinks();
        
    private:
        void adjustLinksAux(QList<NeuroLinkItem *> & list);
    };
    
}

#endif // NEURONODEITEM_H
