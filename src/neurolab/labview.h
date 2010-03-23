#ifndef LABVIEW_H
#define LABVIEW_H

#include <QGraphicsView>

#include "labscene.h"

namespace NeuroLab
{

    /// Derived to display neural network items.
    class LabView
        : public QGraphicsView
    {
        Q_OBJECT

    public:
        LabView(LabScene *scene, QWidget *parent);
        virtual ~LabView();
    };

} // namespace NeuroLab

#endif // LABVIEW_H
