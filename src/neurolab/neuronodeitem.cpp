#include "neuronodeitem.h"
#include "neurolinkitem.h"
#include "labnetwork.h"

#include <QVector2D>
#include <QApplication>

#include <QtProperty>

#include <cmath>

using namespace NeuroLib;

namespace NeuroLab
{
    
    NEUROITEM_DEFINE_CREATOR(NeuroNodeItem);

    NeuroNodeItem::NeuroNodeItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex)
        : NeuroItem(network, cellIndex)
    {
        setRect(QRectF(-NODE_WIDTH/2, -NODE_WIDTH/2, NODE_WIDTH, NODE_WIDTH));
    }
        
    NeuroNodeItem::~NeuroNodeItem()
    {
    }
    
    NeuroItem *NeuroNodeItem::create_new(LabScene *scene, const QPointF & pos)
    {
        NeuroCell::NeuroIndex index = scene->network()->neuronet()->addNode(NeuroCell(NeuroCell::NODE));
        NeuroNodeItem *item = new NeuroNodeItem(scene->network(), index);
        item->setPos(pos.x(), pos.y());
        return item;
    }
    
    void NeuroNodeItem::buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem)
    {
        NeuroItem::buildProperties(manager, parentItem);
        parentItem->setPropertyName(tr("Node"));
    }

    void NeuroNodeItem::buildShape()
    {
        NeuroItem::buildShape();
        _path->addEllipse(rect());
                
        NeuroCell *cell = const_cast<NeuroNodeItem *>(this)->getCell();
        if (cell)
        {
            _texts.append(TextPathRec(QPointF(-8, 4), QString::number(cell->weight())));
        }
    }
    
    QVariant NeuroNodeItem::itemChange(GraphicsItemChange change, const QVariant & value)
    {
        if (change == QGraphicsItem::ItemPositionHasChanged)
        {
            QPointF scenePos = value.toPointF();
            
            // find a link to attach to
            NeuroLinkItem *itemAtPos = 0;
            bool front = false;
            for (QListIterator<QGraphicsItem *> i(this->collidingItems(Qt::IntersectsItemShape)); i.hasNext(); i.next())
            {
                itemAtPos = dynamic_cast<NeuroLinkItem *>(i.peekNext());
                if (itemAtPos && !(itemAtPos->frontLinkTarget() == this || itemAtPos->backLinkTarget() == this))
                {
                    // figure out which end to link
                    qreal frontDist = (QVector2D(scenePos) - QVector2D(itemAtPos->line().p2())).lengthSquared();
                    qreal backDist = (QVector2D(scenePos) - QVector2D(itemAtPos->line().p1())).lengthSquared();
                    front = frontDist < backDist;
    
                    // don't attach to a link that's already attached to something
                    if (!(front ? itemAtPos->frontLinkTarget() : itemAtPos->backLinkTarget()) && itemAtPos->canLinkTo(this))
                        break;
                }
    
                itemAtPos = 0;
            }
    
            // connect to link
            if (itemAtPos)
            {
                if (front)
                    itemAtPos->setFrontLinkTarget(this);
                else
                    itemAtPos->setBackLinkTarget(this);
            }
            
            //
            adjustLinks();            
        }
        
        return value;
    }

    bool NeuroNodeItem::canLinkTo(NeuroItem *)
    {
        return true;
    }

    void NeuroNodeItem::adjustLinks()
    {
        adjustLinksAux(_incoming);
        adjustLinksAux(_outgoing);
    }
    
    void NeuroNodeItem::adjustLinksAux(QList<NeuroItem *> & list)
    {
        QVector2D center(pos());
        
        for (QListIterator<NeuroItem *> ln(list); ln.hasNext(); ln.next())
        {
            NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(ln.peekNext());
            if (link)
            {
                bool frontLink = link->frontLinkTarget() == this;
                bool backLink = link->backLinkTarget() == this;
                
                QPointF front = link->line().p2();
                QPointF back = link->line().p1();
                
                QVector2D toPos = QVector2D(link->pos()) - center;
                
                if (frontLink && backLink)
                {
                    toPos.normalize();                
                    double angle = ::atan2(toPos.y(), toPos.x());
                    angle += (frontLink ? 30.0 : -30.0) * M_PI / 180.0;
                    toPos.setX(::cos(angle));
                    toPos.setY(::sin(angle));
                }
                else
                {
                    //if (toPos.x() < toPos.y())
                    //    toPos.setX(toPos.x() * 2);
                    //else
                    //    toPos.setY(toPos.y() * 2);
                    toPos.normalize();
                }
                
                toPos *= NeuroItem::NODE_WIDTH/2 + 2;
                
                QPointF *point = frontLink ? &front : &back;
                *point = (center + toPos).toPointF();
                
                link->setLine(back, front);
            }
        }        
    }

    void NeuroNodeItem::writeBinary(QDataStream & data) const
    {
        data << _id;
        data << pos();
        data << _rect;
    }

    void NeuroNodeItem::readBinary(QDataStream & data)
    {
        data >> _id;

        QPointF p;
        data >> p;
        setPos(p);

        QRectF r;
        data >> r;
        setRect(r);
    }

} // namespace NeuroLab
