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

        Property<NeuroNarrowItem, QVariant::Double, double, NeuroLib::NeuroCell::NeuroValue> _value_property;

        NeuroLib::NeuroCell::NeuroIndex _cellIndex; ///< The index of the neural network cell that underlies this item.

    public:
        NeuroNarrowItem(LabNetwork *network, const QPointF & scenePos);
        virtual ~NeuroNarrowItem();

        NeuroLib::NeuroCell::NeuroValue outputValue() const;
        void setOutputValue(const NeuroLib::NeuroCell::NeuroValue & value);

        virtual bool addIncoming(NeuroItem *linkItem);
        virtual bool removeIncoming(NeuroItem *linkItem);

        virtual void setPenProperties(QPen & pen) const;

    public slots:
        /// Resets the item.  If it is not frozen, sets the output value to zero.
        virtual void reset();

    protected:
        NeuroLib::NeuroCell::NeuroIndex cellIndex() const { return _cellIndex; }
        void setCellIndex(const NeuroLib::NeuroCell::NeuroIndex & index) { _cellIndex = index; }

        /// \return A pointer to the neural network cell's previous and current state.
        const NeuroLib::NeuroNet::ASYNC_STATE *getCell() const;

        /// \return A pointer to the neural network cell's previous and current state.
        NeuroLib::NeuroNet::ASYNC_STATE *getCell();

        virtual void writeBinary(QDataStream &) const;
        virtual void readBinary(QDataStream &);
    };

}

#endif // NEURONARROWITEM_H
