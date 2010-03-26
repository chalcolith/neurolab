#include "labview.h"
#include "labscene.h"

namespace NeuroLab
{

    LabView::LabView(LabScene *scene, QWidget *parent)
        : QGraphicsView(scene, parent)
    {
        setDragMode(QGraphicsView::RubberBandDrag);
    }

    LabView::~LabView()
    {
    }

} // namespace NeuroLab
