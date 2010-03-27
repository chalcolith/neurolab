#ifndef NEUROGRIDITEM_H
#define NEUROGRIDITEM_H

#include "griditems_global.h"
#include "../neurogui/neuroitem.h"

namespace GridItems
{

    class GRIDITEMSSHARED_EXPORT NeuroGridItem
        : public NeuroLab::NeuroItem
    {
        Q_OBJECT
    public:
        NeuroGridItem(NeuroLab::LabNetwork *network);
        virtual ~NeuroGridItem();

        static NeuroLab::NeuroItem *create_new(NeuroLab::LabScene *scene, const QPointF & pos);

    protected:
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;
    };

}

#endif // NEUROGRIDITEM_H
