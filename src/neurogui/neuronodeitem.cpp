#include "neuronodeitem.h"
#include "neurolinkitem.h"
#include "labnetwork.h"
#include "labscene.h"

#include <QVector2D>
#include <QApplication>
#include <QtProperty>

#include <cmath>

using namespace NeuroLib;

namespace NeuroLab
{

    NEUROITEM_DEFINE_CREATOR(NeuroNodeItem, QObject::tr("Narrow|Node"));

    NeuroNodeItem::NeuroNodeItem(LabNetwork *network, const QPointF & scenePos)
        : NeuroNarrowItem(network, scenePos)
    {
        Q_ASSERT(network != 0 && network->neuronet() != 0);

        NeuroCell::NeuroIndex index = network->neuronet()->addNode(NeuroCell(NeuroCell::NODE));
        setCellIndex(index);
        setRect(QRectF(-NODE_WIDTH/2, -NODE_WIDTH/2, NODE_WIDTH, NODE_WIDTH));
    }

    NeuroNodeItem::~NeuroNodeItem()
    {
    }

    bool NeuroNodeItem::canCreateNewOnMe(const QString &typeName, const QPointF &) const
    {
        return typeName.indexOf("LinkItem") >= 0;
    }

    void NeuroNodeItem::buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem)
    {
        NeuroNarrowItem::buildProperties(manager, parentItem);
        parentItem->setPropertyName(tr("Node"));
    }

    void NeuroNodeItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroNarrowItem::addToShape(drawPath, texts);
        drawPath.addEllipse(rect());

        const NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            texts.append(TextPathRec(QPointF(-4, 4), QString::number(cell->current().weight())));
        }
    }

    bool NeuroNodeItem::canBeAttachedBy(const QPointF &, NeuroItem *item)
    {
        // can only be connected to by links
        return dynamic_cast<NeuroLinkItem *>(item);
    }

    void NeuroNodeItem::adjustLinks()
    {
        adjustLinksAux(incoming());
        adjustLinksAux(outgoing());
    }

    void NeuroNodeItem::adjustLinksAux(const QList<NeuroItem *> & list)
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

                toPos *= NeuroNarrowItem::NODE_WIDTH/2 + 2;

                QPointF *point = frontLink ? &front : &back;
                *point = (center + toPos).toPointF();

                link->setLine(back, front);
            }
        }
    }

    void NeuroNodeItem::writeBinary(QDataStream & data) const
    {
        NeuroNarrowItem::writeBinary(data);
        data << _rect;
    }

    void NeuroNodeItem::readBinary(QDataStream & data)
    {
        NeuroNarrowItem::readBinary(data);

        QRectF r;
        data >> r;
        setRect(r);
    }

} // namespace NeuroLab
