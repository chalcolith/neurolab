#include "neuroitem.h"
#include "mainwindow.h"

#include <QStatusBar>

namespace NeuroLab
{
    
    const QColor NeuroItem::NORMAL_LINE_COLOR = Qt::black;
    const QColor NeuroItem::UNLINKED_LINE_COLOR = Qt::lightGray;
    const QColor NeuroItem::BACKGROUND_COLOR = Qt::white;
    
    const int NeuroItem::NORMAL_LINE_WIDTH = 1;
    const int NeuroItem::HOVER_LINE_WIDTH = 3;

    const int NeuroItem::NODE_WIDTH = 30;
    const int NeuroItem::ELLIPSE_WIDTH = 8;
    
    //////////////////////////////////////////////    
    
    NeuroItem::NeuroItem()
        : QGraphicsItem(), _in_hover(true)
    {
        this->setAcceptHoverEvents(true);
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
    
    void NeuroItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
    {
        MainWindow::instance()->statusBar()->showMessage(QString("Hover enter %1; underMouse %2").arg((int)this).arg(this->isUnderMouse()));
        _in_hover = true;
        update(this->boundingRect());
    }
    
    void NeuroItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
    {
        MainWindow::instance()->statusBar()->showMessage(QString("Hover exit %1; underMouse %2").arg((int)this).arg(this->isUnderMouse()));
        _in_hover = false;
        update(this->boundingRect());
    }

} // namespace NeuroLab
