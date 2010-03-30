#ifndef NEURONODEITEM_H
#define NEURONODEITEM_H

#include "neurogui_global.h"
#include "neuronarrowitem.h"

namespace NeuroLab
{

    /// An item that represents a node in Narrow notation.
    class NEUROGUISHARED_EXPORT NeuroNodeItem
        : public NeuroNarrowItem
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

        QRectF _rect;

    public:
        NeuroNodeItem(LabNetwork *network, const QPointF & scenePos);
        virtual ~NeuroNodeItem();

        /// The node will be drawn as an ellipse within this rectangle (in item coordinates).
        /// \see setRect()
        const QRectF & rect() const { return _rect; }

        /// The node will be drawn as an ellipse within this rectangle (in item coordinates).
        /// \see rect()
        void setRect(const QRectF & r) { _rect = r; update(_rect); }

        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);

        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *);
        virtual void adjustLinks();

    protected:
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;
        virtual bool canCreateNewOnMe(const QString & typeName, const QPointF & pos) const;

        virtual void writeBinary(QDataStream &) const;
        virtual void readBinary(QDataStream &);

    private:
        void adjustLinksAux(const QList<NeuroItem *> &);
    };

    /// An item that represents an oscillator.
    class NEUROGUISHARED_EXPORT NeuroOscillatorItem
        : public NeuroNodeItem
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

        QtVariantProperty *_phase_property;
        QtVariantProperty *_peak_property;
        QtVariantProperty *_gap_property;
        QtVariantProperty *_value_property;

    public:
        NeuroOscillatorItem(LabNetwork *network, const QPointF & scenePos);
        virtual ~NeuroOscillatorItem();

        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);
        virtual void updateProperties();
        virtual void propertyValueChanged(QtProperty *property, const QVariant & value);

    public slots:
        virtual void reset();

    protected:
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;
    };

} // namespace NeuroLab

#endif // NEURONODEITEM_H
