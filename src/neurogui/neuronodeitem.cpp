#include "neuronodeitem.h"
#include "neurolinkitem.h"
#include "labnetwork.h"
#include "labscene.h"

#include <QVector2D>
#include <QApplication>
#include <QtVariantProperty>

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


    //////////////////////////////////////////////////////////////////

    NEUROITEM_DEFINE_CREATOR(NeuroOscillatorItem, QObject::tr("Narrow|Oscillator"));

    NeuroOscillatorItem::NeuroOscillatorItem(LabNetwork *network, const QPointF & scenePos)
        : NeuroNodeItem(network, scenePos),
        _phase_property(0), _peak_property(0), _gap_property(0), _value_property(0)
    {
        Q_ASSERT(network != 0 && network->neuronet() != 0);

        NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            cell->current().setWeight(0);
            cell->former().setWeight(0);
            cell->current().setRun(0);
            cell->former().setRun(0);
        }
    }

    NeuroOscillatorItem::~NeuroOscillatorItem()
    {
    }

    void NeuroOscillatorItem::buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem)
    {
        NeuroNodeItem::buildProperties(manager, parentItem); // NOT calling the narrow version!

        if (!_phase_property)
        {
            manager->connect(manager, SIGNAL(valueChanged(QtProperty*,QVariant)), this, SLOT(propertyValueChanged(QtProperty*,QVariant)));

            _phase_property = manager->addProperty(QVariant::Int, tr("Phase"));
            _peak_property = manager->addProperty(QVariant::Int, tr("Spike"));
            _gap_property = manager->addProperty(QVariant::Int, tr("Gap"));
            _value_property = manager->addProperty(QVariant::Double, tr("Value"));

            updateProperties();
        }

        parentItem->setPropertyName(tr("Oscillator"));
        parentItem->addSubProperty(_phase_property);
        parentItem->addSubProperty(_peak_property);
        parentItem->addSubProperty(_gap_property);
        parentItem->addSubProperty(_value_property);
    }

    void NeuroOscillatorItem::updateProperties()
    {
        NeuroItem::updateProperties();

        _updating = true;

        NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            if (_phase_property)
                _phase_property->setValue(QVariant(cell->current().phase()));
            if (_peak_property)
                _peak_property->setValue(QVariant(cell->current().peak()));
            if (_gap_property)
                _gap_property->setValue(QVariant(cell->current().gap()));
            if (_value_property)
                _value_property->setValue(QVariant(cell->current().outputValue()));
        }

        _updating = false;
    }

    void NeuroOscillatorItem::propertyValueChanged(QtProperty *property, const QVariant & value)
    {
        if (_updating)
            return;

        NeuroItem::propertyValueChanged(property, value);

        QtVariantProperty *vprop = dynamic_cast<QtVariantProperty *>(property);
        NeuroNet::ASYNC_STATE *cell = getCell();

        if (vprop && cell)
        {
            bool changed = false;

            if (vprop == _phase_property)
            {
                cell->current().setPhase(value.toInt());
                cell->former().setPhase(value.toInt());
                changed = true;
            }
            else if (vprop == _peak_property)
            {
                cell->current().setPeak(value.toInt());
                cell->former().setPeak(value.toInt());
                changed = true;
            }
            else if (vprop == _gap_property)
            {
                cell->current().setGap(value.toInt());
                cell->former().setGap(value.toInt());
                changed = true;
            }
            else if (vprop == _value_property)
            {
                cell->current().setOutputValue(value.toFloat());
                cell->former().setOutputValue(value.toFloat());
                changed = true;
            }

            if (changed)
            {
                updateShape();
                update();
            }
        }
    }

    void NeuroOscillatorItem::reset()
    {
        NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            cell->current().setStep(0);
            cell->former().setStep(0);
            cell->current().setOutputValue(cell->current().phase() == 0 ? 1 : 0);
            cell->former().setOutputValue(cell->current().phase() == 0 ? 1 : 0);
        }
    }

    void NeuroOscillatorItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroNarrowItem::addToShape(drawPath, texts);
        drawPath.addEllipse(rect());

        const NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            texts.append(TextPathRec(QPointF(-4, 4), QString("%1/%2").arg(cell->current().peak()).arg(cell->current().gap())));
        }
    }

} // namespace NeuroLab
