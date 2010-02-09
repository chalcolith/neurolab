#include "neurolinkitem.h"
#include "labnetwork.h"
#include "../neurolib/neuronet.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QPainterPathStroker>
#include <QPolygonF>
#include <QVector2D>

#define _USE_MATH_DEFINES
#include <cmath>

using namespace NeuroLib;

namespace NeuroLab
{
    
    //////////////////////////////////////////////////////////////////

    NeuroLinkItem::NeuroLinkItem(LabNetwork *network, NeuroLib::NeuroCell::NeuroIndex cellIndex)
        : NeuroItem(network, cellIndex), _frontLinkTarget(0), _backLinkTarget(0)
    {
    }
    
    NeuroLinkItem::~NeuroLinkItem()
    {
        for (QListIterator<NeuroLinkItem *> i(_incoming); i.hasNext(); i.next())
        {
            NeuroLinkItem *link = i.peekNext();
            if (link && link->frontLinkTarget() == this)
                link->setFrontLinkTarget(0);
            else if (link && link->backLinkTarget() == this)
                link->setBackLinkTarget(0);
        }
        
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
        adjustLinks();
    }
    
    QRectF NeuroLinkItem::boundingRect() const
    {
        qreal w = qAbs(_line.x2() - _line.x1());
        qreal h = qAbs(_line.y2() - _line.y1());
        
        QRectF result(-(w/2.0+ELLIPSE_WIDTH), -(h/2.0+ELLIPSE_WIDTH), w+ELLIPSE_WIDTH*2, h+ELLIPSE_WIDTH*2);

        return result.united(NeuroItem::boundingRect());
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
        
        drawLabel(painter, pen, brush);
        
        setPenWidth(pen);       
        setPenColor(pen);
        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawLine(x1, y1, x2, y2);
    }
    
    QPainterPath NeuroLinkItem::shape() const
    {
        QPainterPath path = NeuroItem::shape();
        
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
    
    void NeuroLinkItem::paintBackLink(QPainter *painter)
    {        
        if (_backLinkTarget)
        {
            int x1 = static_cast<int>(_line.x1() - pos().x());
            int y1 = static_cast<int>(_line.y1() - pos().y());
            
            QPen pen(Qt::SolidLine);            
            pen.setColor(NORMAL_LINE_COLOR);
            painter->setPen(pen);
            setPenWidth(pen);
            painter->drawEllipse(x1 - ELLIPSE_WIDTH/4, y1 - ELLIPSE_WIDTH/4, ELLIPSE_WIDTH/2, ELLIPSE_WIDTH/2);
        }
    }
    
    void NeuroLinkItem::writeBinary(QDataStream & data) const
    {
        data << _id;
        data << pos();
        data << _line;
    }
    
    void NeuroLinkItem::readBinary(QDataStream & data)
    {
        data >> _id;
        
        QPointF p;
        data >> p;
        setPos(p);
        
        QLineF l;
        data >> l;
        setLine(l);
    }
    
    void NeuroLinkItem::writePointerIds(QDataStream & data) const
    {
        NeuroItem::writePointerIds(data);
        
        data << (_frontLinkTarget ? _frontLinkTarget->id() : 0);
        data << (_backLinkTarget ? _backLinkTarget->id() : 0);
    }
    
    void NeuroLinkItem::readPointerIds(QDataStream & data)
    {
        NeuroItem::readPointerIds(data);
        
        setFrontLinkTarget(0);
        setBackLinkTarget(0);

        int id;
        data >> id;
        
        if (id)
            _frontLinkTarget = (NeuroItem *) id;
        
        data >> id;
        
        if (id)
            _backLinkTarget = (NeuroItem *) id;
    }
    
    void NeuroLinkItem::idsToPointers(QGraphicsScene *sc)
    {
        NeuroItem::idsToPointers(sc);
        
        int frontId = (int) _frontLinkTarget;
        int backId = (int) _backLinkTarget;
        
        for (QListIterator<QGraphicsItem *> i(sc->items()); i.hasNext(); )
        {
            NeuroItem *item = dynamic_cast<NeuroItem *>(i.next());
            if (item)
            {
                if (item->id() == frontId)
                    _frontLinkTarget = item;
                else if (item->id() == backId)
                    _backLinkTarget = item;
            }
        }
    }
    
    
    //////////////////////////////////////////////////////////////////
    
    NeuroExcitoryLinkItem::NeuroExcitoryLinkItem(LabNetwork *network, NeuroLib::NeuroCell::NeuroIndex cellIndex)
        : NeuroLinkItem(network, cellIndex)
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

    NeuroInhibitoryLinkItem::NeuroInhibitoryLinkItem(LabNetwork *network, NeuroLib::NeuroCell::NeuroIndex cellIndex)
        : NeuroLinkItem(network, cellIndex)
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

        qreal theta = ::atan2((double)-y2, (double)x2);
        if (theta < 0)
			theta = (2.0 * M_PI) + theta;
        theta += M_PI / 2.0;
        
        int angle = static_cast<int>(theta * 180.0 / M_PI);
        
        painter->setBrush(brush);
        painter->setPen(pen);
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
