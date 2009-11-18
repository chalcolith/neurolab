#include "neuroitem.h"
#include "mainwindow.h"

#include <QStatusBar>

namespace NeuroLab
{
    
    NeuroItem::NeuroItem()
        : QGraphicsItem()
    {
        setAcceptHoverEvents(true);
    }
    
    void NeuroItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
    {
        MainWindow::instance()->statusBar()->showMessage(QString("Hover enter %1").arg((int)this));        
        setSelected(true);
        update(boundingRect());
    }
    
    void NeuroItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
    {
        MainWindow::instance()->statusBar()->showMessage(QString("Hover exit %1").arg((int)this));
        setSelected(false);
        update(boundingRect());
    }
    
} // namespace NeuroLab
