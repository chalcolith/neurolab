#ifndef NEURONODEITEM_H
#define NEURONODEITEM_H

#include "neuronarrowitem.h"

#include <QPainter>
#include <QPainterPath>

namespace NeuroLab
{

    /// An item that represents a node in Narrow notation.
    class NeuroNodeItem
        : public NeuroNarrowItem
    {
        QRectF _rect;

    public:
        NeuroNodeItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex);
        virtual ~NeuroNodeItem();

        /// Creator function for nodes.
        static NeuroItem *create_new(LabScene *scene, const QPointF & pos);

        /// The node will be drawn as an ellipse within this rectangle (in scene coordinates).
        /// \see setRect()
        const QRectF & rect() const { return _rect; }

        /// The node will be drawn as an ellipse within this rectangle (in scene coordinates).
        /// \see rect()
        void setRect(const QRectF & r) { _rect = r; update(_rect); }

        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);
        virtual void addToShape() const;

        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *);
        virtual void adjustLinks();

    private:
        void adjustLinksAux(QList<NeuroItem *> &);

    protected:
        virtual bool canCreateNewOnMe(const QString & typeName, const QPointF & pos) const;

        virtual void writeBinary(QDataStream &) const;
        virtual void readBinary(QDataStream &);
    };

} // namespace NeuroLab

#endif // NEURONODEITEM_H
