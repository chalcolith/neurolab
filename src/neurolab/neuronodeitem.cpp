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
    
    static void remove_links(QList<NeuroLinkItem *> & list, NeuroNodeItem *node)
    {
        for (QListIterator<NeuroLinkItem *> ln(list); ln.hasNext(); ln.next())
        {
            NeuroLinkItem *link = ln.peekNext();
            
            if (link && link->frontLinkTarget() == node)
                link->setFrontLinkTarget(0);
            else if (link && link->backLinkTarget() == node)
                link->setBackLinkTarget(0);
        }
        
        list.clear();
    }
    
    NeuroNodeItem::~NeuroNodeItem()
    {
        remove_links(_incoming, this);
        remove_links(_outgoing, this);
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
        
        if (shouldHighlight())
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
    
    void NeuroNodeItem::adjustLinks()
    {
        adjustLinksAux(_incoming);
        adjustLinksAux(_outgoing);        
    }
    
    void NeuroNodeItem::adjustLinksAux(QList<NeuroLinkItem *> & list)
    {
        for (QListIterator<NeuroLinkItem *> ln(list); ln.hasNext(); ln.next())
        {
            NeuroLinkItem *link = ln.peekNext();
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
    
    void NeuroNodeItem::writeBinary(QDataStream & data) const
    {
        data << _id;
        data << pos();
        data << _rect;
    }
    
    void NeuroNodeItem::readBinary(QDataStream & data)
    {
        data >> _id;
        
        QPointF p;
        data >> p;
        setPos(p);
        
        QRectF r;
        data >> r;
        setRect(r);
    }
    
} // namespace NeuroLab
