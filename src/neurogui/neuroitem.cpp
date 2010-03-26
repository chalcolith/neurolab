#include "neuroitem.h"
#include "mainwindow.h"
#include "neurolinkitem.h"
#include "labscene.h"
#include "labnetwork.h"

#include "../automata/exception.h"
#include "../neurolib/neuronet.h"
#include "../neurolib/neurocell.h"

#include <QApplication>
#include <QStatusBar>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

#include <QtVariantProperty>

using namespace NeuroLib;

namespace NeuroLab
{

    const QColor NeuroItem::NORMAL_LINE_COLOR = Qt::black;
    const QColor NeuroItem::UNLINKED_LINE_COLOR = Qt::lightGray;
    const QColor NeuroItem::BACKGROUND_COLOR = Qt::white;
    const QColor NeuroItem::ACTIVE_COLOR = Qt::red;

    const int NeuroItem::NORMAL_LINE_WIDTH = 1;
    const int NeuroItem::HOVER_LINE_WIDTH = 3;

    const int NeuroItem::NODE_WIDTH = 30;
    const int NeuroItem::ELLIPSE_WIDTH = 10;

    //////////////////////////////////////////////

    int NeuroItem::NEXT_ID = 1;
    QMap<QString, QPair<QString, NeuroItem::CreateFT> > NeuroItem::itemCreators;

    NeuroItem::NeuroItem(LabNetwork *network)
        : QObject(network), QGraphicsItem(),
        PropertyObject(), label_property(0),
        _network(network), _id(NEXT_ID++)
    {
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

        setAcceptHoverEvents(true);
    }

    NeuroItem::~NeuroItem()
    {
        while (_incoming.size() > 0)
            removeIncoming(_incoming.front());
        while (_outgoing.size() > 0)
            removeOutgoing(_outgoing.front());
    }

    NeuroItem *NeuroItem::create(const QString & name, LabScene *scene, const QPointF & pos)
    {
        CreateFT cf = itemCreators[name].second;

        if (cf)
            return cf(scene, pos);
        return 0;
    }

    void NeuroItem::buildNewMenu(LabScene *, NeuroItem *item, const QPointF & pos, QMenu & menu)
    {
        QAction *newAction = menu.addAction(tr("New"));
        newAction->setEnabled(false);

        for (QMapIterator<QString, QPair<QString, NeuroItem::CreateFT> > i(itemCreators); i.hasNext(); i.next())
        {
            const QString & typeName = i.peekNext().key();
            const QPair<QString, NeuroItem::CreateFT> & p = i.peekNext().value();
            const QStringList menuPath = p.first.split('|');

            QMenu *subMenu = &menu;
            for (int j = 0; j < menuPath.size(); ++j)
            {
                if (j == menuPath.size()-1)
                {
                    // create the leaf action
                    QAction *action = subMenu->addAction(menuPath[j]);
                    action->setData(QVariant(typeName));
                    action->setEnabled(!item || item->canCreateNewOnMe(typeName, pos));
                }
                else
                {
                    // find the submenu
                    bool found = false;
                    for (QListIterator<QAction *> k(subMenu->actions()); k.hasNext(); k.next())
                    {
                        QAction *action = k.peekNext();
                        if (action->text() == menuPath[j] && action->menu())
                        {
                            subMenu = action->menu();
                            found = true;
                            break;
                        }
                    }

                    // create the submenu
                    if (!found)
                        subMenu = subMenu->addMenu(menuPath[j]);
                }
            }
        }
    }

    void NeuroItem::buildActionMenu(LabScene *scene, NeuroItem *item, const QPointF & pos, QMenu & menu)
    {
        if (item)
            item->buildActionMenuAux(scene, pos, menu);

        QAction *del = menu.addAction(QObject::tr("Delete"), scene->network(), SLOT(deleteSelected()), QKeySequence(QKeySequence::Delete));
        del->setEnabled(scene->selectedItems().size() > 0);
    }

    void NeuroItem::buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem)
    {
        if (!label_property)
        {
            manager->connect(manager, SIGNAL(valueChanged(QtProperty*,QVariant)), this, SLOT(propertyValueChanged(QtProperty*,QVariant)));
            label_property = manager->addProperty(QVariant::String, tr("Label"));
            _properties.append(label_property);

            updateProperties();
        }

        parentItem->addSubProperty(label_property);
    }

    void NeuroItem::updateProperties()
    {
        _updating = true;

        if (label_property)
            label_property->setValue(QVariant(_label));

        _updating = false;
    }

    void NeuroItem::propertyValueChanged(QtProperty *property, const QVariant & value)
    {
        if (_updating)
            return;

        QtVariantProperty *vprop = dynamic_cast<QtVariantProperty *>(property);

        if (vprop)
        {
            if (vprop == label_property)
            {
                _label = value.toString();

                updateShape();
                update();
            }
        }
    }

    void NeuroItem::bringToFront()
    {
        qreal highest_z = this->zValue();
        for (QListIterator<QGraphicsItem *> i(this->collidingItems()); i.hasNext(); i.next())
        {
            QGraphicsItem *item = i.peekNext();
            if (!item)
                continue;

            if (item->zValue() > highest_z)
                highest_z = item->zValue();
        }

        if (highest_z > this->zValue())
            this->setZValue(highest_z + 0.1);
    }

    bool NeuroItem::addIncoming(NeuroItem *linkItem)
    {
        if (linkItem && !_incoming.contains(linkItem))
        {
            _incoming.append(linkItem);
            linkItem->addOutgoing(this);
            return true;
        }
        return false;
    }

    bool NeuroItem::removeIncoming(NeuroItem *linkItem)
    {
        if (linkItem && _incoming.contains(linkItem))
        {
            _incoming.removeAll(linkItem);
            linkItem->removeOutgoing(this);
            return true;
        }
        return false;
    }

    bool NeuroItem::addOutgoing(NeuroItem *linkItem)
    {
        if (linkItem && !_outgoing.contains(linkItem))
        {
            _outgoing.append(linkItem);
            linkItem->addIncoming(this);
            return true;
        }
        return false;
    }

    bool NeuroItem::removeOutgoing(NeuroItem *linkItem)
    {
        if (linkItem && _outgoing.contains(linkItem))
        {
            _outgoing.removeAll(linkItem);
            linkItem->removeIncoming(this);
            return true;
        }
        return false;
    }

    void NeuroItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
    {
        if (_network && _network->scene())
            _network->scene()->setItemUnderMouse(this);
        update();
    }

    void NeuroItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
    {
        if (_network && _network->scene())
            _network->scene()->setItemUnderMouse(0);
        update(this->boundingRect());
    }

    void NeuroItem::updateShape() const
    {
        const_cast<NeuroItem *>(this)->prepareGeometryChange();

        // re-initialize path
        _drawPath = QPainterPath();
        _drawPath.setFillRule(Qt::WindingFill);

        // call virtual function
        _texts.clear();
        addToShape();

        // stroke shape path
        QPen pen;
        setPenProperties(pen);

        QPainterPathStroker stroker;
        stroker.setCapStyle(Qt::SquareCap);
        stroker.setDashPattern(Qt::SolidLine);
        stroker.setJoinStyle(Qt::MiterJoin);
        stroker.setWidth(pen.width() * 4);

        _shapePath = stroker.createStroke(_drawPath);
        _shapePath.setFillRule(Qt::WindingFill);

        // add texts
        if (!_label.isNull() && !_label.isEmpty())
            _texts.append(TextPathRec(QPointF(NeuroItem::NODE_WIDTH + 4, -1), _label));

        for (QListIterator<TextPathRec> i(_texts); i.hasNext(); i.next())
        {
            const TextPathRec & rec = i.peekNext();
            _shapePath.addText(rec.pos, QApplication::font(), rec.text);
        }

        _shapePath = _drawPath.united(_shapePath);
    }

    void NeuroItem::addToShape() const
    {
    }

    QRectF NeuroItem::boundingRect() const
    {
        return _shapePath.controlPointRect();
    }

    QPainterPath NeuroItem::shape() const
    {
        return _shapePath;
    }

    bool NeuroItem::shouldHighlight() const
    {
        if (isSelected())
            return true;

        const LabScene *sc = dynamic_cast<const LabScene *>(this->scene());
        if (sc && (sc->itemUnderMouse() == this || sc->selectedItems().contains(const_cast<NeuroItem *>(this))))
            return true;

        return false;
    }

    QColor NeuroItem::lerp(const QColor & a, const QColor & b, const qreal & t)
    {
        qreal ar, ag, ab, aa;
        qreal br, bg, bb, ba;

        a.getRgbF(&ar, &ag, &ab, &aa);
        b.getRgbF(&br, &bg, &bb, &ba);

        qreal rr = ar + (br - ar)*t;
        qreal rg = ag + (bg - ag)*t;
        qreal rb = ab + (bb - ab)*t;
        qreal ra = aa + (ba - aa)*t;

        QColor result;
        result.setRgbF(rr, rg, rb, ra);
        return result;
    }

    void NeuroItem::setPenProperties(QPen & pen) const
    {
        if (shouldHighlight())
            pen.setWidth(HOVER_LINE_WIDTH);
        else
            pen.setWidth(NORMAL_LINE_WIDTH);

        pen.setColor(NORMAL_LINE_COLOR);
    }

    void NeuroItem::setBrushProperties(QBrush & brush) const
    {
        brush.setColor(BACKGROUND_COLOR);
    }

    void NeuroItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
    {
        if (MainWindow::instance()->propertyObject() == this)
            updateProperties();

        painter->setRenderHint(QPainter::Antialiasing);

        QPen pen(Qt::SolidLine);
        setPenProperties(pen);

        QBrush brush(Qt::SolidPattern);
        setBrushProperties(brush);

        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawPath(_drawPath);

        QPen textPen(Qt::SolidLine);
        textPen.setColor(NORMAL_LINE_COLOR);
        textPen.setWidth(NORMAL_LINE_WIDTH);
        painter->setPen(textPen);

        for (QListIterator<TextPathRec> i(_texts); i.hasNext(); i.next())
        {
            const TextPathRec & rec = i.peekNext();
            painter->drawText(rec.pos, rec.text);
        }
    }

    bool NeuroItem::canAttachTo(const QPointF &, NeuroItem *item)
    {
        return !_incoming.contains(item) && !_outgoing.contains(item);
    }

    bool NeuroItem::canBeAttachedBy(const QPointF &, NeuroItem *item)
    {
        return !_incoming.contains(item) && !_outgoing.contains(item);
    }

    QVariant NeuroItem::itemChange(GraphicsItemChange change, const QVariant & value)
    {
        LabScene *labScene;

        switch (change)
        {
        case QGraphicsItem::ItemPositionChange:
            if ((labScene = dynamic_cast<LabScene *>(scene())) && labScene->selectedItems().size() <= 1)
            {
                QPointF movePos(value.toPointF());

                if (handleMove(labScene->lastMousePos(), movePos))
                {
                    updateShape();
                    return QVariant(movePos);
                }
            }
            break;
        default:
            break;
        }

        return value;
    }

    bool NeuroItem::handleMove(const QPointF & mousePos, QPointF & movePos)
    {
        // get topmost item at mouse position that is not the item itself
        QRectF mouseRect(mousePos.x() - 2, mousePos.y() - 2, 4, 4);
        NeuroItem *itemAtPos = 0;

        for (QListIterator<QGraphicsItem *> i(_network->scene()->items(mouseRect, Qt::IntersectsItemShape, Qt::DescendingOrder)); i.hasNext(); i.next())
        {
            if ((itemAtPos = dynamic_cast<NeuroItem *>(i.peekNext())))
            {
                if (itemAtPos == this)
                {
                    itemAtPos = 0;
                    continue;
                }
                else
                {
                    break;
                }
            }
        }

        // attach, if possible
        if (itemAtPos && canAttachTo(mousePos, itemAtPos) && itemAtPos->canBeAttachedBy(mousePos, this))
        {
            attachTo(itemAtPos);
            itemAtPos->onAttachedBy(this);

            itemAtPos->adjustLinks(); // this may change the position
            movePos = scenePos();
        }

        adjustLinks();
        return true;
    }


    // I/O

    void NeuroItem::writeBinary(QDataStream & data) const
    {
        data << _label;
        data << _id;
    }

    void NeuroItem::readBinary(QDataStream & data)
    {
        data >> _label;
        data >> _id;
    }

    /// Writes ids of incoming and outgoing pointers.
    void NeuroItem::writePointerIds(QDataStream & ds) const
    {
        qint32 n = _incoming.size();
        ds << n;

        for (qint32 i = 0; i < n; ++i)
        {
            if (_incoming[i])
                ds << static_cast<IdType>(_incoming[i]->_id);
            else
                ds << static_cast<IdType>(0);
        }

        n = _outgoing.size();
        ds << n;

        for (qint32 i = 0; i < n; ++i)
        {
            if (_outgoing[i])
                ds << static_cast<IdType>(_outgoing[i]->_id);
            else
                ds << static_cast<IdType>(0);
        }
    }

    void NeuroItem::readPointerIds(QDataStream & ds)
    {
        qint32 n;

        _incoming.clear();
        ds >> n;
        for (qint32 i = 0; i < n; ++i)
        {
            IdType id;
            ds >> id;

            if (id)
            {
                _incoming.append(reinterpret_cast<NeuroLinkItem *>(id));
            }
        }

        _outgoing.clear();
        ds >> n;
        for (qint32 i = 0; i < n; ++i)
        {
            IdType id;
            ds >> id;

            if (id)
            {
                _outgoing.append(reinterpret_cast<NeuroLinkItem *>(id));
            }
        }
    }

    void NeuroItem::idsToPointers(QGraphicsScene *sc)
    {
        idsToPointersAux(_incoming, sc);
        idsToPointersAux(_outgoing, sc);
    }

    void NeuroItem::idsToPointersAux(QList<NeuroItem *> & list, QGraphicsScene *sc)
    {
        QList<QGraphicsItem *> items = sc->items();

        for (QMutableListIterator<NeuroItem *> in(list); in.hasNext(); in.next())
        {
            bool found = false;
            IdType id = reinterpret_cast<IdType>(in.peekNext());
            for (QListIterator<QGraphicsItem *> i(items); i.hasNext(); i.next())
            {
                NeuroItem *item = dynamic_cast<NeuroItem *>(i.peekNext());
                if (item && item->_id == id)
                {
                    in.peekNext() = item;
                    found = true;
                    break;
                }
            }

            if (!found)
                throw Automata::Exception(QObject::tr("Dangling node ID %1").arg(id));
        }
    }

    QDataStream & operator<< (QDataStream & data, const NeuroItem & item)
    {
        item.writeBinary(data);
        item.writePointerIds(data);
        return data;
    }

    QDataStream & operator>> (QDataStream & data, NeuroItem & item)
    {
        item.readBinary(data);
        item.readPointerIds(data);
        return data;
    }

} // namespace NeuroLab
