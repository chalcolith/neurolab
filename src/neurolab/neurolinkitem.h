#ifndef NEUROLINKITEM_H
#define NEUROLINKITEM_H

#include "neuronarrowitem.h"

#include <QVector2D>
#include <QPainterPath>
#include <QList>

namespace NeuroLab
{

    class NeuroLinkItem
        : public NeuroNarrowItem
    {
    protected:
        QLineF _line;
        mutable QVector2D c1, c2;

        NeuroItem *_frontLinkTarget, *_backLinkTarget;
        bool dragFront, settingLine;

    public:
        NeuroLinkItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex);
        virtual ~NeuroLinkItem();

        QLineF line() const { return _line; }
        void setLine(const QLineF & l);
        void setLine(const qreal & x1, const qreal & y1, const qreal & x2, const qreal & y2);
        void setLine(const QPointF & p1, const QPointF & p2);

        virtual bool addIncoming(NeuroItem *linkItem);
        virtual bool removeIncoming(NeuroItem *linkItem);
        virtual bool addOutgoing(NeuroItem *linkItem);
        virtual bool removeOutgoing(NeuroItem *linkItem);

        NeuroItem *frontLinkTarget() { return _frontLinkTarget; }
        void setFrontLinkTarget(NeuroItem *linkTarget);

        NeuroItem *backLinkTarget() { return _backLinkTarget; }
        void setBackLinkTarget(NeuroItem *linkTarget);

        virtual void addToShape() const;

        virtual bool canAttachTo(const QPointF &, NeuroItem *);
        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *);

        virtual void attachTo(NeuroItem *);
        virtual bool handleMove(const QPointF & mousePos, QPointF & movePos);
        virtual void adjustLinks();

    private:
        void updatePos();

    protected:
        virtual void setPenProperties(QPen & pen) const;
        virtual void setBrushProperties(QBrush & brush) const;

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        virtual void writeBinary(QDataStream &) const;
        virtual void readBinary(QDataStream &);

        virtual void writePointerIds(QDataStream &) const;
        virtual void readPointerIds(QDataStream &);
        virtual void idsToPointers(QGraphicsScene *);
    };


    class NeuroExcitoryLinkItem
        : public NeuroLinkItem
    {
    public:
        NeuroExcitoryLinkItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex);
        virtual ~NeuroExcitoryLinkItem();

        static NeuroItem *create_new(LabScene *scene, const QPointF & pos);

        virtual bool canAttachTo(const QPointF &, NeuroItem *);

        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);
        virtual void addToShape() const;
    };


    class NeuroInhibitoryLinkItem
        : public NeuroLinkItem
    {
    public:
        NeuroInhibitoryLinkItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex);
        virtual ~NeuroInhibitoryLinkItem();

        static NeuroItem *create_new(LabScene *scene, const QPointF & pos);

        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);
        virtual void addToShape() const;
    };

} // namespace NeuroLab

#endif // NEUROLINKITEM_H
