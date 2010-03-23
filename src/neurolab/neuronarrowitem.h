#ifndef NEURONARROWITEM_H
#define NEURONARROWITEM_H

#include "neuroitem.h"
#include "../neurolib/neurocell.h"

namespace NeuroLab
{

    class NeuroNarrowItem : public NeuroItem
    {
        Q_OBJECT

        NeuroLib::NeuroCell::NeuroIndex _cellIndex;

        QtVariantProperty *frozen_property;
        QtVariantProperty *inputs_property;
        QtVariantProperty *weight_property;
        QtVariantProperty *run_property;
        QtVariantProperty *value_property;

    public:
        NeuroNarrowItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex = -1);
        virtual ~NeuroNarrowItem();

        static NeuroItem *create_new(LabScene *scene, const QPointF & pos);

        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);
        virtual void updateProperties();

        virtual bool addIncoming(NeuroItem *linkItem);
        virtual bool removeIncoming(NeuroItem *linkItem);

        virtual void setPenProperties(QPen & pen) const;

        virtual void reset();
        virtual void toggleActivated();
        virtual void toggleFrozen();

    public slots:
        virtual void propertyValueChanged(QtProperty *property, const QVariant & value);

    protected:
        const NeuroLib::NeuroCell *getCell() const;
        NeuroLib::NeuroCell *getCell();

        virtual void writeBinary(QDataStream &) const;
        virtual void readBinary(QDataStream &);
    };

}

#endif // NEURONARROWITEM_H
