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
    
    NeuroItem::NeuroItem(LabNetwork *network, NeuroCell::NeuroIndex cellIndex)
        : QGraphicsItem(), _network(network), _id(NEXT_ID++), _in_hover(false), _labelRect(20, -10, 0, 20), _cellIndex(cellIndex)
    {
        // we don't use qt selection because it seems to be broken
        //this->setFlag(QGraphicsItem::ItemIsSelectable, true);
        this->setFlag(QGraphicsItem::ItemIsMovable, true);
        this->setAcceptHoverEvents(true);
    }
    
    NeuroItem::~NeuroItem()
    {
        NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(this);
        if (link)
        {
            for (QListIterator<NeuroLinkItem *> in(_incoming); in.hasNext(); in.next())
            {
                in.peekNext()->removeOutgoing(link);
            }
            
            for (QListIterator<NeuroLinkItem *> out(_outgoing); out.hasNext(); out.next())
            {
                out.peekNext()->removeIncoming(link);
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
    
    void NeuroItem::addIncoming(NeuroLinkItem *linkItem)
    {
        if (_cellIndex != -1 && linkItem && linkItem->_cellIndex != -1)
        {
            NeuroLib::NeuroNet *neuronet = _network->neuronet();
            neuronet->addEdge(_cellIndex, linkItem->_cellIndex);
        }
        
        if (!_incoming.contains(linkItem))
            _incoming.append(linkItem);
    }
    
    void NeuroItem::removeIncoming(NeuroLinkItem *linkItem)
    {
        if (_cellIndex != -1 && linkItem && linkItem->_cellIndex != -1)
        {
            NeuroLib::NeuroNet *neuronet = _network->neuronet();
            neuronet->removeEdge(_cellIndex, linkItem->_cellIndex);
        }
        
        _incoming.removeAll(linkItem);
    }
    
    void NeuroItem::addOutgoing(NeuroLinkItem *linkItem)
    {
        if (!_outgoing.contains(linkItem))
            _outgoing.append(linkItem);
    }
    
    void NeuroItem::removeOutgoing(NeuroLinkItem *linkItem)
    {
        _outgoing.removeAll(linkItem);
    }
    
    void NeuroItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
    {
        MainWindow::instance()->statusBar()->showMessage(QString("Hover enter %1; underMouse %2").arg((int)this).arg(this->isUnderMouse()));
        _in_hover = true;
        
        if (_network && _network->scene())
        {
            if (_network->scene()->selectedItem() != this)
                _network->scene()->setSelectedItem(0);
        }
        
        update(this->boundingRect());
    }
    
    void NeuroItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
    {
        MainWindow::instance()->statusBar()->showMessage(QString("Hover exit %1; underMouse %2").arg((int)this).arg(this->isUnderMouse()));
        _in_hover = false;

        update(this->boundingRect());
    }
    
    QRectF NeuroItem::boundingRect() const
    {
        return _labelRect;
    }
    
    QPainterPath NeuroItem::shape() const
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        if (!_label.isNull() && !_label.isEmpty())
            path.addRect(QRectF(_labelRect));
        return path;
    }
    
    bool NeuroItem::shouldHighlight() const
    {
        if (_in_hover || isSelected())
            return true;
        
        const LabScene *sc = dynamic_cast<const LabScene *>(this->scene());
        if (sc && sc->selectedItem() == this)
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
            qreal t = static_cast<qreal>(cell->value());
            
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
    
    void NeuroItem::drawLabel(QPainter *painter, QPen & pen, QBrush & brush)
    {
        if (!_label.isNull() && !_label.isEmpty())
        {
            painter->setBrush(brush);
            painter->setPen(pen);

            painter->drawText(_labelRect, Qt::AlignVCenter | Qt::AlignLeft, _label, &_labelRect);
        }
    }
    
    // I/O
    
    /// Writes ids of incoming and outgoing pointers.
    void NeuroItem::writePointerIds(QDataStream & ds) const
    {
        int n = _incoming.size();
        ds << n;
        for (int i = 0; i < n; ++i)
        {
            if (_incoming[i])
                ds << (int) _incoming[i]->_id;
            else
                ds << (int) 0;
        }
        
        n = _outgoing.size();
        ds << n;
        for (int i = 0; i < n; ++i)
        {
            if (_outgoing[i])
                ds << (int) _outgoing[i]->_id;
            else
                ds << (int) 0;
        }
    }
    
    void NeuroItem::readPointerIds(QDataStream & ds)
    {
        int n;
        
        _incoming.clear();
        ds >> n;
        for (int i = 0; i < n; ++i)
        {
            int id;
            ds >> id;
            if (id)
            {
                _incoming.append((NeuroLinkItem *) id);
            }
        }
        
        _outgoing.clear();
        ds >> n;
        for (int i = 0; i < n; ++i)
        {
            int id;
            ds >> id;
            if (id)
            {
                _outgoing.append((NeuroLinkItem *) id);
            }
        }
    }
    
    void NeuroItem::idsToPointers(QGraphicsScene *sc)
    {
        idsToPointersAux(_incoming, sc);
        idsToPointersAux(_outgoing, sc);
    }
    
    void NeuroItem::idsToPointersAux(QList<NeuroLinkItem *> & list, QGraphicsScene *sc)
    {
        QList<QGraphicsItem *> items = sc->items();
        
        for (QMutableListIterator<NeuroLinkItem *> in(list); in.hasNext(); in.next())
        {
            bool found = false;
            int id = (int) in.peekNext();
            for (QListIterator<QGraphicsItem *> i(items); i.hasNext(); i.next())
            {
                NeuroLinkItem *item = dynamic_cast<NeuroLinkItem *>(i.peekNext());
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
        data << static_cast<qint32>(item._cellIndex);
        item.writeBinary(data);
        item.writePointerIds(data);
        return data;
    }
    
    QDataStream & operator>> (QDataStream & data, NeuroItem & item)
    {
        data >> item._label;

        qint32 n;
        data >> n;
        item._cellIndex = static_cast<NeuroCell::NeuroIndex>(n);
        
        item.readBinary(data);
        item.readPointerIds(data);
        item._in_hover = false;
        return data;
    }
    
    // neuro stuff
    
    NeuroCell *NeuroItem::getCell()
    {
        return _cellIndex != -1 ? &((*_network->neuronet())[_cellIndex]) : 0;
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
