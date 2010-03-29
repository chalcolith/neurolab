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
        NEUROITEM_DECLARE_CREATOR

    public:
        NeuroGridItem(NeuroLab::LabNetwork *network, const QPointF & scenePos);
        virtual ~NeuroGridItem();

        virtual bool canAttachTo(const QPointF &, NeuroItem *);
        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *);

    protected:
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;
    };

}

#endif // NEUROGRIDITEM_H
