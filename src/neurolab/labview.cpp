#include "labview.h"

namespace NeuroLab
{
    
    LabView::LabView(LabScene *scene, QWidget *parent)
        : QGraphicsView(scene, parent)
    {
    }
    
    LabView::~LabView()
    {
    }
        
} // namespace NeuroLab
