#ifndef NEURONODEITEM_H
#define NEURONODEITEM_H

#include "neuroitem.h"

#include <QPainter>
#include <QPainterPath>

namespace NeuroLab
{
    
    class NeuroNodeItem
        : public NeuroItem
    {
        QRectF _rect;
        
    public:
        NeuroNodeItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex);
        virtual ~NeuroNodeItem();
        
        static NeuroItem *create_new(LabScene *scene, const QPointF & pos);
        
        const QRectF & rect() const { return _rect; }
        void setRect(const QRectF & r) { _rect = r; update(_rect); }
        
        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);
        virtual void buildShape();
        
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);
        
        virtual bool canLinkTo(NeuroItem *item);
        virtual void adjustLinks();
        
    private:
        void adjustLinksAux(QList<NeuroItem *> &);
        
    protected:
        virtual void writeBinary(QDataStream &) const;
        virtual void readBinary(QDataStream &);
    };
    
} // namespace NeuroLab

#endif // NEURONODEITEM_H
