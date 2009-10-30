#include "neuronodeitem.h"

#include <QBrush>

namespace NeuroLab
{
    
    NeuroNodeItem::NeuroNodeItem()
            : QGraphicsEllipseItem()
    {
        setBrush(QBrush());
        setRect(0, 0, 20, 20);
    }
    
} // namespace NeuroLab
