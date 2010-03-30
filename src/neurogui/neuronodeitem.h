#ifndef NEURONODEITEM_H
#define NEURONODEITEM_H

#include "neurogui_global.h"
#include "neuronarrowitem.h"

namespace NeuroLab
{

    /// Base class for node objects.
    class NEUROGUISHARED_EXPORT NeuroNodeItemBase
        : public NeuroNarrowItem
    {
        Q_OBJECT

        QRectF _rect;

    public:
        NeuroNodeItemBase(LabNetwork *network, const QPointF & scenePos);
        virtual ~NeuroNodeItemBase();

        /// The node will be drawn as an ellipse within this rectangle (in item coordinates).
        /// \see setRect()
        const QRectF & rect() const { return _rect; }

        /// The node will be drawn as an ellipse within this rectangle (in item coordinates).
        /// \see rect()
        void setRect(const QRectF & r) { _rect = r; update(_rect); }

        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *);
        virtual void adjustLinks();

    protected:
        virtual bool canCreateNewOnMe(const QString & typeName, const QPointF & pos) const;

        virtual void writeBinary(QDataStream &) const;
        virtual void readBinary(QDataStream &);

    private:
        void adjustLinksAux(const QList<NeuroItem *> &);
    };


    /// An item that represents a node in Narrow notation.
    class NEUROGUISHARED_EXPORT NeuroNodeItem
        : public NeuroNodeItemBase
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

        Property<NeuroNodeItem, QVariant::Bool, bool, bool> _frozen_property;
        Property<NeuroNodeItem, QVariant::Double, double, NeuroLib::NeuroCell::NeuroValue> _inputs_property;
        Property<NeuroNodeItem, QVariant::Double, double, NeuroLib::NeuroCell::NeuroValue> _run_property;

    public:
        NeuroNodeItem(LabNetwork *network, const QPointF & scenePos);
        virtual ~NeuroNodeItem();

        virtual QString uiName() const { return tr("Node"); }

        bool frozen() const;
        void setFrozen(const bool & f);

        NeuroLib::NeuroCell::NeuroValue inputs() const;
        void setInputs(const NeuroLib::NeuroCell::NeuroValue &);

        NeuroLib::NeuroCell::NeuroValue run() const;
        void setRun(const NeuroLib::NeuroCell::NeuroValue &);

    public slots:
        virtual void reset();

        /// Toggles the output value of the item between 0 and 1.
        virtual void toggleActivated();

        /// Toggles whether or not the item is frozen (i.e. whether or not its output value will change during a time step).
        virtual void toggleFrozen();

    protected:
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;
        virtual void buildActionMenu(LabScene *, const QPointF &, QMenu &);
    };


    /// An item that represents an oscillator.
    class NEUROGUISHARED_EXPORT NeuroOscillatorItem
        : public NeuroNodeItemBase
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

        Property<NeuroOscillatorItem, QVariant::Int, int, NeuroLib::NeuroCell::NeuroStep> _phase_property;
        Property<NeuroOscillatorItem, QVariant::Int, int, NeuroLib::NeuroCell::NeuroStep> _peak_property;
        Property<NeuroOscillatorItem, QVariant::Int, int, NeuroLib::NeuroCell::NeuroStep> _gap_property;

    public:
        NeuroOscillatorItem(LabNetwork *network, const QPointF & scenePos);
        virtual ~NeuroOscillatorItem();

        virtual QString uiName() const { return tr("Oscillator"); }

        NeuroLib::NeuroCell::NeuroStep phase() const;
        void setPhase(const NeuroLib::NeuroCell::NeuroStep &);

        NeuroLib::NeuroCell::NeuroStep peak() const;
        void setPeak(const NeuroLib::NeuroCell::NeuroStep &);

        NeuroLib::NeuroCell::NeuroStep gap() const;
        void setGap(const NeuroLib::NeuroCell::NeuroStep &);

    public slots:
        virtual void reset();

    protected:
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;
    };

} // namespace NeuroLab

#endif // NEURONODEITEM_H
