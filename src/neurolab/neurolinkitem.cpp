#include "neurolinkitem.h"

#include <QPainter>

namespace NeuroLab
{
    
    //////////////////////////////////////////////////////////////////

    static const int ELLIPSE_WIDTH = 8;
    
    NeuroLinkItem::NeuroLinkItem()
            : QGraphicsItem()
    {
    }
    
    NeuroLinkItem::~NeuroLinkItem()
    {
    }
    
    void NeuroLinkItem::setLine(const QLineF & l)
    { 
        prepareGeometryChange();
        _line = l;
        updatePos(); 
    }
    
    void NeuroLinkItem::setLine(const qreal & x1, const qreal & y1, const qreal & x2, const qreal & y2)
    { 
        prepareGeometryChange();
        _line = QLineF(x1, y1, x2, y2);
        updatePos(); 
    }

    void NeuroLinkItem::updatePos()
    {
        setPos( (_line.x1() + _line.x2())/2, (_line.y1() + _line.y2())/2 );
    }
    
    QRectF NeuroLinkItem::boundingRect() const
    {
        qreal w = qAbs(_line.x2() - _line.x1());
        qreal h = qAbs(_line.y2() - _line.y1());
        
        return QRectF(-(w/2+ELLIPSE_WIDTH/2+1), -(h/2+ELLIPSE_WIDTH/2+1), w+ELLIPSE_WIDTH+2, h+ELLIPSE_WIDTH+2);
    }
    
    void NeuroLinkItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        int x1 = static_cast<int>(_line.x1() - pos().x());
        int y1 = static_cast<int>(_line.y1() - pos().y());
        int x2 = static_cast<int>(_line.x2() - pos().x());
        int y2 = static_cast<int>(_line.y2() - pos().y());

        painter->setBrush(Qt::SolidPattern);
        painter->drawLine(x1, y1, x2, y2);
    }
    
    
    //////////////////////////////////////////////////////////////////
    
    NeuroExcitoryLinkItem::NeuroExcitoryLinkItem()
            : NeuroLinkItem()
    {
    }
    
    NeuroExcitoryLinkItem::~NeuroExcitoryLinkItem()
    {
    }
    
    void NeuroExcitoryLinkItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        int x1 = static_cast<int>(_line.x1() - pos().x());
        int y1 = static_cast<int>(_line.y1() - pos().y());
        int x2 = static_cast<int>(_line.x2() - pos().x());
        int y2 = static_cast<int>(_line.y2() - pos().y());
        
        NeuroLinkItem::paint(painter, option, widget);
        painter->drawEllipse(x2 - ELLIPSE_WIDTH/2, y2 - ELLIPSE_WIDTH/2, ELLIPSE_WIDTH, ELLIPSE_WIDTH);
    }
    
    
    //////////////////////////////////////////////////////////////////

    NeuroInhibitoryLinkItem::NeuroInhibitoryLinkItem()
            : NeuroLinkItem()
    {
    }
    
    NeuroInhibitoryLinkItem::~NeuroInhibitoryLinkItem()
    {
    }
    
    void NeuroInhibitoryLinkItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        int x1 = static_cast<int>(_line.x1() - pos().x());
        int y1 = static_cast<int>(_line.y1() - pos().y());
        int x2 = static_cast<int>(_line.x2() - pos().x());
        int y2 = static_cast<int>(_line.y2() - pos().y());
        
        NeuroLinkItem::paint(painter, option, widget);
        //painter->drawEllipse(x2, y2, 4, 4);
    }
    
} // namespace NeuroLab
