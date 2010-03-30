#include "neuronarrowitem.h"
#include "neuronodeitem.h"
#include "neurolinkitem.h"
#include "labnetwork.h"

#include <QMenu>
#include <QPen>
#include <QGraphicsScene>

#include <QtVariantProperty>

using namespace NeuroLib;

namespace NeuroLab
{

    NeuroNarrowItem::NeuroNarrowItem(LabNetwork *network, const QPointF & scenePos)
        : NeuroItem(network, scenePos), 
        _cellIndex(-1),
        _value_property(this, &outputValue, &setOutputValue)
    {
    }

    NeuroNarrowItem::~NeuroNarrowItem()
    {
    }
    
    const NeuroCell::NeuroValue & NeuroNarrowItem::outputValue() const
    {
        NeuroNet::ASYNC_STATE *cell = getCell();
        return cell ? cell->current().outputValue() : 0.0f;
    }
    
    void NeuroNarrowItem::setOutputValue(const NeuroCell::NeuroValue & value)
    {
        NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            cell->current()->setOutputValue(value);
            cell->former()->setOutputValue(value);
        }
    }

    void NeuroNarrowItem::buildActionMenuAux(LabScene *, const QPointF &, QMenu & menu)
    {
        menu.addAction(tr("Activate/Deactivate"), this, SLOT(toggleActivated()));
        menu.addAction(tr("Freeze/Unfreeze"), this, SLOT(toggleFrozen()));
    }

    bool NeuroNarrowItem::addIncoming(NeuroItem *item)
    {
        if (!network() || !network()->neuronet())
            return false;

        NeuroNarrowItem *linkItem;

        if (NeuroItem::addIncoming(item) && (linkItem = dynamic_cast<NeuroNarrowItem *>(item)))
        {
            if (_cellIndex != -1 && linkItem->_cellIndex != -1)
                network()->neuronet()->addEdge(_cellIndex, linkItem->_cellIndex);
            return true;
        }

        return false;
    }

    bool NeuroNarrowItem::removeIncoming(NeuroItem *item)
    {
        if (!network() || !network()->neuronet())
            return false;

        NeuroNarrowItem *linkItem;

        if (NeuroItem::removeIncoming(item) && (linkItem = dynamic_cast<NeuroNarrowItem *>(item)))
        {
            if (_cellIndex != -1 && linkItem->_cellIndex != -1)
                network()->neuronet()->removeEdge(_cellIndex, linkItem->_cellIndex);
            return true;
        }

        return false;
    }

    void NeuroNarrowItem::setPenProperties(QPen & pen) const
    {
        NeuroItem::setPenProperties(pen);

        const NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            qreal t = qBound(static_cast<qreal>(0), qAbs(static_cast<qreal>(cell->current().outputValue())), static_cast<qreal>(1));

            QColor result = lerp(NORMAL_LINE_COLOR, ACTIVE_COLOR, t);

            if (cell->current().frozen())
                pen.setColor(lerp(result, Qt::gray, 0.5f));
            else
                pen.setColor(result);
        }
    }

    const NeuroNet::ASYNC_STATE *NeuroNarrowItem::getCell() const
    {
        if (!network() || !network()->neuronet())
            return 0;

        return _cellIndex != -1 ? &((*network()->neuronet())[_cellIndex]) : 0;
    }

    NeuroNet::ASYNC_STATE *NeuroNarrowItem::getCell()
    {
        if (!network() || !network()->neuronet())
            return 0;

        return _cellIndex != -1 ? &((*network()->neuronet())[_cellIndex]) : 0;
    }

    void NeuroNarrowItem::reset()
    {
        NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell && !cell->current().frozen())
        {
            cell->current().setOutputValue(0);
            cell->former().setOutputValue(0);
            update();
        }
    }

    void NeuroNarrowItem::toggleActivated()
    {
        if (!scene())
            return;

        for (QListIterator<QGraphicsItem *> i(scene()->selectedItems()); i.hasNext(); i.next())
        {
            NeuroNarrowItem *item = dynamic_cast<NeuroNarrowItem *>(i.peekNext());
            if (item)
            {
                NeuroNet::ASYNC_STATE *cell = item->getCell();

                if (cell)
                {
                    NeuroCell::NeuroValue val = 0;

                    if (qAbs(cell->current().outputValue()) < 0.1f)
                        val = 1;

                    if (cell->current().weight() < 0)
                        val *= -1;

                    cell->current().setOutputValue(val);
                    cell->former().setOutputValue(val);

                    item->update();
                }
            }
        }
    }

    void NeuroNarrowItem::toggleFrozen()
    {
        if (!scene())
            return;

        for (QListIterator<QGraphicsItem *> i(scene()->selectedItems()); i.hasNext(); i.next())
        {
            NeuroNarrowItem *item = dynamic_cast<NeuroNarrowItem *>(i.peekNext());
            if (item)
            {
                NeuroNet::ASYNC_STATE *cell = item->getCell();
                if (cell)
                {
                    bool val = !cell->current().frozen();

                    cell->current().setFrozen(val);
                    cell->former().setFrozen(val);

                    item->update();
                }
            }
        }
    }

    void NeuroNarrowItem::writeBinary(QDataStream & data) const
    {
        NeuroItem::writeBinary(data);
        data << static_cast<qint32>(_cellIndex);
    }

    void NeuroNarrowItem::readBinary(QDataStream & data)
    {
        NeuroItem::readBinary(data);

        qint32 n;
        data >> n; _cellIndex = static_cast<NeuroCell::NeuroIndex>(n);
    }

} // namespace NeuroLab
