/*
Neurocognitive Linguistics Lab
Copyright (c) 2010,2011 Gordon Tisher
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

#include "multigridioitem.h"
#include "neurogriditem.h"
#include "../neurogui/labscene.h"
#include "../neurogui/labview.h"
#include "../neurogui/labtree.h"
#include "../neurogui/labnetwork.h"
#include "../neurolib/neuronet.h"

#include <QTextCodec>

using namespace NeuroGui;

namespace GridItems
{

    MultiGridIOItem::MultiGridIOItem(NeuroGui::LabNetwork *network, const QPointF &scenePos, const CreateContext &context)
        : MultiItem(network, scenePos, context),
          _top_item(0), _bottom_item(0),
          _width_property(this, &MultiGridIOItem::width, &MultiGridIOItem::setWidth, tr("Width"))
    {
        if (context == NeuroItem::CREATE_UI)
        {
            const int width = NODE_WIDTH * 4;
            const int height = NODE_WIDTH;

            setRect(QRectF(-width/2, -height/2, width, height));
            setLabelPos(QPointF(rect().left() + rect().width() + 5, 0));
        }
    }

    MultiGridIOItem::~MultiGridIOItem()
    {
    }

    void MultiGridIOItem::setWidth(const int & w)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        int new_width = w;
        bool update_property = false;

        if (new_width < 1)
        {
            new_width = 1;
            update_property = true;
        }

        if (new_width != width())
        {
            foreach (NeuroItem *item, connections())
                removeEdges(item);

            while (new_width > width())
            {
                NeuroLib::NeuroCell cell(NeuroLib::NeuroCell::NODE);
                _incoming_cells.append(network()->neuronet()->addNode(cell));
                _outgoing_cells.append(network()->neuronet()->addNode(cell));
            }

            while (new_width < width())
            {
                network()->neuronet()->removeNode(_incoming_cells.last());
                _incoming_cells.removeLast();
                network()->neuronet()->removeNode(_outgoing_cells.last());
                _outgoing_cells.removeLast();
            }

            foreach (NeuroItem *item, connections())
                addEdges(item);
        }

        if (update_property)
            _width_property.setValueInPropertyBrowser(QVariant(new_width));
    }

    bool MultiGridIOItem::handleMove(const QPointF &mousePos, QPointF &movePos)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->scene());

        // we want to connect with the whole bounding rect, not the mouse pos
        QPointF topLeft = mapToScene(rect().left(), rect().top());
        QRectF myRect(topLeft.x(), topLeft.y(), rect().width(), rect().height());

        bool attached = false;
        foreach (QGraphicsItem *item, network()->scene()->items(myRect, Qt::IntersectsItemShape, Qt::DescendingOrder))
        {
            NeuroItem *ni = dynamic_cast<NeuroItem *>(item);
            if (ni && ni != this
                && !connections().contains(ni)
                && canAttachTo(mousePos, ni) && ni->canBeAttachedBy(mousePos, this))
            {
                this->onAttachTo(ni);
                ni->onAttachedBy(this);
                movePos = scenePos();
                attached = true;
                break;
            }
        }

        // break if no intersect
        if (!attached)
        {
            if (_top_item && !_top_item->containsScenePos(mousePos))
            {
                _top_item->onDetach(this);
                onDetach(_top_item);
            }
            else if (_bottom_item && !_bottom_item->containsScenePos(mousePos))
            {
                _bottom_item->onDetach(this);
                onDetach(_bottom_item);
            }
        }

        // if we're still connected to a grid item, tell it to move us to the right position
        NeuroGridItem *gi = _top_item ? dynamic_cast<NeuroGridItem *>(_top_item) : dynamic_cast<NeuroGridItem *>(_bottom_item);
        if (gi)
        {
            gi->adjustLinks();
            movePos = scenePos();
        }

        return true;
    }

    bool MultiGridIOItem::canAttachTo(const QPointF &, NeuroItem *item) const
    {
        NeuroGridItem *gi = dynamic_cast<NeuroGridItem *>(item);
        return gi && !_top_item && !_bottom_item;
    }

    void MultiGridIOItem::onAttachTo(NeuroItem *item)
    {
        _top_item = 0;
        _bottom_item = 0;

        NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
        if (ni)
        {
            if (ni->scenePos().y() < scenePos().y())
                _top_item = ni;
            else
                _bottom_item = ni;
        }
    }

    void MultiGridIOItem::onDetach(NeuroItem *)
    {
        _top_item = 0;
        _bottom_item = 0;
        updateShape();
    }

    QList<MultiGridIOItem::Index> MultiGridIOItem::getIncomingCellsFor(const NeuroItem *item) const
    {
        const NeuroNetworkItem *ni = dynamic_cast<const NeuroNetworkItem *>(item);
        if (ni && (ni == _top_item || ni == _bottom_item))
            return _incoming_cells;
        else
            return QList<Index>();
    }

    QList<MultiGridIOItem::Index> MultiGridIOItem::getOutgoingCellsFor(const NeuroItem *item) const
    {
        const NeuroNetworkItem *ni = dynamic_cast<const NeuroNetworkItem *>(item);
        if (ni && (ni == _top_item || ni == _bottom_item))
            return _outgoing_cells;
        else
            return QList<Index>();
    }

    QList<MultiGridIOItem::Index> MultiGridIOItem::allCells() const
    {
        return _incoming_cells + _outgoing_cells;
    }

    void MultiGridIOItem::addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const
    {
        MultiItem::addToShape(drawPath, texts);
        drawPath.addRect(_rect);

        if (_top_item || _bottom_item)
        {
            qreal y = _top_item ? _rect.top() : _rect.bottom();

            const int num = 7;
            for (int i = 0; i < num; ++i)
            {
                qreal x = _rect.left() + (i+1)*(static_cast<qreal>(_rect.width()) / (num+1));
                drawPath.moveTo(x, y-3);
                drawPath.lineTo(x, y+3);
            }
        }
    }

    void MultiGridIOItem::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        MultiItem::writeBinary(ds, file_version);
        ds << _rect;

        quint32 num = _incoming_cells.size();
        ds << num;
        foreach (const Index & idx, _incoming_cells)
            ds << idx;

        num = _outgoing_cells.size();
        ds << num;
        foreach (const Index & idx, _outgoing_cells)
            ds << idx;
    }

    void MultiGridIOItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        MultiItem::readBinary(ds, file_version);
        ds >> _rect;
        setLabelPos(QPointF(rect().left() + rect().width() + 5, 0));

        quint32 num;
        _incoming_cells.clear();
        ds >> num;
        for (quint32 i = 0; i < num; ++i)
        {
            Index idx;
            ds >> idx;
            _incoming_cells.append(idx);
        }

        _outgoing_cells.clear();
        ds >> num;
        for (quint32 i = 0; i < num; ++i)
        {
            Index idx;
            ds >> idx;
            _outgoing_cells.append(idx);
        }
    }

    void MultiGridIOItem::writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        MultiItem::writePointerIds(ds, file_version);

        IdType id = _top_item ? _top_item->id() : 0;
        ds << id;

        id = _bottom_item ? _bottom_item->id() : 0;
        ds << id;
    }

    void MultiGridIOItem::readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        MultiItem::readPointerIds(ds, file_version);

        IdType id;
        ds >> id;
        _top_item = reinterpret_cast<NeuroNetworkItem *>(id);

        ds >> id;
        _bottom_item = reinterpret_cast<NeuroNetworkItem *>(id);
    }

    void MultiGridIOItem::idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap)
    {
        IdType wanted_id = static_cast<NeuroItem::IdType>(reinterpret_cast<quint64>(_top_item));
        if (wanted_id != 0)
        {
            NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(idMap[wanted_id]);
            if (ni)
                _top_item = ni;
        }

        wanted_id = static_cast<NeuroItem::IdType>(reinterpret_cast<quint64>(_bottom_item));
        if (wanted_id != 0)
        {
            NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(idMap[wanted_id]);
            if (ni)
                _bottom_item = ni;
        }
    }

} // namespace GridItems
