#include "neuronodeitem.h"
#include "neurolinkitem.h"

#include <QVector2D>

namespace NeuroLab
{
        
    NeuroNodeItem::NeuroNodeItem()
        : NeuroItem()
    {
        setRect(QRectF(-NODE_WIDTH/2, -NODE_WIDTH/2, NODE_WIDTH, NODE_WIDTH));
    }
    
    NeuroNodeItem::~NeuroNodeItem()
    {
    }

    QRectF NeuroNodeItem::boundingRect() const
    {
        return _rect;
    }
    
    void NeuroNodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        painter->setRenderHint(QPainter::Antialiasing);

        QPen pen(Qt::SolidLine);
        pen.setColor(NORMAL_LINE_COLOR);
        
        QBrush brush(Qt::SolidPattern);
        brush.setColor(BACKGROUND_COLOR);
        
        if (_in_hover)
            pen.setWidth(HOVER_LINE_WIDTH);
        else
            pen.setWidth(NORMAL_LINE_WIDTH);
        
        painter->setBrush(brush);
        painter->setPen(pen);
        painter->drawEllipse(rect());
    }
    
    QPainterPath NeuroNodeItem::shape() const
    {
        QPainterPath path;
        path.addEllipse(QPointF(), NODE_WIDTH/2.0 + 2.0, NODE_WIDTH/2.0 + 2.0);
        return path;
    }
    
    void NeuroNodeItem::adjustIncomingLinks()
    {
        QListIterator<NeuroLinkItem *> i(_incoming);
        while (i.hasNext())
        {
            NeuroLinkItem *link = i.next();
            if (!link)
                continue;
            
            QVector2D center(pos());
            QVector2D front(link->line().p2());
            QVector2D back(link->line().p1());
            
            qreal front_dist = (center - front).lengthSquared();
            qreal back_dist = (center - back).lengthSquared();
            
            QVector2D & pointToAdjust = front_dist < back_dist ? front : back;
            QVector2D & pointSource = front_dist < back_dist ? back : front;

            QVector2D toSource = pointSource - center;
            toSource.normalize();
            
            pointToAdjust = center + (toSource * (NODE_WIDTH / 1.8));
            link->setLine(back.toPointF(), front.toPointF());
        }
    }
    
} // namespace NeuroLab
