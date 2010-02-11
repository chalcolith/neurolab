#include "neuronodeitem.h"
#include "neurolinkitem.h"
#include "labnetwork.h"

#include <QVector2D>
#include <QApplication>

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

    void NeuroNodeItem::buildShape()
    {
        NeuroItem::buildShape();
        _path->addEllipse(rect());
                
        NeuroCell *cell = const_cast<NeuroNodeItem *>(this)->getCell();
        if (cell)
        {
            int threshold = static_cast<int>(cell->input_threshold() + static_cast<NeuroCell::NeuroValue>(0.5));
            QString str("%1");
            _textPath->addText(-3, 2, QApplication::font(), str.arg(threshold));
        }
    }

    bool NeuroNodeItem::canLinkTo(EditInfo &, NeuroItem *)
    {
        return true;
    }

    bool NeuroNodeItem::handlePickup(EditInfo &)
    {
        return true;
    }

    void NeuroNodeItem::handleMove(EditInfo & info)
    {
        // move
        setPos(info.scenePos.x(), info.scenePos.y());
        adjustLinks();

        // find a link to attach to
        NeuroLinkItem *itemAtPos = 0;
        bool front = false;
        for (QListIterator<QGraphicsItem *> i(this->collidingItems(Qt::IntersectsItemShape)); i.hasNext(); i.next())
        {
            itemAtPos = dynamic_cast<NeuroLinkItem *>(i.peekNext());
            if (itemAtPos && !(itemAtPos->frontLinkTarget() == this || itemAtPos->backLinkTarget() == this))
            {
                // figure out which end to link
                qreal frontDist = (QVector2D(info.scenePos) - QVector2D(itemAtPos->line().p2())).lengthSquared();
                qreal backDist = (QVector2D(info.scenePos) - QVector2D(itemAtPos->line().p1())).lengthSquared();
                front = frontDist < backDist;

                // don't attach to a link that's already attached to something
                if (!(front ? itemAtPos->frontLinkTarget() : itemAtPos->backLinkTarget()) && itemAtPos->canLinkTo(info, this))
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

            this->adjustLinks();
        }
    }

    void NeuroNodeItem::adjustLinks()
    {
        QVector2D center(pos());
        QVector2D bottom = QVector2D(center.x(), center.y() + NeuroItem::NODE_WIDTH/2);
        QVector2D top = QVector2D(center.x(), center.y() - NeuroItem::NODE_WIDTH/2);

        for (QListIterator<NeuroItem *> ln(_incoming); ln.hasNext(); ln.next())
        {
            NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(ln.peekNext());
            if (link)
                link->setLine(link->line().p1(), bottom.toPointF());
        }

        for (QListIterator<NeuroItem *> ln(_outgoing); ln.hasNext(); ln.next())
        {
            NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(ln.peekNext());
            if (link)
                link->setLine(top.toPointF(), link->line().p2());
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
