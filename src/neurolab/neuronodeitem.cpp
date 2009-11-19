#include "neuronodeitem.h"

namespace NeuroLab
{
    
    static const int NODE_WIDTH = 20;
    
    NeuroNodeItem::NeuroNodeItem()
        : NeuroItem()
    {
        setRect(QRectF(-NODE_WIDTH/2, -NODE_WIDTH/2, NODE_WIDTH, NODE_WIDTH));
    }

    QRectF NeuroNodeItem::boundingRect() const
    {
        return _rect;
    }
    
    void NeuroNodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        QPen pen(Qt::SolidLine);
        pen.setColor(Qt::black);
        
        QBrush brush(Qt::SolidPattern);
        brush.setColor(Qt::white);
        
        if (_in_hover || isSelected())
        {
            pen.setWidth(3);
        }
        else
        {
            pen.setWidth(1);
        }
        
        painter->setBrush(brush);
        painter->setPen(pen);
        painter->drawEllipse(rect());
    }
    
    QPainterPath NeuroNodeItem::shape() const
    {
        QPainterPath path;
        path.addEllipse(rect());
        return path;
    }
    
} // namespace NeuroLab
