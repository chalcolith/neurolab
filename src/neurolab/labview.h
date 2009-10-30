#ifndef LABVIEW_H
#define LABVIEW_H

#include <QGraphicsView>

#include "labscene.h"

namespace NeuroLab
{
    
    class LabView 
            : public QGraphicsView
    {
    public:
        LabView(LabScene *scene, QWidget *parent);
        virtual ~LabView();
    };
    
} // namespace NeuroLab

#endif // LABVIEW_H
