#ifndef NEURONODEITEM_H
#define NEURONODEITEM_H

#include "neuronarrowitem.h"

#include <QPainter>
#include <QPainterPath>

namespace NeuroLab
{

    class NeuroNodeItem
        : public NeuroNarrowItem
    {
        QRectF _rect;

    public:
        NeuroNodeItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex);
        virtual ~NeuroNodeItem();

        static NeuroItem *create_new(LabScene *scene, const QPointF & pos);

        const QRectF & rect() const { return _rect; }
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
