#include "neuroitem.h"
#include "mainwindow.h"

#include <QStatusBar>

namespace NeuroLab
{
    
    NeuroItem::NeuroItem()
        : QGraphicsItem(), _in_hover(false)
    {
        setAcceptHoverEvents(true);
    }
    
    void NeuroItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
    {
        MainWindow::instance()->statusBar()->showMessage(QString("Hover enter %1").arg((int)this));        
        _in_hover = true;
        update(boundingRect());
    }
    
    void NeuroItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
    {
        MainWindow::instance()->statusBar()->showMessage(QString("Hover exit %1").arg((int)this));
        _in_hover = false;
        update(boundingRect());
    }
    
} // namespace NeuroLab
