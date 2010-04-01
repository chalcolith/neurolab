#include "labview.h"
#include "labscene.h"

namespace NeuroLab
{

    LabView::LabView(LabScene *scene, QWidget *parent)
        : QGraphicsView(scene, parent)
    {
        setAlignment(Qt::AlignCenter);
        setDragMode(QGraphicsView::RubberBandDrag);
        setResizeAnchor(QGraphicsView::AnchorViewCenter);
    }

    LabView::~LabView()
    {
    }

} // namespace NeuroLab
