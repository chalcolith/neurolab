#include "neuroitem.h"
#include "mainwindow.h"
#include "neurolinkitem.h"

#include <QStatusBar>

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
    
    NeuroItem::NeuroItem()
        : QGraphicsItem(), _in_hover(true)
    {
        this->setAcceptHoverEvents(true);
    }
    
    NeuroItem::~NeuroItem()
    {
        NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(this);
        if (link)
        {
            QListIterator<NeuroLinkItem *> in(_incoming);
            while (in.hasNext())
            {
                in.next()->removeOutgoing(link);
            }
            
            QListIterator<NeuroLinkItem *> out(_outgoing);
            while (out.hasNext())
            {
                out.next()->removeIncoming(link);
            }
        }
    }
    
    void NeuroItem::bringToFront()
    {
        qreal highest_z = this->zValue();
        QListIterator<QGraphicsItem *> i(this->collidingItems());
        while (i.hasNext())
        {
            QGraphicsItem *item = i.next();
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

} // namespace NeuroLab
