#include "mixinarrow.h"
#include "neuroitem.h"
#include "neurolinkitem.h"
#include "labscene.h"

namespace NeuroGui
{

    MixinArrow::MixinArrow(NeuroItem *self)
        : _self(self),
        _frontLinkTarget(0), _backLinkTarget(0), _dragFront(false), _settingLine(false)
    {
        Q_ASSERT(self);
    }

    MixinArrow::~MixinArrow()
    {
        _frontLinkTarget = 0;
        _backLinkTarget = 0;
    }

    QLineF MixinArrow::line() const
    {
        QVector2D center(_self->scenePos());
        QVector2D p1 = QVector2D(_line.p1()) + center;
        QVector2D p2 = QVector2D(_line.p2()) + center;

        return QLineF(p1.toPointF(), p2.toPointF());
    }

    void MixinArrow::setLine(const QLineF & l, const QPointF *const c)
    {
        _settingLine = true;
        _self->prepGeomChange();

        QVector2D p1(l.p1());
        QVector2D p2(l.p2());
        QVector2D center = c ? QVector2D(*c) : ((p1 + p2) * 0.5f);

        _line = QLineF((p1 - center).toPointF(), (p2 - center).toPointF());
        _self->setPos(center.toPointF());

        _self->updateShape();
        _self->adjustLinks();

        _settingLine = false;
    }

    void MixinArrow::setLine(const qreal & x1, const qreal & y1, const qreal & x2, const qreal & y2)
    {
        setLine(QLineF(x1, y1, x2, y2));
    }

    void MixinArrow::setLine(const QPointF & p1, const QPointF & p2)
    {
        setLine(QLineF(p1, p2));
    }

    void MixinArrow::setFrontLinkTarget(NeuroItem *linkTarget)
    {
        NeuroItem *frontTarget = _frontLinkTarget;
        _frontLinkTarget = 0;

        if (frontTarget)
            _self->removeOutgoing(frontTarget);

        if (linkTarget)
            _self->addOutgoing(linkTarget);

        _frontLinkTarget = linkTarget;

        _self->updateShape();
    }

    void MixinArrow::setBackLinkTarget(NeuroItem *linkTarget)
    {
        NeuroItem *backTarget = _backLinkTarget;
        _backLinkTarget = 0;

        if (backTarget)
            _self->removeIncoming(backTarget);

        if (linkTarget)
            _self->addIncoming(linkTarget);

        _backLinkTarget = linkTarget;

        _self->updateShape();
    }

    void MixinArrow::addArrow(QPainterPath & drawPath) const
    {
        QVector2D myPos(_self->scenePos());

        // in my frame
        QVector2D myFront(_line.p2());
        QVector2D myBack(_line.p1());

        // calculate control points
        if (_frontLinkTarget && _frontLinkTarget == _backLinkTarget)
        {
            QVector2D toFront = myFront - myBack;
            QVector2D toBack = myBack - myFront;

            c1 = toBack;
            c2 = toFront;
        }
        else
        {
            c1 = myBack + (myFront - myBack) * 0.33;
            c2 = myBack + (myFront - myBack) * 0.66;

            if (_backLinkTarget)
            {
                QVector2D center(_backLinkTarget->pos());
                QVector2D toBack = QVector2D(line().p1()) - center;
                c1 = (center + toBack * 3) - myPos;
            }

            if (_frontLinkTarget && !dynamic_cast<NeuroLinkItem *>(_frontLinkTarget))
            {
                QVector2D center(_frontLinkTarget->scenePos());
                QVector2D toFront = QVector2D(line().p2()) - center;
                c2 = (center + toFront * 3) - myPos;
            }
        }

        QPointF front = myFront.toPointF();
        QPointF back = myBack.toPointF();

        drawPath.moveTo(back);
        drawPath.cubicTo(c1.toPointF(), c2.toPointF(), front);
    }

    QVariant MixinArrow::changePos(LabScene *labScene, const QVariant & value, bool canDragFront, bool canDragBack)
    {
        if (!_settingLine && labScene
            && labScene->selectedItems().size() <= 1
            && labScene->mouseIsDown())
        {
            _self->prepGeomChange();

            // adjust position
            QVector2D oldCenter(_self->scenePos());
            QVector2D front = QVector2D(_line.p2()) + oldCenter;
            QVector2D back = QVector2D(_line.p1()) + oldCenter;
            QVector2D mousePos(labScene->lastMousePos());

            qreal distFront = (mousePos - front).lengthSquared();
            qreal distBack = (mousePos - back).lengthSquared();

            _dragFront = distFront < distBack;
            if (_dragFront)
            {
                if (canDragFront)
                    front = mousePos;
                else
                    return QVariant(oldCenter);
            }
            else if (!_dragFront)
            {
                if (canDragBack)
                    back = mousePos;
                else
                    return QVariant(oldCenter);
            }

            QVector2D newCenter = (front + back) * 0.5f;

            // set new position and line
            _settingLine = true;
            _line = QLineF((back - newCenter).toPointF(), (front - newCenter).toPointF());
            _self->setPos(newCenter.toPointF());
            _settingLine = false;

            // adjust if we're linked
            if (_frontLinkTarget && _dragFront)
                _frontLinkTarget->adjustLinks();
            if (_backLinkTarget && !_dragFront)
                _backLinkTarget->adjustLinks();

            // handle move
            QPointF movePos(_self->scenePos());
            _self->handleMove(mousePos.toPointF(), movePos);

            _self->adjustLinks();
            _self->updateShape();

            return QVariant(_self->scenePos());
        }
        else
        {
            return value;
        }
    }

} // namespace NeuroGui
