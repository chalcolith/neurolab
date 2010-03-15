#include "neuroitem.h"
#include "mainwindow.h"
#include "neurolinkitem.h"
#include "labscene.h"
#include "labnetwork.h"

#include "../automata/exception.h"
#include "../neurolib/neuronet.h"
#include "../neurolib/neurocell.h"

#include <QApplication>
#include <QStatusBar>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>

#include <QtVariantProperty>

using namespace NeuroLib;

namespace NeuroLab
{
    
    const QColor NeuroItem::NORMAL_LINE_COLOR = Qt::black;
    const QColor NeuroItem::UNLINKED_LINE_COLOR = Qt::lightGray;
    const QColor NeuroItem::BACKGROUND_COLOR = Qt::white;
    const QColor NeuroItem::ACTIVE_COLOR = Qt::red;
    
    const int NeuroItem::NORMAL_LINE_WIDTH = 1;
    const int NeuroItem::HOVER_LINE_WIDTH = 5;

    const int NeuroItem::NODE_WIDTH = 30;
    const int NeuroItem::ELLIPSE_WIDTH = 10;
    
    //////////////////////////////////////////////    
    
    int NeuroItem::NEXT_ID = 1;
    QMap<QString, NeuroItem::CreateFT> NeuroItem::itemCreators;
    
    NeuroItem::NeuroItem(LabNetwork *network, const NeuroCell::NeuroIndex & cellIndex)
        : QObject(network), QGraphicsItem(), PropertyObject(),
        label_property(0), frozen_property(0), 
        inputs_property(0), weight_property(0), 
        run_property(0), value_property(0),
        _network(network), _id(NEXT_ID++), 
        _path(0), _textPath(0), _cellIndex(cellIndex)
    {
        this->setFlag(QGraphicsItem::ItemIsSelectable, true);
        this->setFlag(QGraphicsItem::ItemIsMovable, true);
        this->setAcceptHoverEvents(true);
    }
    
    NeuroItem::~NeuroItem()
    {
        delete _path;
        delete _textPath;
        
        while (_incoming.size() > 0)
            removeIncoming(_incoming.front());
        while (_outgoing.size() > 0)
            removeOutgoing(_outgoing.front());
    }
    
    void NeuroItem::buildProperties(QtVariantPropertyManager *manager, QtProperty *topItem)
    {
        if (_properties.count() == 0)
        {
            manager->connect(manager, SIGNAL(valueChanged(QtProperty*,QVariant)), this, SLOT(propertyValueChanged(QtProperty*,QVariant)));
            
            label_property = manager->addProperty(QVariant::String, tr("Label"));            
            frozen_property = manager->addProperty(QVariant::Bool, tr("Frozen"));
            inputs_property = manager->addProperty(QVariant::Double, tr("Inputs"));
            weight_property = manager->addProperty(QVariant::Double, tr("Weight"));
            run_property = manager->addProperty(QVariant::Double, tr("Slope Width"));
            value_property = manager->addProperty(QVariant::Double, tr("Value"));

            _properties.append(label_property);
            _properties.append(frozen_property);
            _properties.append(inputs_property);
            _properties.append(weight_property);
            _properties.append(run_property);
            _properties.append(value_property);
            
            updateProperties();
        }

        topItem->addSubProperty(label_property);
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
    
    void NeuroItem::updateProperties()
    {
        _updating = true;
        
        if (label_property)
            label_property->setValue(QVariant(_label));
        
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
    
    void NeuroItem::propertyValueChanged(QtProperty *property, const QVariant & value)
    {
        if (_updating)
            return;
        
        QtVariantProperty *vprop = dynamic_cast<QtVariantProperty *>(property);
        
        if (vprop)
        {
            bool changed = false;
            NeuroCell *cell = getCell();
            
            if (vprop == label_property)
            {
                _label = value.toString();
                changed = true;
            }
            else if (vprop == frozen_property)
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
            }
            
            if (changed)
            {
                buildShape();
                update();
            }
        }
    }
    
    NeuroItem *NeuroItem::create(const QString & name, LabScene *scene, const QPointF & pos)
    {
        CreateFT cf = itemCreators[name];
        
        if (cf)
            return cf(scene, pos);        
        return 0;
    }
    
    void NeuroItem::bringToFront()
    {
        qreal highest_z = this->zValue();
        for (QListIterator<QGraphicsItem *> i(this->collidingItems()); i.hasNext(); i.next())
        {
            QGraphicsItem *item = i.peekNext();
            if (!item)
                continue;
            
            if (item->zValue() > highest_z)
                highest_z = item->zValue();
        }
        
        if (highest_z > this->zValue())
            this->setZValue(highest_z + 0.1);
    }
    
    void NeuroItem::addIncoming(NeuroItem *linkItem)
    {
        if (linkItem && !_incoming.contains(linkItem))
        {
            _incoming.append(linkItem);
            
            if (_cellIndex != -1 && linkItem->_cellIndex != -1)
                _network->neuronet()->addEdge(_cellIndex, linkItem->_cellIndex);            
            
            linkItem->addOutgoing(this);
        }
    }
    
    void NeuroItem::removeIncoming(NeuroItem *linkItem)
    {
        if (linkItem && _incoming.contains(linkItem))
        {
            _incoming.removeAll(linkItem);
            
            if (_cellIndex != -1 && linkItem->_cellIndex != -1)
                _network->neuronet()->removeEdge(_cellIndex, linkItem->_cellIndex);

            linkItem->removeOutgoing(this);
        }
    }
    
    void NeuroItem::addOutgoing(NeuroItem *linkItem)
    {
        if (linkItem && !_outgoing.contains(linkItem))
        {
            _outgoing.append(linkItem);
            
            linkItem->addIncoming(this);
        }
    }
    
    void NeuroItem::removeOutgoing(NeuroItem *linkItem)
    {
        if (linkItem && _outgoing.contains(linkItem))
        {
            _outgoing.removeAll(linkItem);
            
            linkItem->removeIncoming(this);
        }
    }
    
    void NeuroItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
    {
        if (_network && _network->scene())
        {
            _network->scene()->setItemUnderMouse(this);
        }
        
        update(this->boundingRect());
    }
    
    void NeuroItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
    {
        if (_network && _network->scene())
            _network->scene()->setItemUnderMouse(0);
        
        update(this->boundingRect());
    }
    
    void NeuroItem::buildShape()
    {
        updateProperties();
        
        delete _path;
        _path = new QPainterPath();
        _path->setFillRule(Qt::WindingFill);
        
        delete _textPath;
        _textPath = 0;
        _texts.clear();
        
        if (!_label.isNull() && !_label.isEmpty())            
            _texts.append(TextPathRec(QPointF(NeuroItem::NODE_WIDTH + 4, -1), _label));
        
        const_cast<NeuroItem *>(this)->prepareGeometryChange();
    }
   
    void NeuroItem::buildTextPath() const
    {
        if (!_textPath)
        {
            _textPath = new QPainterPath();
            _textPath->setFillRule(Qt::WindingFill);

            for (QListIterator<TextPathRec> i(_texts); i.hasNext(); i.next())
            {
                const TextPathRec & rec = i.peekNext();                
                _textPath->addText(rec.pos, QApplication::font(), rec.text);
            }
        }
    }

    QRectF NeuroItem::boundingRect() const
    {
        buildTextPath();
        return _path->united(*_textPath).controlPointRect();
    }

    QPainterPath NeuroItem::shape() const
    {
        buildTextPath();
        return _path->united(*_textPath);
    }

    bool NeuroItem::shouldHighlight() const
    {
        if (isSelected())
            return true;

        const LabScene *sc = dynamic_cast<const LabScene *>(this->scene());
        if (sc && (sc->itemUnderMouse() == this || sc->selectedItems().contains(const_cast<NeuroItem *>(this))))
            return true;

        return false;
    }

    QColor NeuroItem::lerp(const QColor & a, const QColor & b, const qreal & t)
    {
        qreal ar, ag, ab, aa;
        qreal br, bg, bb, ba;

        a.getRgbF(&ar, &ag, &ab, &aa);
        b.getRgbF(&br, &bg, &bb, &ba);

        qreal rr = ar + (br - ar)*t;
        qreal rg = ag + (bg - ag)*t;
        qreal rb = ab + (bb - ab)*t;
        qreal ra = aa + (ba - aa)*t;

        QColor result;
        result.setRgbF(rr, rg, rb, ra);
        return result;
    }
    
    void NeuroItem::setPenProperties(QPen & pen)
    {
        if (shouldHighlight())
            pen.setWidth(HOVER_LINE_WIDTH);
        else
            pen.setWidth(NORMAL_LINE_WIDTH);
        
        //
        NeuroCell *cell = getCell();
        if (cell)
        {
            qreal t = qBound(static_cast<qreal>(0), qAbs(static_cast<qreal>(cell->currentValue())), static_cast<qreal>(1));

            QColor result = lerp(NORMAL_LINE_COLOR, ACTIVE_COLOR, t);

            if (cell->frozen())
                pen.setColor(lerp(result, Qt::gray, 0.5f));
            else
                pen.setColor(result);
        }
        else
        {
            pen.setColor(NORMAL_LINE_COLOR);
        }
    }
    
    void NeuroItem::setBrushProperties(QBrush & brush)
    {
        brush.setColor(BACKGROUND_COLOR);
    }

    void NeuroItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
    {
        painter->setRenderHint(QPainter::Antialiasing);

        QPen pen(Qt::SolidLine);
        setPenProperties(pen);

        QBrush brush(Qt::SolidPattern);
        setBrushProperties(brush);

        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawPath(*_path);
        
        QPen textPen(Qt::SolidLine);
        textPen.setColor(NORMAL_LINE_COLOR);
        textPen.setWidth(NORMAL_LINE_WIDTH);
        painter->setPen(textPen);
        
        for (QListIterator<TextPathRec> i(_texts); i.hasNext(); i.next())
        {
            const TextPathRec & rec = i.peekNext();
            painter->drawText(rec.pos, rec.text);
        }
    }


    // I/O

    /// Writes ids of incoming and outgoing pointers.
    void NeuroItem::writePointerIds(QDataStream & ds) const
    {
        qint32 n = _incoming.size();
        ds << n;

        for (qint32 i = 0; i < n; ++i)
        {
            if (_incoming[i])
                ds << static_cast<IdType>(_incoming[i]->_id);
            else
                ds << static_cast<IdType>(0);
        }

        n = _outgoing.size();
        ds << n;

        for (qint32 i = 0; i < n; ++i)
        {
            if (_outgoing[i])
                ds << static_cast<IdType>(_outgoing[i]->_id);
            else
                ds << static_cast<IdType>(0);
        }
    }

    void NeuroItem::readPointerIds(QDataStream & ds)
    {
        qint32 n;

        _incoming.clear();
        ds >> n;
        for (qint32 i = 0; i < n; ++i)
        {
            IdType id;
            ds >> id;

            if (id)
            {
                _incoming.append(reinterpret_cast<NeuroLinkItem *>(id));
            }
        }

        _outgoing.clear();
        ds >> n;
        for (qint32 i = 0; i < n; ++i)
        {
            IdType id;
            ds >> id;

            if (id)
            {
                _outgoing.append(reinterpret_cast<NeuroLinkItem *>(id));
            }
        }
    }

    void NeuroItem::idsToPointers(QGraphicsScene *sc)
    {
        idsToPointersAux(_incoming, sc);
        idsToPointersAux(_outgoing, sc);
    }

    void NeuroItem::idsToPointersAux(QList<NeuroItem *> & list, QGraphicsScene *sc)
    {
        QList<QGraphicsItem *> items = sc->items();

        for (QMutableListIterator<NeuroItem *> in(list); in.hasNext(); in.next())
        {
            bool found = false;
            IdType id = reinterpret_cast<IdType>(in.peekNext());
            for (QListIterator<QGraphicsItem *> i(items); i.hasNext(); i.next())
            {
                NeuroItem *item = dynamic_cast<NeuroItem *>(i.peekNext());
                if (item && item->_id == id)
                {
                    in.peekNext() = item;
                    found = true;
                    break;
                }
            }

            if (!found)
                throw Automata::Exception(QObject::tr("Dangling node ID %1").arg(id));
        }
    }

    QDataStream & operator<< (QDataStream & data, const NeuroItem & item)
    {
        data << item._label;
        data << static_cast<qint64>(item._cellIndex);

        item.writeBinary(data);
        item.writePointerIds(data);

        return data;
    }

    QDataStream & operator>> (QDataStream & data, NeuroItem & item)
    {
        qint64 n;

        data >> item._label;
        data >> n; item._cellIndex = static_cast<NeuroCell::NeuroIndex>(n);

        item.readBinary(data);
        item.readPointerIds(data);

        return data;
    }

    // neuro stuff
    
    NeuroCell *NeuroItem::getCell()
    {
        return _cellIndex != -1 ? &((*_network->neuronet())[_cellIndex]) : 0;
    }

    void NeuroItem::reset()
    {
        NeuroCell *cell = getCell();
        if (cell && !cell->frozen())
            cell->setCurrentValue(0);
    }
    
    void NeuroItem::toggleActivated()
    {
        NeuroCell *cell = getCell();
        if (cell)
        {
            if (cell->currentValue() > 0.1f)
                cell->setCurrentValue(0);
            else
                cell->setCurrentValue(1);
        }
    }

    void NeuroItem::toggleFrozen()
    {
        NeuroCell *cell = getCell();
        if (cell)
        {
            cell->setFrozen(!cell->frozen());
            update(boundingRect());
        }
    }

} // namespace NeuroLab
