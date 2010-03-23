#include "neuronarrowitem.h"
#include "neuronodeitem.h"
#include "neurolinkitem.h"
#include "labnetwork.h"

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

    void NeuroNarrowItem::buildProperties(QtVariantPropertyManager *manager, QtProperty *topItem)
    {
        NeuroItem::buildProperties(manager, topItem);

        if (_properties.count() <= 1)
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

        NeuroCell *cell = getCell();
        if (cell)
        {
            if (frozen_property)
                frozen_property->setValue(QVariant(cell->frozen()));

            if (dynamic_cast<NeuroLinkItem *>(this))
            {
                if (weight_property)
                    weight_property->setValue(QVariant(cell->weight()));
            }
            else
            {
                if (inputs_property)
                    inputs_property->setValue(QVariant(cell->weight()));
                if (run_property)
                    run_property->setValue(QVariant(cell->run()));
            }

            if (value_property)
                value_property->setValue(QVariant(cell->currentValue()));
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
            NeuroCell *cell = getCell();

            if (vprop == frozen_property)
            {
                if (cell)
                    cell->setFrozen(value.toBool());
                changed = true;
            }
            else if (vprop == inputs_property)
            {
                if (cell)
                    cell->setWeight(value.toFloat());
                changed = true;
            }
            else if (vprop == weight_property)
            {
                if (cell)
                    cell->setWeight(value.toFloat());
                changed = true;
            }
            else if (vprop == value_property)
            {
                if (cell)
                    cell->setCurrentValue(value.toFloat());
                changed = true;
            }
            else if (vprop == run_property)
            {
                if (cell)
                    cell->setRun(value.toFloat());
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

        const NeuroCell *cell = getCell();
        if (cell)
        {
            qreal t = qBound(static_cast<qreal>(0), qAbs(static_cast<qreal>(cell->currentValue())), static_cast<qreal>(1));

            QColor result = lerp(NORMAL_LINE_COLOR, ACTIVE_COLOR, t);

            if (cell->frozen())
                pen.setColor(lerp(result, Qt::gray, 0.5f));
            else
                pen.setColor(result);
        }
    }

    const NeuroCell *NeuroNarrowItem::getCell() const
    {
        return _cellIndex != -1 ? &((*_network->neuronet())[_cellIndex]) : 0;
    }

    NeuroCell *NeuroNarrowItem::getCell()
    {
        return _cellIndex != -1 ? &((*_network->neuronet())[_cellIndex]) : 0;
    }

    void NeuroNarrowItem::reset()
    {
        NeuroCell *cell = getCell();
        if (cell && !cell->frozen())
        {
            cell->setCurrentValue(0);
            update();
        }
    }

    void NeuroNarrowItem::toggleActivated()
    {
        NeuroCell *cell = getCell();
        if (cell)
        {
            if (cell->currentValue() > 0.1f)
                cell->setCurrentValue(0);
            else
                cell->setCurrentValue(1);

            update();
        }
    }

    void NeuroNarrowItem::toggleFrozen()
    {
        NeuroCell *cell = getCell();
        if (cell)
        {
            cell->setFrozen(!cell->frozen());
            update();
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
