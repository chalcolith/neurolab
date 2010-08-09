/*
Neurocognitive Linguistics Lab
Copyright (c) 2010, Gordon Tisher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

 - Neither the name of the Neurocognitive Linguistics Lab nor the
   names of its contributors may be used to endorse or promote
   products derived from this software without specific prior
   written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

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
#include <QPainter>
#include <QtAlgorithms>

#include <QtVariantProperty>

using namespace NeuroLib;

namespace NeuroLab
{

    const QColor NeuroItem::NORMAL_LINE_COLOR = Qt::black;
    const QColor NeuroItem::UNLINKED_LINE_COLOR = Qt::lightGray;
    const QColor NeuroItem::BACKGROUND_COLOR = Qt::white;
    const QColor NeuroItem::ACTIVE_COLOR = Qt::red;

    const int NeuroItem::NORMAL_LINE_WIDTH = 1;
    const int NeuroItem::HOVER_LINE_WIDTH = 3;

    const int NeuroItem::NODE_WIDTH = 30;
    const int NeuroItem::ELLIPSE_WIDTH = 10;

    //////////////////////////////////////////////

    NeuroItem::IdType NeuroItem::NEXT_ID = 1;
    QMap<QString, QString> *NeuroItem::_typeNames = 0;
    QMap<QString, QPair<QString, NeuroItem::CreateFT> > *NeuroItem::_itemCreators = 0;


    NeuroItem::NeuroItem(LabNetwork *network, const QPointF & scenePos, const CreateContext &)
        : PropertyObject(network), QGraphicsItem(),
        _label_property(this, &NeuroItem::label, &NeuroItem::setLabel, tr("Label")),
        _network(network), _id(NEXT_ID++)
    {
        setPos(scenePos);

        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

        setAcceptHoverEvents(true);

        if (_network)
        {
            connect(this, SIGNAL(labelChanged(NeuroItem*,QString)), _network, SLOT(changeItemLabel(NeuroItem*,QString)), Qt::UniqueConnection);
        }
    }

    NeuroItem::~NeuroItem()
    {
        while (_incoming.size() > 0)
            removeIncoming(_incoming.front());
        while (_outgoing.size() > 0)
            removeOutgoing(_outgoing.front());
    }

    void NeuroItem::registerTypeName(const QString & mangledName, const QString & friendlyName)
    {
        if (!_typeNames)
            _typeNames = new QMap<QString, QString>();

        if (!mangledName.isNull() && !mangledName.isEmpty() && !friendlyName.isNull() && !friendlyName.isEmpty() && !_typeNames->contains(mangledName))
        {
            (*_typeNames)[mangledName] = friendlyName;
        }
    }

    QString NeuroItem::getTypeName() const
    {
        if (!_typeNames)
            _typeNames = new QMap<QString, QString>();

        QString mangledName(typeid(*this).name());
        return getTypeName(mangledName);
    }

    QString NeuroItem::getTypeName(const QString & mangledName)
    {
        if (!_typeNames)
            _typeNames = new QMap<QString, QString>();

        // we don't want to insert empty values
        if (_typeNames->contains(mangledName))
            return (*_typeNames)[mangledName];
        else
            return QString();
    }

    void NeuroItem::registerItemCreator(const QString & typeName, const QString & description, CreateFT createFunc)
    {
        if (!_itemCreators)
            _itemCreators = new QMap<QString, QPair<QString, NeuroItem::CreateFT> >();

        if (!typeName.isNull() && !typeName.isEmpty() && createFunc)
        {
            (*_itemCreators)[typeName] = QPair<QString, CreateFT>(description, createFunc);
        }
    }

    void NeuroItem::removeItemCreator(const QString & typeName)
    {
        if (!_itemCreators)
            _itemCreators = new QMap<QString, QPair<QString, NeuroItem::CreateFT> >();

        _itemCreators->remove(typeName);
    }

    NeuroItem *NeuroItem::create(const QString & name, LabScene *scene, const QPointF & pos, const CreateContext & context)
    {
        if (!_itemCreators)
            _itemCreators = new QMap<QString, QPair<QString, NeuroItem::CreateFT> >();

        CreateFT cf = (*_itemCreators)[name].second;

        if (cf)
            return cf(scene, pos, context);

        return 0;
    }

    struct ItemTypeLessThan
    {
        const QMap<QString, QPair<QString, NeuroItem::CreateFT> > & _item_creators;

        ItemTypeLessThan(const QMap<QString, QPair<QString, NeuroItem::CreateFT> > & item_creators)
            : _item_creators(item_creators)
        {
        }

        bool operator() (const QString & a, const QString & b)
        {
            return _item_creators[a].first < _item_creators[b].first;
        }
    };

    void NeuroItem::buildNewMenu(LabScene *, NeuroItem *item, const QPointF & pos, QMenu & menu)
    {
        QAction *newAction = menu.addAction(tr("New"));
        newAction->setEnabled(false);

        if (!_itemCreators)
            _itemCreators = new QMap<QString, QPair<QString, NeuroItem::CreateFT> >();

        // sort type names
        QList<QString> keys = _itemCreators->keys();
        qSort(keys.begin(), keys.end(), ItemTypeLessThan(*_itemCreators));

        for (QListIterator<QString> i(keys); i.hasNext(); )
        {
            const QString & typeName = i.next();

            // kludge to get rid of mangled names
            const QString friendlyName = getTypeName(typeName);
            if (!friendlyName.isEmpty() && _itemCreators->contains(friendlyName))
                continue;

            // get menu text and create function
            const QPair<QString, NeuroItem::CreateFT> & p = (*_itemCreators)[typeName];
            const QStringList menuPath = p.first.split('|');

            QMenu *subMenu = &menu;
            for (int j = 0; j < menuPath.size(); ++j)
            {
                if (j == menuPath.size()-1)
                {
                    // create the leaf action
                    QAction *action = subMenu->addAction(menuPath[j]);
                    action->setData(QVariant(typeName));
                    action->setEnabled(!item || item->canCreateNewOnMe(typeName, pos));
                }
                else
                {
                    // find the submenu
                    bool found = false;
                    for (QListIterator<QAction *> k(subMenu->actions()); k.hasNext(); )
                    {
                        QAction *action = k.next();
                        if (action->text() == menuPath[j] && action->menu())
                        {
                            subMenu = action->menu();
                            found = true;
                            break;
                        }
                    }

                    // create the submenu
                    if (!found)
                        subMenu = subMenu->addMenu(menuPath[j]);
                }
            }
        }
    }

    void NeuroItem::buildActionMenu(LabScene *scene, NeuroItem *item, const QPointF & pos, QMenu & menu)
    {
        if (item)
            item->buildActionMenu(scene, pos, menu);

        if (!scene->network())
            return;

        menu.addSeparator();

        QAction *cut = menu.addAction(tr("Cut"), scene->network(), SLOT(cutSelected()), QKeySequence(QKeySequence::Cut));
        cut->setEnabled(scene->selectedItems().size() > 0);

        QAction *copy = menu.addAction(tr("Copy"), scene->network(), SLOT(copySelected()), QKeySequence(QKeySequence::Copy));
        copy->setEnabled(scene->selectedItems().size() > 0);

        QAction *paste = menu.addAction(tr("Paste"), scene->network(), SLOT(pasteItems()), QKeySequence(QKeySequence::Paste));
        paste->setEnabled(scene->network()->canPaste());

        QAction *del = menu.addAction(tr("Delete"), scene->network(), SLOT(deleteSelected()), QKeySequence(QKeySequence::Delete));
        del->setEnabled(scene->selectedItems().size() > 0);
    }

    void NeuroItem::setChanged(bool changed)
    {
        Q_ASSERT(_network != 0);
        _network->setChanged(changed);

        updateShape();
        update();
    }

    void NeuroItem::updateProperties()
    {
        PropertyObject::updateProperties();

        updateShape();
        update();
    }

    void NeuroItem::bringToFront()
    {
        qreal highest_z = this->zValue();
        for (QListIterator<QGraphicsItem *> i(this->collidingItems()); i.hasNext(); )
        {
            QGraphicsItem *item = i.next();
            if (!item)
                continue;

            if (item->zValue() > highest_z)
                highest_z = item->zValue();
        }

        if (highest_z > this->zValue())
            this->setZValue(highest_z + 0.1);
    }

    bool NeuroItem::addIncoming(NeuroItem *linkItem)
    {
        if (linkItem && !_incoming.contains(linkItem))
        {
            _incoming.append(linkItem);
            linkItem->addOutgoing(this);
            return true;
        }
        return false;
    }

    bool NeuroItem::removeIncoming(NeuroItem *linkItem)
    {
        if (linkItem && _incoming.contains(linkItem))
        {
            _incoming.removeAll(linkItem);
            linkItem->removeOutgoing(this);
            return true;
        }
        return false;
    }

    bool NeuroItem::addOutgoing(NeuroItem *linkItem)
    {
        if (linkItem && !_outgoing.contains(linkItem))
        {
            _outgoing.append(linkItem);
            linkItem->addIncoming(this);
            return true;
        }
        return false;
    }

    bool NeuroItem::removeOutgoing(NeuroItem *linkItem)
    {
        if (linkItem && _outgoing.contains(linkItem))
        {
            _outgoing.removeAll(linkItem);
            linkItem->removeIncoming(this);
            return true;
        }
        return false;
    }

    void NeuroItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
    {
        if (_network && _network->scene())
            _network->scene()->setItemUnderMouse(this);
        update();
    }

    void NeuroItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
    {
        if (_network && _network->scene())
            _network->scene()->setItemUnderMouse(0);
        update(this->boundingRect());
    }

    void NeuroItem::updateShape() const
    {
        const_cast<NeuroItem *>(this)->prepareGeometryChange();

        // re-initialize path
        _drawPath = QPainterPath();
        _drawPath.setFillRule(Qt::WindingFill);

        // call virtual function
        _texts.clear();
        addToShape(_drawPath, _texts);

        // stroke shape path
        QPen pen;
        setPenProperties(pen);

        QPainterPathStroker stroker;
        stroker.setCapStyle(Qt::SquareCap);
        stroker.setDashPattern(Qt::SolidLine);
        stroker.setJoinStyle(Qt::MiterJoin);
        stroker.setWidth(pen.width() * 4);

        _shapePath = stroker.createStroke(_drawPath);
        _shapePath.setFillRule(Qt::WindingFill);

        // add texts
        if (!_label.isNull() && !_label.isEmpty())
            _texts.append(TextPathRec(QPointF(NeuroItem::NODE_WIDTH + 4, -1), _label));

        for (QListIterator<TextPathRec> i(_texts); i.hasNext(); )
        {
            const TextPathRec & rec = i.next();
            _shapePath.addText(rec.pos, rec.font, rec.text);
        }

        _shapePath = _drawPath.united(_shapePath);
    }

    void NeuroItem::addToShape(QPainterPath &, QList<TextPathRec> &) const
    {
    }

    QRectF NeuroItem::boundingRect() const
    {
        return _shapePath.controlPointRect();
    }

    QPainterPath NeuroItem::shape() const
    {
        return _shapePath;
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

    void NeuroItem::setPenProperties(QPen & pen) const
    {
        if (shouldHighlight())
            pen.setWidth(HOVER_LINE_WIDTH);
        else
            pen.setWidth(NORMAL_LINE_WIDTH);

        pen.setColor(NORMAL_LINE_COLOR);
    }

    void NeuroItem::setBrushProperties(QBrush & brush) const
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
        painter->drawPath(_drawPath);

        QPen textPen(Qt::SolidLine);
        textPen.setColor(NORMAL_LINE_COLOR);
        textPen.setWidth(NORMAL_LINE_WIDTH);
        painter->setPen(textPen);

        for (QListIterator<TextPathRec> i(_texts); i.hasNext(); )
        {
            const TextPathRec & rec = i.next();
            painter->setFont(rec.font);
            painter->setPen(rec.pen);
            painter->drawText(rec.pos, rec.text);
        }
    }

    bool NeuroItem::canAttachTo(const QPointF &, NeuroItem *item)
    {
        bool result = !_incoming.contains(item) && !_outgoing.contains(item);
        return result;
    }

    bool NeuroItem::canBeAttachedBy(const QPointF &, NeuroItem *item)
    {
        return !_incoming.contains(item) && !_outgoing.contains(item);
    }

    QVariant NeuroItem::itemChange(GraphicsItemChange change, const QVariant & value)
    {
        LabScene *labScene;

        switch (change)
        {
        case QGraphicsItem::ItemPositionChange:
            if ((labScene = dynamic_cast<LabScene *>(scene())) && labScene->selectedItems().size() <= 1)
            {
                QPointF movePos(value.toPointF());

                if (handleMove(labScene->lastMousePos(), movePos))
                {
                    updateShape();
                    return QVariant(movePos);
                }
            }
            break;
        default:
            break;
        }

        return value;
    }

    bool NeuroItem::handleMove(const QPointF & mousePos, QPointF & movePos)
    {
        Q_ASSERT(_network != 0 && _network->scene() != 0);
        _network->setChanged();

        // get topmost item at mouse position that is not the item itself
        QRectF mouseRect(mousePos.x() - 2, mousePos.y() - 2, 4, 4);
        NeuroItem *itemAtPos = 0;

        for (QListIterator<QGraphicsItem *> i(_network->scene()->items(mouseRect, Qt::IntersectsItemShape, Qt::DescendingOrder)); i.hasNext(); )
        {
            if ((itemAtPos = dynamic_cast<NeuroItem *>(i.next())))
            {
                if (itemAtPos == this)
                {
                    itemAtPos = 0;
                    continue;
                }
                else
                {
                    break;
                }
            }
        }

        // attach, if possible
        if (itemAtPos && canAttachTo(mousePos, itemAtPos) && itemAtPos->canBeAttachedBy(mousePos, this))
        {
            attachTo(itemAtPos);
            itemAtPos->onAttachedBy(this);

            movePos = scenePos();
        }

        adjustLinks();
        return true;
    }


    // I/O

    void NeuroItem::writeClipboard(QDataStream & ds, const QMap<int, int> & id_map) const
    {
        PropertyObject::writeClipboard(ds);
        ds << _label;

        // incoming
        ds << static_cast<quint32>(_incoming.size());
        for (int i = 0; i < _incoming.size(); ++i)
            ds << static_cast<qint32>(id_map[_incoming[i]->id()]);

        // outgoing
        ds << static_cast<quint32>(_outgoing.size());
        for (int i = 0; i < _outgoing.size(); ++i)
            ds << static_cast<qint32>(id_map[_outgoing[i]->id()]);
    }

    void NeuroItem::readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> & id_map)
    {
        PropertyObject::readClipboard(ds);
        ds >> _label;

        quint32 size;
        qint32 id;

        // incoming
        ds >> size;
        for (quint32 i = 0; i < size; ++i)
        {
            ds >> id;

            if (id_map[id])
                addIncoming(id_map[i]);
        }

        // outgoing
        ds >> size;
        for (quint32 i = 0; i < size; ++i)
        {
            ds >> id;

            if (id_map[id])
                addOutgoing(id_map[i]);
        }
    }


    void NeuroItem::writeBinary(QDataStream & ds, const NeuroLabFileVersion &) const
    {
        ds << pos();
        ds << _label;
        ds << _id;
    }

    void NeuroItem::readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version)
    {
        if (file_version.neurolab_version >= NeuroLab::NEUROLAB_FILE_VERSION_1)
        {
            QPointF p;
            ds >> p;
            setPos(p);

            ds >> _label;
            ds >> _id;
        }
        else // if (file_version.neurolab_version >= NeuroLab::NEUROLAB_FILE_VERSION_OLD)
        {
            QPointF p;
            ds >> p;
            setPos(p);

            ds >> _label;

            qint64 n;
            ds >> n;
            _id = n;
        }

        if (_id >= NEXT_ID)
            NEXT_ID = _id + 1;
    }

    /// Writes ids of incoming and outgoing pointers.
    void NeuroItem::writePointerIds(QDataStream & ds, const NeuroLabFileVersion &) const
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

    void NeuroItem::readPointerIds(QDataStream & ds, const NeuroLabFileVersion & file_version)
    {
        if (file_version.neurolab_version >= NeuroLab::NEUROLAB_FILE_VERSION_1)
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
        else // if (file_version.neurolab_version >= NeuroLab::NEUROLAB_FILE_VERSION_OLD)
        {
            qint32 n;

            _incoming.clear();
            ds >> n;
            for (qint32 i = 0; i < n; ++i)
            {
                qint64 id64;
                IdType id;
                ds >> id64; id = static_cast<IdType>(id64);

                if (id)
                {
                    _incoming.append(reinterpret_cast<NeuroLinkItem *>(id));
                }
            }

            _outgoing.clear();
            ds >> n;
            for (qint32 i = 0; i < n; ++i)
            {
                qint64 id64;
                IdType id;
                ds >> id64; id = static_cast<IdType>(id64);

                if (id)
                {
                    _outgoing.append(reinterpret_cast<NeuroLinkItem *>(id));
                }
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

            for (QListIterator<QGraphicsItem *> i(items); i.hasNext(); )
            {
                NeuroItem *item = dynamic_cast<NeuroItem *>(i.next());
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

} // namespace NeuroLab
