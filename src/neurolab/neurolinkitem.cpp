#include "neurolinkitem.h"

#include <QPainter>
#include <QPainterPathStroker>
#include <QPolygonF>
#include <QVector2D>
#include <cmath>

namespace NeuroLab
{
    
    //////////////////////////////////////////////////////////////////

    NeuroLinkItem::NeuroLinkItem()
        : NeuroItem(), _frontLinkTarget(0), _backLinkTarget(0)
    {
    }
    
    NeuroLinkItem::~NeuroLinkItem()
    {
        if (_frontLinkTarget)
            _frontLinkTarget->removeIncoming(this);
        _frontLinkTarget = 0;
        
        if (_backLinkTarget)
            _backLinkTarget->removeOutgoing(this);
        _backLinkTarget = 0;
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
    
    void NeuroLinkItem::setLine(const QPointF &p1, const QPointF &p2)
    {
        prepareGeometryChange();
        _line = QLineF(p1, p2);
        updatePos();
    }

    void NeuroLinkItem::setFrontLinkTarget(NeuroItem *linkTarget)
    {
        if (_frontLinkTarget)
        {
            _frontLinkTarget->removeIncoming(this);
        }
        
        if (linkTarget)
        {
            linkTarget->addIncoming(this);
        }
        
        _frontLinkTarget = linkTarget;
    }
    
    void NeuroLinkItem::setBackLinkTarget(NeuroItem *linkTarget)
    {
        if (_backLinkTarget)
        {
            _backLinkTarget->removeOutgoing(this);
        }
        
        if (linkTarget)
        {
            linkTarget->addOutgoing(this);
        }
        
        _backLinkTarget = linkTarget;
    }
    
    void NeuroLinkItem::updatePos()
    {
        setPos( (_line.x1() + _line.x2())/2, (_line.y1() + _line.y2())/2 );
    }
    
    QRectF NeuroLinkItem::boundingRect() const
    {
        qreal w = qAbs(_line.x2() - _line.x1());
        qreal h = qAbs(_line.y2() - _line.y1());
        
        return QRectF(-(w/2.0+ELLIPSE_WIDTH), -(h/2.0+ELLIPSE_WIDTH), w+ELLIPSE_WIDTH*2, h+ELLIPSE_WIDTH*2);
    }
    
    void NeuroLinkItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        painter->setRenderHint(QPainter::Antialiasing);
        
        int x1 = static_cast<int>(_line.x1() - pos().x());
        int y1 = static_cast<int>(_line.y1() - pos().y());
        int x2 = static_cast<int>(_line.x2() - pos().x());
        int y2 = static_cast<int>(_line.y2() - pos().y());

        QPen pen(Qt::SolidLine);
        pen.setColor(NORMAL_LINE_COLOR);
        
        QBrush brush(Qt::SolidPattern);
        brush.setColor(BACKGROUND_COLOR);
        
        setPenWidth(pen);        
        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawLine(x1, y1, x2, y2);
    }
    
    QPainterPath NeuroLinkItem::shape() const
    {
        QPainterPath path;
        
        path.setFillRule(Qt::WindingFill);
        path.moveTo(_line.p1() - pos());
        path.lineTo(_line.p2() - pos());
        
        QPainterPathStroker stroker;        
        stroker.setWidth(ELLIPSE_WIDTH);
                
        QPainterPath path2 = stroker.createStroke(path);
        path2.setFillRule(Qt::WindingFill);
        path2.addPath(path);
        return path2;
    }
    
    void NeuroLinkItem::adjustLinks()
    {
        QVector2D myFront(_line.p2());
        QVector2D myBack(_line.p1());
        QVector2D frontToBack(myBack - myFront);
        qreal myLength = frontToBack.length();
        frontToBack.normalize();
        
        int numLinks = _incoming.length();
        for (int i = 0; i < numLinks; ++i)
        {
            NeuroLinkItem *link = _incoming[i];
            if (link)
                link->setLine(link->line().p1(), (myFront + (frontToBack * (1 * myLength/2.0))).toPointF());
        }
    }
    
    void NeuroLinkItem::setPenWidth(QPen & pen)
    {
        if (_in_hover)
            pen.setWidth(HOVER_LINE_WIDTH);
        else   
            pen.setWidth(NORMAL_LINE_WIDTH);
    }
    
    void NeuroLinkItem::paintBackLink(QPainter *painter)
    {        
        if (_backLinkTarget)
        {
            int x1 = static_cast<int>(_line.x1() - pos().x());
            int y1 = static_cast<int>(_line.y1() - pos().y());
            
            QPen pen(Qt::SolidLine);
            
            if (_backLinkTarget)
                pen.setColor(NORMAL_LINE_COLOR);
            else
                pen.setColor(UNLINKED_LINE_COLOR);
            
            setPenWidth(pen);
            painter->drawEllipse(x1 - ELLIPSE_WIDTH/4, y1 - ELLIPSE_WIDTH/4, ELLIPSE_WIDTH/2, ELLIPSE_WIDTH/2);
        }
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
        NeuroLinkItem::paint(painter, option, widget);

        int x2 = static_cast<int>(_line.x2() - pos().x());
        int y2 = static_cast<int>(_line.y2() - pos().y());
        
        QPen pen(Qt::SolidLine);
        if (_frontLinkTarget)
            pen.setColor(NORMAL_LINE_COLOR);
        else
            pen.setColor(UNLINKED_LINE_COLOR);
        
        setPenWidth(pen);
        painter->setPen(pen);
        painter->drawEllipse(x2 - ELLIPSE_WIDTH/2, y2 - ELLIPSE_WIDTH/2, ELLIPSE_WIDTH, ELLIPSE_WIDTH);
        
        paintBackLink(painter);
    }
    
    QPainterPath NeuroExcitoryLinkItem::shape() const
    {
        QPainterPath path;
        
        path.setFillRule(Qt::WindingFill);
        path.addEllipse(_line.p1() - pos(), ELLIPSE_WIDTH, ELLIPSE_WIDTH);
        path.addEllipse(_line.p2() - pos(), ELLIPSE_WIDTH, ELLIPSE_WIDTH);
        path.addPath(NeuroLinkItem::shape());
        
        return path;
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
        NeuroLinkItem::paint(painter, option, widget);
        
        QBrush brush(Qt::NoBrush);
        
        QPen pen(Qt::SolidLine);
        if (_frontLinkTarget)
            pen.setColor(NORMAL_LINE_COLOR);
        else
            pen.setColor(UNLINKED_LINE_COLOR);
        setPenWidth(pen);
        
        int x2 = static_cast<int>(_line.x2() - pos().x());
        int y2 = static_cast<int>(_line.y2() - pos().y());
        
        QRectF r(x2 - ELLIPSE_WIDTH/2, y2 - ELLIPSE_WIDTH/2, ELLIPSE_WIDTH, ELLIPSE_WIDTH);

        qreal theta = ::atan2(-y2, x2);
        if (theta < 0)
            theta = (2.0 * M_PI) + theta;
        theta += M_PI / 2.0;
        
        int angle = static_cast<int>(theta * 180.0 / M_PI);
        
        painter->setBrush(brush);
        painter->setPen(pen);
        //painter->drawChord(r, 16 * angle, 16 * 180);
        painter->drawArc(r, 16 * angle, 16 * 180);
        
        paintBackLink(painter);
    }
    
    QPainterPath NeuroInhibitoryLinkItem::shape() const
    {
        QPainterPath path;
        
        path.setFillRule(Qt::WindingFill);
        path.addEllipse(_line.p1() - pos(), ELLIPSE_WIDTH, ELLIPSE_WIDTH);
        path.addEllipse(_line.p2() - pos(), ELLIPSE_WIDTH, ELLIPSE_WIDTH);
        path.addPath(NeuroLinkItem::shape());
        
        return path;
    }
    
} // namespace NeuroLab
