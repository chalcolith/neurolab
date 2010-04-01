#include "labview.h"
#include "labscene.h"

namespace NeuroLab
{

    LabView::LabView(LabScene *scene, QWidget *parent)
        : QGraphicsView(scene, parent)
    {
        setSceneRect(0, 0, 10000, 10000);
        setAlignment(Qt::AlignLeft | Qt::AlignTop);
        setDragMode(QGraphicsView::RubberBandDrag);
        setResizeAnchor(QGraphicsView::AnchorViewCenter);
    }

    LabView::~LabView()
    {
    }

} // namespace NeuroLab
