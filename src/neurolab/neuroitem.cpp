#include "neuroitem.h"
#include "mainwindow.h"
#include "neurolinkitem.h"
#include "labscene.h"
#include "../automata/exception.h"

#include <QStatusBar>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>

namespace NeuroLab
{
    
    const QColor NeuroItem::NORMAL_LINE_COLOR = Qt::black;
    const QColor NeuroItem::UNLINKED_LINE_COLOR = Qt::lightGray;
    const QColor NeuroItem::BACKGROUND_COLOR = Qt::white;
    
    const int NeuroItem::NORMAL_LINE_WIDTH = 1;
    const int NeuroItem::HOVER_LINE_WIDTH = 3;

    const int NeuroItem::NODE_WIDTH = 30;
    const int NeuroItem::ELLIPSE_WIDTH = 10;
    
    //////////////////////////////////////////////    
    
    int NeuroItem::NEXT_ID = 1;
    
    NeuroItem::NeuroItem()
        : QGraphicsItem(), _id(NEXT_ID++), _in_hover(false)
    {
        this->setFlag(QGraphicsItem::ItemIsSelectable, true);
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
        if (!_incoming.contains(linkItem))
            _incoming.append(linkItem);
    }
    
    void NeuroItem::removeIncoming(NeuroLinkItem *linkItem)
    {
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
    
    bool NeuroItem::shouldHighlight() const
    {
        if (_in_hover || isSelected())
            return true;
        
        const LabScene *sc = dynamic_cast<const LabScene *>(this->scene());
        if (sc && sc->itemToSelect() == dynamic_cast<const QGraphicsItem *>(this))
            return true;
        
        return false;
    }
    
    void NeuroItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
    {
        MainWindow::instance()->statusBar()->showMessage(QString("Hover enter %1; underMouse %2").arg((int)this).arg(this->isUnderMouse()));
        _in_hover = true;
        setSelected(true);
        update(this->boundingRect());
    }
    
    void NeuroItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
    {
        MainWindow::instance()->statusBar()->showMessage(QString("Hover exit %1; underMouse %2").arg((int)this).arg(this->isUnderMouse()));
        _in_hover = false;
        setSelected(false);
        update(this->boundingRect());
    }
    
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
        item.writeBinary(data);
        item.writePointerIds(data);
        return data;
    }
    
    QDataStream & operator>> (QDataStream & data, NeuroItem & item)
    {
        item.readBinary(data);
        item.readPointerIds(data);
        item._in_hover = false;
        return data;
    }
        
} // namespace NeuroLab
