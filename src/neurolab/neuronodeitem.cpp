#include "neuronodeitem.h"
#include "neurolinkitem.h"
#include "labnetwork.h"

#include <QVector2D>

using namespace NeuroLib;

namespace NeuroLab
{

    NeuroNodeItem::NeuroNodeItem(LabNetwork *network, NeuroLib::NeuroCell::NeuroIndex cellIndex)
        : NeuroItem(network, cellIndex)
    {
        setRect(QRectF(-NODE_WIDTH/2, -NODE_WIDTH/2, NODE_WIDTH, NODE_WIDTH));
    }
        
    NeuroNodeItem::~NeuroNodeItem()
    {
    }
    
    QRectF NeuroNodeItem::boundingRect() const
    {
        return _rect.united(NeuroItem::boundingRect());
    }
    
    void NeuroNodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
    {
        painter->setRenderHint(QPainter::Antialiasing);

        QPen pen(Qt::SolidLine);
        pen.setColor(NORMAL_LINE_COLOR);
        
        QBrush brush(Qt::SolidPattern);
        brush.setColor(BACKGROUND_COLOR);
        
        drawLabel(painter, pen, brush);
        
        setPenWidth(pen);
        setPenColor(pen);
        painter->setBrush(brush);
        painter->setPen(pen);
        painter->drawEllipse(rect());
        
        NeuroCell *cell = getCell();
        if (cell)
        {
            int threshold = static_cast<int>(cell->input_threshold() + static_cast<NeuroCell::NeuroValue>(0.5));
            QString str("%1");
            painter->drawText(-4, 3, str.arg(threshold));
        }
    }
    
    QPainterPath NeuroNodeItem::shape() const
    {
        QPainterPath path = NeuroItem::shape();
        path.addEllipse(QPointF(), NODE_WIDTH/2.0 + 2.0, NODE_WIDTH/2.0 + 2.0);
        return path;
    }
    
    void NeuroNodeItem::adjustLinks()
    {
        adjustLinksAux(_incoming);
        adjustLinksAux(_outgoing);        
    }
    
    void NeuroNodeItem::adjustLinksAux(QList<NeuroItem *> & list)
    {
        for (QListIterator<NeuroItem *> ln(list); ln.hasNext(); ln.next())
        {
            NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(ln.peekNext());
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
