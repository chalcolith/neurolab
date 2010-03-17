#ifndef NEUROLINKITEM_H
#define NEUROLINKITEM_H

#include "neuroitem.h"

#include <QVector2D>
#include <QPainterPath>
#include <QList>

namespace NeuroLab
{
    
    class NeuroLinkItem
        : public NeuroItem
    {
    protected:
        QLineF _line;
        QVector2D c1, c2;
        
        NeuroItem *_frontLinkTarget, *_backLinkTarget;
        bool dragFront, settingLine;
        
    public:
        NeuroLinkItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex);
        virtual ~NeuroLinkItem();
        
        QLineF line() const { return _line; }
        void setLine(const QLineF & l);
        void setLine(const qreal & x1, const qreal & y1, const qreal & x2, const qreal & y2);
        void setLine(const QPointF & p1, const QPointF & p2);
        
        virtual void addIncoming(NeuroItem *linkItem);
        virtual void removeIncoming(NeuroItem *linkItem);
        virtual void addOutgoing(NeuroItem *linkItem);
        virtual void removeOutgoing(NeuroItem *linkItem);
        
        NeuroItem *frontLinkTarget() { return _frontLinkTarget; }
        void setFrontLinkTarget(NeuroItem *linkTarget);
        
        NeuroItem *backLinkTarget() { return _backLinkTarget; }
        void setBackLinkTarget(NeuroItem *linkTarget);

        virtual void buildShape();        
        
        virtual bool canLinkTo(NeuroItem *item);
        virtual void adjustLinks();
        virtual void handleMove();
        
    private:
        void updatePos();
        
    protected:
        virtual void setPenProperties(QPen & pen);
        virtual void setBrushProperties(QBrush & brush);

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
        
        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);
        virtual void buildShape();
    };
    
    
    class NeuroInhibitoryLinkItem
        : public NeuroLinkItem
    {
    public:
        NeuroInhibitoryLinkItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex);
        virtual ~NeuroInhibitoryLinkItem();

        static NeuroItem *create_new(LabScene *scene, const QPointF & pos);
        
        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);
        virtual void buildShape();
    };
    
} // namespace NeuroLab

#endif // NEUROLINKITEM_H
