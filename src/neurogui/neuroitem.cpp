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
#include "labexception.h"
#include "labscene.h"
#include "labnetwork.h"
#include "mainwindow.h"

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

namespace NeuroGui
{

    const QColor NeuroItem::NORMAL_LINE_COLOR = Qt::black;
    const QColor NeuroItem::UNLINKED_LINE_COLOR = Qt::lightGray;
    const QColor NeuroItem::BACKGROUND_COLOR = Qt::white;
    const QColor NeuroItem::ACTIVE_COLOR = Qt::red;

    const int NeuroItem::NORMAL_LINE_WIDTH = 0;
    const int NeuroItem::HOVER_LINE_WIDTH = 3;

    const int NeuroItem::NODE_WIDTH = 30;
    const int NeuroItem::ELLIPSE_WIDTH = 10;

    //////////////////////////////////////////////

    NeuroItem::IdType NeuroItem::NEXT_ID = 1;
    qreal NeuroItem::HIGHEST_Z_VALUE = 0;

    QMap<QString, NeuroItem::TypeNameRec> *NeuroItem::_typeNames = 0;
    QMap<QString, QPair<QString, NeuroItem::CreateFT> > *NeuroItem::_itemCreators = 0;


    NeuroItem::NeuroItem(LabNetwork *network, const QPointF & scenePos, const CreateContext &)
        : PropertyObject(network), QGraphicsItem(),
        _label_property(this, &NeuroItem::label, &NeuroItem::setLabel, tr("Label")),
        _setting_pos(false), _ui_delete(false),
        _network(network), _id(NEXT_ID++)
    {
        setPos(scenePos);

        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        setFlag(QGraphicsItem::ItemIsFocusable, false);

        setAcceptHoverEvents(true);

        if (_network)
        {
            connect(this, SIGNAL(labelChanged(NeuroItem*,QString)), _network, SLOT(changeItemLabel(NeuroItem*,QString)), Qt::UniqueConnection);
        }
    }

    NeuroItem::~NeuroItem()
    {
        if (_ui_delete)
        {
            foreach (NeuroItem *item, _connections)
            {
                item->onDetach(this);
                this->onDetach(item);
            }
        }
    }

    void NeuroItem::registerTypeName(const QString & mangledName, const QString & typeName, const QString & menuPath, const QString & uiName)
    {
        if (!_typeNames)
            _typeNames = new QMap<QString, TypeNameRec>();

        if (!mangledName.isEmpty()
            && !typeName.isEmpty()
            && !menuPath.isEmpty()
            && !uiName.isEmpty())
        {
            if (!_typeNames->contains(mangledName))
                (*_typeNames)[mangledName] = TypeNameRec(mangledName, typeName, uiName, menuPath);
            else
                qDebug() << "You cannot register a duplicate name: " << mangledName;
        }
        else
        {
            qDebug() << "You cannot register a type with a null string: " << mangledName;
        }
    }

    QString NeuroItem::uiName() const
    {
        if (!_typeNames)
            _typeNames = new QMap<QString, TypeNameRec>();

        QString mangledName(typeid(*this).name());

        if (_typeNames->contains(mangledName))
            return (*_typeNames)[mangledName].uiName;
        return QString("?NeuroItem?");
    }

    QString NeuroItem::getTypeName() const
    {
        if (!_typeNames)
            _typeNames = new QMap<QString, TypeNameRec>();

        QString mangledName(typeid(*this).name());
        return getTypeName(mangledName);
    }

    QString NeuroItem::getTypeName(const QString & mangledName)
    {
        if (!_typeNames)
            _typeNames = new QMap<QString, TypeNameRec>();

        // we don't want to insert empty values
        if (_typeNames->contains(mangledName))
            return (*_typeNames)[mangledName].typeName;
        return QString("?unknown?");
    }

    void NeuroItem::registerItemCreator(const QString & typeName, const QString & menuPath,
                                        const QString & uiName, CreateFT createFunc)
    {
        if (!_itemCreators)
            _itemCreators = new QMap<QString, QPair<QString, NeuroItem::CreateFT> >();

        if (!typeName.isNull() && !typeName.isEmpty() && createFunc)
        {
            (*_itemCreators)[typeName] = QPair<QString, CreateFT>(QString("%1|%2").arg(menuPath, uiName), createFunc);
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

        foreach (QString typeName, keys)
        {
            // kludge to get rid of mangled names
            const QString friendlyName = getTypeName(typeName);
            if (!friendlyName.isEmpty() && _itemCreators->contains(friendlyName))
                continue;

            // get menu text and create function
            const QPair<QString, NeuroItem::CreateFT> & p = (*_itemCreators)[typeName];
            const QString pathName = p.first;

            // skip internal types
            if (pathName.startsWith("__INTERNAL__"))
                continue;

#ifndef DEBUG
            // skip debug classes
            if (pathName.startsWith("Debug"))
                continue;
#endif

            // build menu
            const QStringList menuPath = pathName.split('|');

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
                    foreach (QAction *action, subMenu->actions())
                    {
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
        del->setEnabled(scene->selectedItems().size() > 0 || scene->itemUnderMouse());
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
        setZValue(HIGHEST_Z_VALUE += 0.001f);
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

        foreach (const TextPathRec & rec, _texts)
            _shapePath.addText(rec.pos, rec.font, rec.text);

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
        pen.setStyle(Qt::SolidLine);

        if (shouldHighlight())
            pen.setWidth(HOVER_LINE_WIDTH);
        else
            pen.setWidth(NORMAL_LINE_WIDTH);

        pen.setColor(NORMAL_LINE_COLOR);
    }

    void NeuroItem::setBrushProperties(QBrush & brush) const
    {
        brush.setColor(BACKGROUND_COLOR);
        brush.setStyle(Qt::NoBrush);
    }

    void NeuroItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
    {
        painter->setRenderHint(QPainter::Antialiasing);

        QPen pen;
        setPenProperties(pen);

        QBrush brush;
        setBrushProperties(brush);

        painter->setBrush(brush);
        painter->setPen(pen);
        painter->drawPath(_drawPath);

        foreach (const TextPathRec & rec, _texts)
        {
            painter->setFont(rec.font);
            painter->setPen(rec.pen);
            painter->drawText(rec.pos, rec.text);
        }
    }

    bool NeuroItem::canAttachTo(const QPointF &, NeuroItem *item)
    {
        return false;
    }

    bool NeuroItem::canBeAttachedBy(const QPointF &, NeuroItem *item)
    {
        return false;
    }

    QVariant NeuroItem::itemChange(GraphicsItemChange change, const QVariant & value)
    {
        LabScene *labScene;

        switch (change)
        {
        case QGraphicsItem::ItemPositionChange:
            if (!_setting_pos
                && (labScene = dynamic_cast<LabScene *>(scene()))
                && labScene->selectedItems().size() <= 1
                && labScene->itemUnderMouse() == this)
            {
                _setting_pos = true;

                QPointF movePos(value.toPointF());

                if (handleMove(labScene->lastMousePos(), movePos))
                {
                    updateShape();
                    _setting_pos = false;

                    return QVariant(movePos);
                }

                _setting_pos = false;
            }
            break;
        default:
            break;
        }

        return QGraphicsItem::itemChange(change, value);
    }

    static int TOUCH_LEN = 1;

    bool NeuroItem::containsScenePos(const QPointF & mousePos)
    {
        QPointF localPos = mapFromScene(mousePos);
        QRectF mouseRect(localPos.x() - TOUCH_LEN, localPos.y() - TOUCH_LEN, TOUCH_LEN*2, TOUCH_LEN*2);
        return shape().intersects(mouseRect);
    }

    bool NeuroItem::handleMove(const QPointF & mousePos, QPointF & movePos)
    {
        Q_ASSERT(_network != 0);
        Q_ASSERT(_network->scene() != 0);

        _network->setChanged();

        // get topmost item at mouse position that is not the item itself
        QRectF mouseRect(mousePos.x() - TOUCH_LEN, mousePos.y() - TOUCH_LEN, TOUCH_LEN*2, TOUCH_LEN*2);
        NeuroItem *itemAtPos = 0;

        foreach (QGraphicsItem *gi, _network->scene()->items(mouseRect, Qt::IntersectsItemShape, Qt::DescendingOrder))
        {
            if ((itemAtPos = dynamic_cast<NeuroItem *>(gi)))
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
        if (itemAtPos)
        {
            if (!_connections.contains(itemAtPos) && canAttachTo(mousePos, itemAtPos) && itemAtPos->canBeAttachedBy(mousePos, this))
            {
                this->onAttachTo(itemAtPos);
                itemAtPos->onAttachedBy(this);
                movePos = scenePos();
            }
        }

        adjustLinks();
        return true;
    }


    // I/O

    void NeuroItem::writeClipboard(QDataStream & ds, const QMap<int, int> & id_map) const
    {
        PropertyObject::writeClipboard(ds);
        ds << _label;

        // connections
        ds << static_cast<quint32>(_connections.size());
        foreach (NeuroItem *ni, _connections)
        {
            qint32 id = static_cast<qint32>(ni->id());

            if (id && id_map.contains(id))
                ds << static_cast<qint32>(id_map[id]);
            else
                ds << static_cast<qint32>(0);
        }
    }

    void NeuroItem::readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> & id_map)
    {
        PropertyObject::readClipboard(ds);
        ds >> _label;

        quint32 size;
        qint32 id;

        // connections
        ds >> size;
        for (quint32 i = 0; i < size; ++i)
        {
            ds >> id;

            if (id_map.contains(id))
                _connections.insert(id_map[id]);
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
        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_1)
        {
            QPointF p;
            ds >> p;
            setPos(p);

            ds >> _label;
            ds >> _id;
        }
        else // if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_OLD)
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
        // connections
        qint32 n = _connections.size();
        ds << n;

        foreach (NeuroItem *item, _connections)
        {
            if (item)
                ds << static_cast<IdType>(item->_id);
            else
                ds << static_cast<IdType>(0);
        }

    }

    void NeuroItem::readPointerIds(QDataStream & ds, const NeuroLabFileVersion & file_version)
    {
        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_6)
        {
            qint32 n;

            _connections.clear();
            ds >> n;
            for (qint32 i = 0; i < n; ++i)
            {
                IdType id;
                ds >> id;

                if (id)
                    _connections.insert(reinterpret_cast<NeuroItem *>(id));
            }
        }
        else if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_1)
        {
            qint32 n;

            _connections.clear();
            ds >> n;
            for (qint32 i = 0; i < n; ++i)
            {
                IdType id;
                ds >> id;

                if (id)
                {
                    _connections.insert(reinterpret_cast<NeuroItem *>(id));
                }
            }

            ds >> n;
            for (qint32 i = 0; i < n; ++i)
            {
                IdType id;
                ds >> id;

                if (id)
                {
                    _connections.insert(reinterpret_cast<NeuroItem *>(id));
                }
            }
        }
        else // if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_OLD)
        {
            qint32 n;

            _connections.clear();
            ds >> n;
            for (qint32 i = 0; i < n; ++i)
            {
                qint64 id64;
                IdType id;
                ds >> id64; id = static_cast<IdType>(id64);

                if (id)
                {
                    _connections.insert(reinterpret_cast<NeuroItem *>(id));
                }
            }

            ds >> n;
            for (qint32 i = 0; i < n; ++i)
            {
                qint64 id64;
                IdType id;
                ds >> id64; id = static_cast<IdType>(id64);

                if (id)
                {
                    _connections.insert(reinterpret_cast<NeuroItem *>(id));
                }
            }
        }
    }

    void NeuroItem::idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> & idMap)
    {
        idsToPointersAux(_connections, idMap);
    }

    void NeuroItem::idsToPointersAux(QSet<NeuroItem *> & items, const QMap<NeuroItem::IdType, NeuroItem *> & idMap)
    {
        QSet<NeuroItem *> itemsToAdd;

        foreach (NeuroItem *ni, items)
        {
            IdType wanted_id = reinterpret_cast<IdType>(ni);
            NeuroItem *wanted_item = idMap[wanted_id];

            if (wanted_item)
                itemsToAdd.insert(wanted_item);
            else
                throw LabException(tr("Dangling node ID in file: %1").arg(wanted_id));
        }

        items = itemsToAdd;
    }

} // namespace NeuroGui
