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
    QMap<QString, NeuroItem::CreateFT> NeuroItem::itemCreators;
    
    NeuroItem::NeuroItem(LabNetwork *network, const NeuroCell::NeuroIndex & cellIndex)
        : QGraphicsItem(), _network(network), _id(NEXT_ID++), _path(0), _textPath(0), _cellIndex(cellIndex)
    {
        // we don't use qt selection because it seems to be broken
        //this->setFlag(QGraphicsItem::ItemIsSelectable, true);
        this->setFlag(QGraphicsItem::ItemIsMovable, true);
        this->setAcceptHoverEvents(true);
    }
    
    NeuroItem::~NeuroItem()
    {
        delete _path;
        delete _textPath;
        
        while (_incoming.size() > 0)
            removeIncoming(_incoming.front());
        while (_outgoing.size() > 0)
            removeOutgoing(_outgoing.front());
    }
    
    NeuroItem *NeuroItem::create(const QString & name, LabScene *scene, const QPointF & pos)
    {
        CreateFT cf = itemCreators[name];
        
        if (cf)
            return cf(scene, pos);        
        return 0;
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
    
    void NeuroItem::addIncoming(NeuroItem *linkItem)
    {
        if (linkItem && !_incoming.contains(linkItem))
        {
            _incoming.append(linkItem);
            
            if (_cellIndex != -1 && linkItem->_cellIndex != -1)
                _network->neuronet()->addEdge(_cellIndex, linkItem->_cellIndex);            
            
            linkItem->addOutgoing(this);
        }
    }
    
    void NeuroItem::removeIncoming(NeuroItem *linkItem)
    {
        if (linkItem && _incoming.contains(linkItem))
        {
            _incoming.removeAll(linkItem);
            
            if (_cellIndex != -1 && linkItem->_cellIndex != -1)
                _network->neuronet()->removeEdge(_cellIndex, linkItem->_cellIndex);

            linkItem->removeOutgoing(this);
        }
    }
    
    void NeuroItem::addOutgoing(NeuroItem *linkItem)
    {
        if (linkItem && !_outgoing.contains(linkItem))
        {
            _outgoing.append(linkItem);
            
            linkItem->addIncoming(this);
        }
    }
    
    void NeuroItem::removeOutgoing(NeuroItem *linkItem)
    {
        if (linkItem && _outgoing.contains(linkItem))
        {
            _outgoing.removeAll(linkItem);
            
            linkItem->removeIncoming(this);
        }
    }
    
    void NeuroItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
    {
        if (_network && _network->scene())
        {
            _network->scene()->setItemUnderMouse(this);
            
            if (_network->scene()->selectedItem() != this)
                _network->scene()->setSelectedItem(0);
        }
        
        update(this->boundingRect());
    }
    
    void NeuroItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
    {
        if (_network && _network->scene())
            _network->scene()->setItemUnderMouse(0);
        
        update(this->boundingRect());
    }
    
    void NeuroItem::buildShape()
    {
        delete _path;
        _path = new QPainterPath();
        _path->setFillRule(Qt::WindingFill);
        
        delete _textPath;
        _textPath = new QPainterPath();
        _textPath->setFillRule(Qt::WindingFill);

        if (!_label.isNull() && !_label.isEmpty())
            _textPath->addText(NeuroItem::NODE_WIDTH + 4, -1, QApplication::font(), _label);

        const_cast<NeuroItem *>(this)->prepareGeometryChange();
    }

    QRectF NeuroItem::boundingRect() const
    {
        if (!_path || !_textPath)
            const_cast<NeuroItem *>(this)->buildShape();
        
        return _path->united(*_textPath).controlPointRect();
    }

    QPainterPath NeuroItem::shape() const
    {
        if (!_path || !_textPath)
            const_cast<NeuroItem *>(this)->buildShape();

        return _path->united(*_textPath);
    }

    bool NeuroItem::shouldHighlight() const
    {
        if (isSelected())
            return true;

        const LabScene *sc = dynamic_cast<const LabScene *>(this->scene());
        if (sc && (sc->selectedItem() == this || sc->itemUnderMouse() == this))
            return true;

        return false;
    }

    void NeuroItem::setPenWidth(QPen & pen)
    {
        if (shouldHighlight())
            pen.setWidth(HOVER_LINE_WIDTH);
        else
            pen.setWidth(NORMAL_LINE_WIDTH);
    }

    static QColor lerp(const QColor & a, const QColor & b, const qreal & t)
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

    void NeuroItem::setPenColor(QPen & pen)
    {
        NeuroCell *cell = getCell();
        if (cell)
        {
            qreal t = qBound(static_cast<qreal>(0), qAbs(static_cast<qreal>(cell->value())), static_cast<qreal>(1));

            QColor result = lerp(NORMAL_LINE_COLOR, ACTIVE_COLOR, t);

            if (cell->frozen())
                pen.setColor(lerp(result, Qt::gray, 0.5f));
            else
                pen.setColor(result);
        }
        else
        {
            pen.setColor(NORMAL_LINE_COLOR);
        }
    }

    void NeuroItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
    {
        if (!_path || !_textPath)
            buildShape();
        
        painter->setRenderHint(QPainter::Antialiasing);

        QPen pen(Qt::SolidLine);
        setPenWidth(pen);
        setPenColor(pen);

        QBrush brush(Qt::SolidPattern);
        brush.setColor(BACKGROUND_COLOR);

        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawPath(*_path);
        
        QPen textPen(Qt::SolidLine);
        pen.setColor(NORMAL_LINE_COLOR);
        painter->setPen(textPen);
        painter->drawPath(*_textPath);
    }


    // I/O

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
        data << item._label;
        data << static_cast<qint64>(item._cellIndex);

        item.writeBinary(data);
        item.writePointerIds(data);

        return data;
    }

    QDataStream & operator>> (QDataStream & data, NeuroItem & item)
    {
        qint64 n;

        data >> item._label;
        data >> n; item._cellIndex = static_cast<NeuroCell::NeuroIndex>(n);

        item.readBinary(data);
        item.readPointerIds(data);

        return data;
    }

    // neuro stuff

    NeuroCell *NeuroItem::getCell()
    {
        return _cellIndex != -1 ? &((*_network->neuronet())[_cellIndex]) : 0;
    }

    void NeuroItem::reset()
    {
        NeuroCell *cell = getCell();
        if (cell && !cell->frozen())
            deactivate();
    }

    void NeuroItem::activate()
    {
        NeuroCell *cell = getCell();
        if (cell)
        {
            cell->setValue(1);
            update(boundingRect());
        }
    }

    void NeuroItem::deactivate()
    {
        NeuroCell *cell = getCell();
        if (cell)
        {
            cell->setValue(0);
            update(boundingRect());
        }
    }

    void NeuroItem::toggleFrozen()
    {
        NeuroCell *cell = getCell();
        if (cell)
        {
            cell->setFrozen(!cell->frozen());
            update(boundingRect());
        }
    }

} // namespace NeuroLab
