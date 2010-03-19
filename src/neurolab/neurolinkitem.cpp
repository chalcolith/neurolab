#include "neurolinkitem.h"
#include "labnetwork.h"
#include "mainwindow.h"
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
        : NeuroNarrowItem(network, cellIndex), _frontLinkTarget(0), _backLinkTarget(0), dragFront(false), settingLine(false)
    {
    }

    NeuroLinkItem::~NeuroLinkItem()
    {
        _frontLinkTarget = 0;
        _backLinkTarget = 0;
    }

    void NeuroLinkItem::setLine(const QLineF & l)
    {
        settingLine = true;
        prepareGeometryChange();
        _line = l;
        updatePos();
        settingLine = false;
    }

    void NeuroLinkItem::setLine(const qreal & x1, const qreal & y1, const qreal & x2, const qreal & y2)
    {
        setLine(QLineF(x1, y1, x2, y2));
    }

    void NeuroLinkItem::setLine(const QPointF & p1, const QPointF & p2)
    {
        setLine(QLineF(p1, p2));
    }

    bool NeuroLinkItem::addIncoming(NeuroItem *linkItem)
    {
        return NeuroNarrowItem::addIncoming(linkItem);
    }

    bool NeuroLinkItem::removeIncoming(NeuroItem *linkItem)
    {
        if (linkItem && linkItem == _backLinkTarget)
            setBackLinkTarget(0);
        return NeuroNarrowItem::removeIncoming(linkItem);
    }

    bool NeuroLinkItem::addOutgoing(NeuroItem *linkItem)
    {
        return NeuroNarrowItem::addOutgoing(linkItem);
    }

    bool NeuroLinkItem::removeOutgoing(NeuroItem *linkItem)
    {
        if (linkItem && linkItem == _frontLinkTarget)
            setFrontLinkTarget(0);
        return NeuroNarrowItem::removeOutgoing(linkItem);
    }

    void NeuroLinkItem::setFrontLinkTarget(NeuroItem *linkTarget)
    {
        if (_frontLinkTarget)
            NeuroNarrowItem::removeOutgoing(_frontLinkTarget);

        if (linkTarget)
            addOutgoing(linkTarget);

        _frontLinkTarget = linkTarget;

        buildShape();
    }

    void NeuroLinkItem::setBackLinkTarget(NeuroItem *linkTarget)
    {
        if (_backLinkTarget)
            NeuroNarrowItem::removeIncoming(_backLinkTarget);

        if (linkTarget)
            addIncoming(linkTarget);

        _backLinkTarget = linkTarget;

        buildShape();
    }

    void NeuroLinkItem::buildShape()
    {
        NeuroNarrowItem::buildShape();

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

        QPointF front = myFront.toPointF();
        QPointF back = myBack.toPointF();

        _path->moveTo(back);
        _path->cubicTo(c1.toPointF(), c2.toPointF(), front);
    }

    void NeuroLinkItem::setPenProperties(QPen & pen)
    {
        NeuroNarrowItem::setPenProperties(pen);

        NeuroCell *cell = getCell();
        if (cell)
        {
            qreal weight = qBound(0.1f, qAbs(cell->weight()), 1.0f);
            pen.setColor(lerp(Qt::lightGray, pen.color(), weight));
        }
    }

    void NeuroLinkItem::setBrushProperties(QBrush & brush)
    {
        NeuroNarrowItem::setBrushProperties(brush);
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
        QVector2D front(_line.p2());
        QVector2D back(_line.p1());

        QVector2D center = (back + front) * 0.5;

        for (QListIterator<NeuroItem *> i(_incoming); i.hasNext(); i.next())
        {
            NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(i.peekNext());
            if (link)
                link->setLine(link->line().p1(), center.toPointF());
        }
    }

    bool NeuroLinkItem::canAttachTo(const QPointF & pos, NeuroItem *item)
    {
        // cannot link the back of a link to another link
        if (dynamic_cast<NeuroLinkItem *>(item) && !dragFront)
            return false;
        return NeuroNarrowItem::canAttachTo(pos, item);
    }

    bool NeuroLinkItem::canBeAttachedBy(const QPointF &, NeuroItem *item)
    {
        // can only be attached to by an inhibitory link
        return dynamic_cast<NeuroInhibitoryLinkItem *>(item);
    }

    void NeuroLinkItem::attachTo(NeuroItem *item)
    {
        if (dragFront)
            setFrontLinkTarget(item);
        else
            setBackLinkTarget(item);
    }

    bool NeuroLinkItem::handleMove(const QPointF & mousePos, QPointF & movePos)
    {
        // break links
        NeuroItem *linkedItem = dragFront ? _frontLinkTarget : _backLinkTarget;
        if (linkedItem && !linkedItem->contains(linkedItem->mapFromScene(mousePos)))
        {
            if (dragFront)
                setFrontLinkTarget(0);
            else
                setBackLinkTarget(0);
        }

        // attach
        return NeuroNarrowItem::handleMove(mousePos, movePos);
    }

    QVariant NeuroLinkItem::itemChange(GraphicsItemChange change, const QVariant & value)
    {
        LabScene *labScene;

        switch (change)
        {
        case QGraphicsItem::ItemPositionChange:
            if ((labScene = dynamic_cast<LabScene *>(scene())) && labScene->selectedItems().size() <= 1)
            {
                if (!settingLine)
                {
                    // adjust position
                    QVector2D center(pos());
                    QVector2D front(_line.p2());
                    QVector2D back(_line.p1());
                    QVector2D mousePos(labScene->lastMousePos());

                    qreal distFront = (mousePos - front).lengthSquared();
                    qreal distBack = (mousePos - back).lengthSquared();

                    dragFront = distFront < distBack;
                    if (dragFront)
                        front = mousePos;
                    else
                        back = mousePos;

                    _line.setP1(back.toPointF());
                    _line.setP2(front.toPointF());

                    QVector2D newCenter = (front + back) * 0.5;

                    // adjust if we're linked
                    if (_frontLinkTarget)
                        _frontLinkTarget->adjustLinks();
                    if (_backLinkTarget)
                        _backLinkTarget->adjustLinks();

                    QVector2D adjustedCenter(pos());
                    if (adjustedCenter != center)
                        newCenter = adjustedCenter;

                    // handle move
                    QPointF movePos(newCenter.toPointF());

                    if (handleMove(mousePos.toPointF(), movePos))
                    {
                        adjustLinks();
                        buildShape();

                        return QVariant(movePos);
                    }
                }

                return value;
            }
            break;

        default:
            break;
        }

        return NeuroNarrowItem::itemChange(change, value);
    }

    void NeuroLinkItem::writeBinary(QDataStream & data) const
    {
        NeuroNarrowItem::writeBinary(data);
        data << _line;
    }

    void NeuroLinkItem::readBinary(QDataStream & data)
    {
        NeuroNarrowItem::readBinary(data);

        QLineF l;
        data >> l;
        setLine(l);
    }

    void NeuroLinkItem::writePointerIds(QDataStream & data) const
    {
        NeuroNarrowItem::writePointerIds(data);

        IdType id = _frontLinkTarget ? _frontLinkTarget->id() : 0;
        data << id;

        id = _backLinkTarget ? _backLinkTarget->id() : 0;
        data << id;
    }

    void NeuroLinkItem::readPointerIds(QDataStream & data)
    {
        NeuroNarrowItem::readPointerIds(data);

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
        NeuroNarrowItem::idsToPointers(sc);

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
        item->setLine(pos.x(), pos.y(), pos.x()+NeuroNarrowItem::NODE_WIDTH*2, pos.y()-NeuroNarrowItem::NODE_WIDTH*2);
        return item;
    }

    void NeuroExcitoryLinkItem::buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem)
    {
        NeuroNarrowItem::buildProperties(manager, parentItem);
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

    bool NeuroExcitoryLinkItem::canAttachTo(const QPointF & pos, NeuroItem *item)
    {
        // can not attach to a link at all
        return NeuroLinkItem::canAttachTo(pos, item) && !dynamic_cast<NeuroLinkItem *>(item);
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
        item->setLine(pos.x(), pos.y(), pos.x()+NeuroNarrowItem::NODE_WIDTH*2, pos.y()-NeuroNarrowItem::NODE_WIDTH*2);
        return item;
    }

    void NeuroInhibitoryLinkItem::buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem)
    {
        NeuroNarrowItem::buildProperties(manager, parentItem);
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
