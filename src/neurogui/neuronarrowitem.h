#ifndef NEURONARROWITEM_H
#define NEURONARROWITEM_H

#include "neurogui_global.h"
#include "neuroitem.h"
#include "../neurolib/neuronet.h"

namespace NeuroLab
{

    /// Base class for items in Narrow notation.  These include nodes and one-directional links.
    class NEUROGUISHARED_EXPORT NeuroNarrowItem
        : public NeuroItem
    {
        Q_OBJECT

        NeuroLib::NeuroCell::NeuroIndex _cellIndex; ///< The index of the neural network cell that underlies this item.

        QtVariantProperty *_frozen_property; ///< Property for controlling whether or not the item is frozen.
        QtVariantProperty *_inputs_property; ///< Property for controlling the item's input weight.
        QtVariantProperty *_weight_property; ///< Property for controlling the item's output weight.
        QtVariantProperty *_run_property;    ///< Property for controlling the item's sigmoid curve's slope.
        QtVariantProperty *_value_property;  ///< Property for controlling the item's output value.

    public:
        NeuroNarrowItem(LabNetwork *network, const QPointF & scenePos);
        virtual ~NeuroNarrowItem();

        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);
        virtual void updateProperties();

        virtual bool addIncoming(NeuroItem *linkItem);
        virtual bool removeIncoming(NeuroItem *linkItem);

        virtual void setPenProperties(QPen & pen) const;

    public slots:
        /// Resets the item.  If it is not frozen, sets the output value to zero.
        virtual void reset();

        /// Toggles the output value of the item between 0 and 1.
        virtual void toggleActivated();

        /// Toggles whether or not the item is frozen (i.e. whether or not its output value will change during a time step).
        virtual void toggleFrozen();

        virtual void propertyValueChanged(QtProperty *property, const QVariant & value);

    protected:
        NeuroLib::NeuroCell::NeuroIndex cellIndex() const { return _cellIndex; }
        void setCellIndex(const NeuroLib::NeuroCell::NeuroIndex & index) { _cellIndex = index; }

        void buildActionMenuAux(LabScene *scene, const QPointF &pos, QMenu &menu);

        /// \return A pointer to the neural network cell's previous and current state.
        const NeuroLib::NeuroNet::ASYNC_STATE *getCell() const;

        /// \return A pointer to the neural network cell's previous and current state.
        NeuroLib::NeuroNet::ASYNC_STATE *getCell();

        virtual void writeBinary(QDataStream &) const;
        virtual void readBinary(QDataStream &);
    };

}

#endif // NEURONARROWITEM_H
