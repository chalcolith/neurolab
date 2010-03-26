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

    NeuroNarrowItem::NeuroNarrowItem(LabNetwork *network, const NeuroLib::NeuroCell::NeuroIndex & cellIndex)
        : NeuroItem(network), _cellIndex(cellIndex),
        frozen_property(0), inputs_property(0), weight_property(0), run_property(0), value_property(0)
    {
    }

    NeuroNarrowItem::~NeuroNarrowItem()
    {
    }

    void NeuroNarrowItem::buildActionMenuAux(LabScene *, const QPointF &, QMenu & menu)
    {
        menu.addAction(tr("Activate/Deactivate"), this, SLOT(toggleActivated()));
        menu.addAction(tr("Freeze/Unfreeze"), this, SLOT(toggleFrozen()));
    }

    void NeuroNarrowItem::buildProperties(QtVariantPropertyManager *manager, QtProperty *topItem)
    {
        NeuroItem::buildProperties(manager, topItem);

        if (!frozen_property)
        {
            manager->connect(manager, SIGNAL(valueChanged(QtProperty*,QVariant)), this, SLOT(propertyValueChanged(QtProperty*,QVariant)));

            frozen_property = manager->addProperty(QVariant::Bool, tr("Frozen"));
            inputs_property = manager->addProperty(QVariant::Double, tr("Inputs"));
            weight_property = manager->addProperty(QVariant::Double, tr("Weight"));
            run_property = manager->addProperty(QVariant::Double, tr("Slope Width"));
            value_property = manager->addProperty(QVariant::Double, tr("Value"));

            _properties.append(frozen_property);
            _properties.append(inputs_property);
            _properties.append(weight_property);
            _properties.append(run_property);
            _properties.append(value_property);

            updateProperties();
        }

        topItem->addSubProperty(frozen_property);

        if (dynamic_cast<NeuroLinkItem *>(this))
        {
            topItem->addSubProperty(weight_property);
        }
        else
        {
            topItem->addSubProperty(inputs_property);
            topItem->addSubProperty(run_property);
        }

        topItem->addSubProperty(value_property);
    }

    void NeuroNarrowItem::updateProperties()
    {
        NeuroItem::updateProperties();

        _updating = true;

        NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            if (frozen_property)
                frozen_property->setValue(QVariant(cell->current().frozen()));

            if (dynamic_cast<NeuroLinkItem *>(this))
            {
                if (weight_property)
                    weight_property->setValue(QVariant(cell->current().weight()));
            }
            else
            {
                if (inputs_property)
                    inputs_property->setValue(QVariant(cell->current().weight()));
                if (run_property)
                    run_property->setValue(QVariant(cell->current().run()));
            }

            if (value_property)
                value_property->setValue(QVariant(cell->current().outputValue()));
        }

        _updating = false;
    }

    void NeuroNarrowItem::propertyValueChanged(QtProperty *property, const QVariant & value)
    {
        if (_updating)
            return;

        NeuroItem::propertyValueChanged(property, value);

        QtVariantProperty *vprop = dynamic_cast<QtVariantProperty *>(property);

        if (vprop)
        {
            bool changed = false;
            NeuroNet::ASYNC_STATE *cell = getCell();

            if (vprop == frozen_property)
            {
                if (cell)
                {
                    cell->current().setFrozen(value.toBool());
                    cell->former().setFrozen(value.toBool());
                }
                changed = true;
            }
            else if (vprop == inputs_property)
            {
                if (cell)
                {
                    cell->current().setWeight(value.toFloat());
                    cell->former().setWeight(value.toFloat());
                }
                changed = true;
            }
            else if (vprop == weight_property)
            {
                if (cell)
                {
                    cell->current().setWeight(value.toFloat());
                    cell->former().setWeight(value.toFloat());
                }
                changed = true;
            }
            else if (vprop == value_property)
            {
                if (cell)
                {
                    cell->current().setOutputValue(value.toFloat());
                    cell->former().setOutputValue(value.toFloat());
                }
                changed = true;
            }
            else if (vprop == run_property)
            {
                if (cell)
                {
                    cell->current().setRun(value.toFloat());
                    cell->former().setRun(value.toFloat());
                }
                changed = true;
            }

            if (changed)
            {
                updateShape();
                update();
            }
        }
    }

    bool NeuroNarrowItem::addIncoming(NeuroItem *item)
    {
        NeuroNarrowItem *linkItem;

        if (NeuroItem::addIncoming(item) && (linkItem = dynamic_cast<NeuroNarrowItem *>(item)))
        {
            if (_cellIndex != -1 && linkItem->_cellIndex != -1)
                _network->neuronet()->addEdge(_cellIndex, linkItem->_cellIndex);
            return true;
        }
        return false;
    }

    bool NeuroNarrowItem::removeIncoming(NeuroItem *item)
    {
        NeuroNarrowItem *linkItem;

        if (NeuroItem::removeIncoming(item) && (linkItem = dynamic_cast<NeuroNarrowItem *>(item)))
        {
            if (_cellIndex != -1 && linkItem->_cellIndex != -1)
                _network->neuronet()->removeEdge(_cellIndex, linkItem->_cellIndex);
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
        return _cellIndex != -1 ? &((*_network->neuronet())[_cellIndex]) : 0;
    }

    NeuroNet::ASYNC_STATE *NeuroNarrowItem::getCell()
    {
        return _cellIndex != -1 ? &((*_network->neuronet())[_cellIndex]) : 0;
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
