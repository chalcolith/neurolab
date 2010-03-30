#include "neuronarrowitem.h"
#include "neuronodeitem.h"
#include "neurolinkitem.h"
#include "labnetwork.h"

#include <QPen>
#include <QGraphicsScene>

#include <QtVariantProperty>

using namespace NeuroLib;

namespace NeuroLab
{

    NeuroNarrowItem::NeuroNarrowItem(LabNetwork *network, const QPointF & scenePos)
        : NeuroItem(network, scenePos),
        _value_property(this, &NeuroNarrowItem::outputValue, &NeuroNarrowItem::setOutputValue, tr("Output Value")),
        _cellIndex(-1)
    {
    }

    NeuroNarrowItem::~NeuroNarrowItem()
    {
    }

    NeuroCell::NeuroValue NeuroNarrowItem::outputValue() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell();
        return cell ? cell->current().outputValue() : 0.0f;
    }

    void NeuroNarrowItem::setOutputValue(const NeuroCell::NeuroValue & value)
    {
        NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            cell->current().setOutputValue(value);
            cell->former().setOutputValue(value);
        }
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
        setOutputValue(0);
        updateProperties();
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
