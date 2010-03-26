#ifndef LABVIEW_H
#define LABVIEW_H

#include "neurogui_global.h"

#include <QGraphicsView>

namespace NeuroLab
{

    class LabScene;

    /// Derived to display neural network items.
    class NEUROGUISHARED_EXPORT LabView
        : public QGraphicsView
    {
        Q_OBJECT

    public:
        LabView(LabScene *scene, QWidget *parent);
        virtual ~LabView();
    };

} // namespace NeuroLab

#endif // LABVIEW_H
