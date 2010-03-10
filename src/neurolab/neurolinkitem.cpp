#include "neurolinkitem.h"
#include "labnetwork.h"
#include "../neurolib/neuronet.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QPainterPathStroker>
#include <QPolygonF>
#include <QVector2D>

#include <QtProperty>

#define _USE_MATH_DEFINES
#include <cmath>

using namespace NeuroLib;

namespace NeuroLab
{
    
    //////////////////////////////////////////////////////////////////

    NeuroLinkItem::NeuroLinkItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex)
        : NeuroItem(network, cellIndex), _frontLinkTarget(0), _backLinkTarget(0)
    {
    }
    
    NeuroLinkItem::~NeuroLinkItem()
    {
        _frontLinkTarget = 0;
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
    
    void NeuroLinkItem::addIncoming(NeuroItem *linkItem)
    {
        NeuroItem::addIncoming(linkItem);
    }
    
    void NeuroLinkItem::removeIncoming(NeuroItem *linkItem)
    {
        if (linkItem && linkItem == _backLinkTarget)
            setBackLinkTarget(0);
        NeuroItem::removeIncoming(linkItem);
    }
    
    void NeuroLinkItem::addOutgoing(NeuroItem *linkItem)
    {
        NeuroItem::addOutgoing(linkItem);
    }
    
    void NeuroLinkItem::removeOutgoing(NeuroItem *linkItem)
    {
        if (linkItem && linkItem == _frontLinkTarget)
            setFrontLinkTarget(0);
        NeuroItem::removeOutgoing(linkItem);
    }

    void NeuroLinkItem::setFrontLinkTarget(NeuroItem *linkTarget)
    {
        if (_frontLinkTarget)
            NeuroItem::removeOutgoing(_frontLinkTarget);
        
        if (linkTarget)
            addOutgoing(linkTarget);
        
        _frontLinkTarget = linkTarget;
        
        buildShape();
    }
    
    void NeuroLinkItem::setBackLinkTarget(NeuroItem *linkTarget)
    {
        if (_backLinkTarget)
            NeuroItem::removeIncoming(_backLinkTarget);
        
        if (linkTarget)
            addIncoming(linkTarget);
        
        _backLinkTarget = linkTarget;
        
        buildShape();
    }
    
    void NeuroLinkItem::buildShape()
    {
        NeuroItem::buildShape();

        QVector2D myPos(pos());
        QVector2D myFront(_line.p2());
        QVector2D myBack(_line.p1());
        
        // convert to my frame
        myFront = myFront - myPos;
        myBack = myBack - myPos;
        
        // calculate control points
        c1 = myBack + (myFront - myBack) * 0.33;
        c2 = myBack + (myFront - myBack) * 0.66;
        
        if (_backLinkTarget)
        {
            QVector2D center(_backLinkTarget->pos());
            QVector2D toBack = QVector2D(_line.p1()) - center;
            c1 = (center + toBack * 3) - myPos;
        }
        
        if (_frontLinkTarget && !dynamic_cast<NeuroLinkItem *>(_frontLinkTarget))
        {
            QVector2D center(_frontLinkTarget->pos());
            QVector2D toFront = QVector2D(_line.p2()) - center;
            c2 = (center + toFront * 3) - myPos;
        }
        
        _path->moveTo(myBack.toPointF());
        _path->cubicTo(c1.toPointF(), c2.toPointF(), myFront.toPointF());
        
//        _path->addEllipse(c1.toPointF(), 4, 4);
//        _path->addEllipse(c2.toPointF(), 8, 8);
//        _path->addEllipse(myFront.toPointF(), 12, 12);
    }
    
    void NeuroLinkItem::setBrushProperties(QBrush & brush)
    {
        NeuroItem::setBrushProperties(brush);
        brush.setStyle(Qt::NoBrush);
    }
    
    void NeuroLinkItem::updatePos()
    {
        setPos( (_line.x1() + _line.x2())/2, (_line.y1() + _line.y2())/2 );
        buildShape();
        adjustLinks();
    }
    
    void NeuroLinkItem::adjustLinks()
    {
        QVector2D myFront(_line.p2());
        QVector2D myBack(_line.p1());
        
        QVector2D center = (myBack + myFront) * 0.5;
                
        for (QListIterator<NeuroItem *> i(_incoming); i.hasNext(); i.next())
        {
            NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(i.peekNext());
            if (link)
                link->setLine(link->line().p1(), center.toPointF());
        }
    }
    
    bool NeuroLinkItem::canLinkTo(EditInfo & info, NeuroItem *item)
    {
        // cannot link the back of a link to another link
        if (dynamic_cast<NeuroLinkItem *>(item) && !info.linkFront)
            return false;
        
        // cannot link an excitory link to another link
        if (dynamic_cast<NeuroExcitoryLinkItem *>(this) && dynamic_cast<NeuroLinkItem *>(item))
            return false;
        
        return true;        
    }
    
    bool NeuroLinkItem::handlePickup(EditInfo & info)
    {
        qreal front_dist = (QVector2D(info.scenePos) - QVector2D(this->line().p2())).lengthSquared();
        qreal back_dist = (QVector2D(info.scenePos) - QVector2D(this->line().p1())).lengthSquared();
        
        info.linkFront = front_dist < back_dist;
        
        return true;
    }
    
    void NeuroLinkItem::handleMove(EditInfo & info)
    {
        // update line            
        QPointF front = _line.p2();
        QPointF back = _line.p1();            
        QPointF & endToMove = info.linkFront ? front : back;
        
        endToMove = info.scenePos;
        
        setLine(back.x(), back.y(), front.x(), front.y());
        adjustLinks();
        
        if (_frontLinkTarget)
            _frontLinkTarget->adjustLinks();
        else if (_backLinkTarget)
            _backLinkTarget->adjustLinks();        
        
        // get topmost item that is not the link itself
        NeuroItem *itemAtPos = 0;
        for (QListIterator<QGraphicsItem *> i(_network->scene()->items(info.scenePos, Qt::IntersectsItemShape, Qt::DescendingOrder)); i.hasNext(); i.next())
        {
            itemAtPos = dynamic_cast<NeuroItem *>(i.peekNext());
            if (itemAtPos && dynamic_cast<NeuroLinkItem *>(itemAtPos) != this && canLinkTo(info, itemAtPos))
                break;
            else
                itemAtPos = 0;
        }
        
        // break links
        NeuroItem *linkedItem = info.linkFront ? _frontLinkTarget : _backLinkTarget;
        if (linkedItem)
        {
            if (itemAtPos != linkedItem)
            {
                if (info.linkFront)
                    setFrontLinkTarget(0);
                else
                    setBackLinkTarget(0);
            }
        }
        
        // form links
        if (itemAtPos)
        {
            if (info.linkFront)
                setFrontLinkTarget(itemAtPos);
            else
                setBackLinkTarget(itemAtPos);
            
            itemAtPos->adjustLinks();
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

        IdType id = _frontLinkTarget ? _frontLinkTarget->id() : 0;
        data << id;
        
        id = _backLinkTarget ? _backLinkTarget->id() : 0;
        data << id;
    }
    
    void NeuroLinkItem::readPointerIds(QDataStream & data)
    {
        NeuroItem::readPointerIds(data);
        
        setFrontLinkTarget(0);
        setBackLinkTarget(0);

        IdType id;
        
        data >> id;
        if (id)
            _frontLinkTarget = reinterpret_cast<NeuroItem *>(id);
        
        data >> id;
        if (id)
            _backLinkTarget = reinterpret_cast<NeuroItem *>(id);
    }
    
    void NeuroLinkItem::idsToPointers(QGraphicsScene *sc)
    {
        NeuroItem::idsToPointers(sc);
        
        IdType frontId = reinterpret_cast<IdType>(_frontLinkTarget);
        IdType backId = reinterpret_cast<IdType>(_backLinkTarget);
        
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
    
    NEUROITEM_DEFINE_CREATOR(NeuroExcitoryLinkItem);
    
    NeuroExcitoryLinkItem::NeuroExcitoryLinkItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex)
        : NeuroLinkItem(network, cellIndex)
    {
    }
    
    NeuroExcitoryLinkItem::~NeuroExcitoryLinkItem()
    {
    }
    
    NeuroItem *NeuroExcitoryLinkItem::create_new(LabScene *scene, const QPointF & pos)
    {
        NeuroCell::NeuroIndex index = scene->network()->neuronet()->addNode(NeuroCell(NeuroCell::EXCITORY_LINK));
        NeuroLinkItem *item = new NeuroExcitoryLinkItem(scene->network(), index);
        item->setLine(pos.x(), pos.y(), pos.x()+NeuroItem::NODE_WIDTH*2, pos.y()-NeuroItem::NODE_WIDTH*2);
        return item;
    }
    
    void NeuroExcitoryLinkItem::buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem)
    {
        NeuroItem::buildProperties(manager, parentItem);
        parentItem->setPropertyName(tr("Excitory Link"));
    }
    
    void NeuroExcitoryLinkItem::buildShape()
    {
        NeuroLinkItem::buildShape();
        
        QVector2D center(pos());        
        QVector2D front(_line.p2());
        QVector2D fromFront((center + c2) - front);
        double angle = ::atan2(fromFront.y(), fromFront.x());
        
        double langle = angle + (20.0 * M_PI / 180.0);
        QVector2D left(::cos(langle), ::sin(langle));
        
        QVector2D end = front + left * ELLIPSE_WIDTH;
        
        _path->moveTo((front - center).toPointF());
        _path->lineTo((end - center).toPointF());
        
        double rangle = angle - (20.0 * M_PI / 180.0);
        QVector2D right(::cos(rangle), ::sin(rangle));
        
        end = front + right * ELLIPSE_WIDTH;
        
        _path->moveTo((front - center).toPointF());
        _path->lineTo((end - center).toPointF());
    }
    
    
    //////////////////////////////////////////////////////////////////
    
    NEUROITEM_DEFINE_CREATOR(NeuroInhibitoryLinkItem);

    NeuroInhibitoryLinkItem::NeuroInhibitoryLinkItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex)
        : NeuroLinkItem(network, cellIndex)
    {
    }
    
    NeuroInhibitoryLinkItem::~NeuroInhibitoryLinkItem()
    {
    }
    
    NeuroItem *NeuroInhibitoryLinkItem::create_new(LabScene *scene, const QPointF & pos)
    {
        NeuroCell::NeuroIndex index = scene->network()->neuronet()->addNode(NeuroCell(NeuroCell::INHIBITORY_LINK, -1));
        NeuroLinkItem *item = new NeuroInhibitoryLinkItem(scene->network(), index);
        item->setLine(pos.x(), pos.y(), pos.x()+NeuroItem::NODE_WIDTH*2, pos.y()-NeuroItem::NODE_WIDTH*2);
        return item;
    }
    
    void NeuroInhibitoryLinkItem::buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem)
    {
        NeuroItem::buildProperties(manager, parentItem);
        parentItem->setPropertyName(tr("Inhibitory Link"));
    }
    
    void NeuroInhibitoryLinkItem::buildShape()
    {
        NeuroLinkItem::buildShape();
        
        QVector2D center(pos());
        QVector2D front(_line.p2());
        
        _path->addEllipse((front-center).toPointF(), ELLIPSE_WIDTH/2, ELLIPSE_WIDTH/2);
    }
    
} // namespace NeuroLab
