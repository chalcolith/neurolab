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

#include "compactlinkitem.h"
#include "../narrow/neuronodeitem.h"
#include "../labnetwork.h"
#include "../labscene.h"

using namespace NeuroLib;

namespace NeuroGui
{

    NEUROITEM_DEFINE_CREATOR(CompactLinkItem, QObject::tr("Abstract|Link"));

    CompactLinkItem::CompactLinkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : CompactItem(network, scenePos, context), MixinArrow(this),
        _length_property(this, &CompactLinkItem::length, &CompactLinkItem::setLength, tr("Length")),
        _weight_property(this, &CompactLinkItem::weight, &CompactLinkItem::setWeight, tr("Weight"))
    {
        Q_ASSERT(network != 0);
        Q_ASSERT(network->neuronet() != 0);

        _upward_output_property.setName(tr("Output Value A"));
        _downward_output_property.setName(tr("Output Value B"));

        if (context == CREATE_UI)
        {
            _upward_cells.clear();
            addNewCell(true);

            _downward_cells.clear();
            addNewCell(false);
        }

        setLine(scenePos.x(), scenePos.y(), scenePos.x() + NeuroItem::NODE_WIDTH * 2, scenePos.y() - NeuroItem::NODE_WIDTH * 2);
    }

    CompactLinkItem::~CompactLinkItem()
    {
    }

    void CompactLinkItem::addNewCell(bool upwards)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        NeuroCell::NeuroIndex index = network()->neuronet()->addNode(NeuroCell(NeuroCell::EXCITORY_LINK));
        if (upwards)
            _upward_cells.append(index);
        else
            _downward_cells.append(index);
    }

    void CompactLinkItem::setLength(const int &value)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        NeuroNet *neuronet = network()->neuronet();

        bool updateValue = false;
        int newLength = value;
        if (newLength < 1)
        {
            newLength = 1;
            updateValue = true;
        }

        if (newLength != length())
        {
            QList<NeuroCell::NeuroIndex> cellsToDelete;
            NeuroCell::NeuroIndex oldUpwardLast = _upward_cells.last();
            NeuroCell::NeuroIndex oldDownwardLast = _downward_cells.last();

            if (newLength > length())
            {
                NeuroCell::NeuroIndex oldUp, newUp, oldDown, newDown;

                while (length() < newLength)
                {
                    oldUp = _upward_cells.last();
                    addNewCell(true);
                    newUp = _upward_cells.last();
                    neuronet->addEdge(newUp, oldUp);

                    oldDown = _downward_cells.last();
                    addNewCell(false);
                    newDown = _downward_cells.last();
                    neuronet->addEdge(newDown, oldDown);
                }
            }
            else
            {
                while (length() > newLength)
                {
                    NeuroCell::NeuroIndex index = _upward_cells.last();
                    cellsToDelete.append(index);
                    _upward_cells.removeLast();

                    index = _downward_cells.last();
                    cellsToDelete.append(index);
                    _downward_cells.removeLast();
                }
            }

            // adjust the neighbors of our targets to reflect the new last cell
            NeuroCell::NeuroIndex newUpwardLast = _upward_cells.last();
            NeuroCell::NeuroIndex newDownwardLast = _downward_cells.last();

            CompactItem *compactFront = dynamic_cast<CompactItem *>(_frontLinkTarget);
            if (compactFront && newUpwardLast != oldUpwardLast)
            {
                NeuroCell::NeuroIndex target = compactFront->upwardCells().first();
                if (neuronet->containsEdge(target, oldUpwardLast))
                {
                    neuronet->removeEdge(target, oldUpwardLast);
                    neuronet->addEdge(target, newUpwardLast);
                }

                target = compactFront->downwardCells().first();
                if (neuronet->containsEdge(target, oldUpwardLast))
                {
                    neuronet->removeEdge(target, oldUpwardLast);
                    neuronet->addEdge(target, newUpwardLast);
                }
            }

            CompactItem *compactBack = dynamic_cast<CompactItem *>(_backLinkTarget);
            if (compactBack && newDownwardLast != oldDownwardLast)
            {
                NeuroCell::NeuroIndex target = compactBack->upwardCells().first();
                if (neuronet->containsEdge(target, oldDownwardLast))
                {
                    neuronet->removeEdge(target, oldDownwardLast);
                    neuronet->addEdge(target, newDownwardLast);
                }

                target = compactBack->downwardCells().first();
                if (neuronet->containsEdge(target, oldDownwardLast))
                {
                    neuronet->removeEdge(target, oldDownwardLast);
                    neuronet->addEdge(target, newDownwardLast);
                }
            }

            NeuroNarrowItem *narrowFront = dynamic_cast<NeuroNarrowItem *>(_frontLinkTarget);
            if (narrowFront && newUpwardLast != oldUpwardLast)
            {
                NeuroCell::NeuroIndex target = narrowFront->cellIndices().first();
                if (neuronet->containsEdge(target, oldUpwardLast))
                {
                    neuronet->removeEdge(target, oldUpwardLast);
                    neuronet->addEdge(target, newUpwardLast);
                }
            }

            NeuroNarrowItem *narrowBack = dynamic_cast<NeuroNarrowItem *>(_backLinkTarget);
            if (narrowBack && newDownwardLast != oldDownwardLast)
            {
                NeuroCell::NeuroIndex target = narrowBack->cellIndices().first();
                if (neuronet->containsEdge(target, oldDownwardLast))
                {
                    neuronet->removeEdge(target, oldDownwardLast);
                    neuronet->addEdge(target, newDownwardLast);
                }
            }

            // delete the unused cells
            for (QListIterator<NeuroCell::NeuroIndex> i(cellsToDelete); i.hasNext(); )
            {
                neuronet->removeNode(i.next());
            }
        }

        if (updateValue)
            _length_property.setValue(QVariant(newLength));
    }

    NeuroCell::NeuroValue CompactLinkItem::weight() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell(_upward_cells.last());
        return cell ? cell->current().weight() : 0;
    }

    void CompactLinkItem::setWeight(const NeuroLib::NeuroCell::NeuroValue &value)
    {
        setWeightAux(_upward_cells, value);
        setWeightAux(_downward_cells, value);
    }

    void CompactLinkItem::setWeightAux(QList<NeuroLib::NeuroCell::NeuroIndex> &cells, const NeuroLib::NeuroCell::NeuroValue &value)
    {
        for (int i = 0; i < cells.size(); ++i)
        {
            NeuroNet::ASYNC_STATE *cell = getCell(cells[i]);
            if (cell)
            {
                if (i == cells.size() - 1)
                {
                    cell->current().setWeight(value);
                    cell->former().setWeight(value);
                }
                else
                {
                    cell->current().setWeight(1);
                    cell->current().setWeight(1);
                }
            }
        }
    }

    bool CompactLinkItem::handleMove(const QPointF &mousePos, QPointF &movePos)
    {
        // break links
        NeuroItem *linkedItem = _dragFront ? _frontLinkTarget : _backLinkTarget;
        if (linkedItem && !linkedItem->contains(linkedItem->mapFromScene(mousePos)))
        {
            if (_dragFront)
                setFrontLinkTarget(0);
            else
                setBackLinkTarget(0);
        }

        // attach
        return CompactItem::handleMove(mousePos, movePos);
    }

    bool CompactLinkItem::canAttachTo(const QPointF &, NeuroItem *item)
    {
        return true;
    }

    void CompactLinkItem::onAttachTo(NeuroItem *item)
    {
        if (_dragFront)
            setFrontLinkTarget(item);
        else
            setBackLinkTarget(item);
    }

    bool CompactLinkItem::addIncoming(NeuroItem *linkItem)
    {
        Q_ASSERT(linkItem);
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        if (incoming().contains(linkItem))
            return false;

        // the nodes will handle all compact stuff; this is only for narrow nodes
        NeuroNarrowItem *node = dynamic_cast<NeuroNarrowItem *>(linkItem);
        if (node)
        {
            // the node is the back link target, so link to the beginning of the upward chain, and the end of the downward chain
            NeuroCell::NeuroIndex nodeOutgoingIndex = node->cellIndices().last();
            NeuroCell::NeuroIndex nodeIncomingIndex = node->cellIndices().first();

            NeuroCell::NeuroIndex myIncomingIndex = _upward_cells.first();
            NeuroCell::NeuroIndex myOutgoingIndex = _downward_cells.last();

            if (nodeOutgoingIndex != -1 && myIncomingIndex != -1)
                network()->neuronet()->addEdge(myIncomingIndex, nodeOutgoingIndex);
            if (nodeIncomingIndex != -1 && myOutgoingIndex != -1)
                network()->neuronet()->addEdge(nodeIncomingIndex, myOutgoingIndex);
        }

        CompactItem::addIncoming(linkItem);
        return true;
    }

    bool CompactLinkItem::removeIncoming(NeuroItem *linkItem)
    {
        Q_ASSERT(linkItem);
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        // nodes handle all compact stuff; this is only for narrow nodes
        NeuroNarrowItem *node = dynamic_cast<NeuroNarrowItem *>(linkItem);
        if (node)
        {
            // the node is the back link target, so link to the beginning of the upward chain, and the end of the downward chain
            NeuroCell::NeuroIndex nodeOutgoingIndex = node->cellIndices().last();
            NeuroCell::NeuroIndex nodeIncomingIndex = node->cellIndices().first();

            NeuroCell::NeuroIndex myIncomingIndex = _upward_cells.first();
            NeuroCell::NeuroIndex myOutgoingIndex = _downward_cells.last();

            if (nodeOutgoingIndex != -1 && myIncomingIndex != -1)
                network()->neuronet()->removeEdge(myIncomingIndex, nodeOutgoingIndex);
            if (nodeIncomingIndex != -1 && myOutgoingIndex != -1)
                network()->neuronet()->removeEdge(nodeIncomingIndex, myOutgoingIndex);

        }

        // remove the back link target
        if (_backLinkTarget && linkItem == _backLinkTarget)
            setBackLinkTarget(0);

        CompactItem::removeIncoming(linkItem);
        return true;
    }

    bool CompactLinkItem::addOutgoing(NeuroItem *linkItem)
    {
        Q_ASSERT(linkItem);
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        if (outgoing().contains(linkItem))
            return false;

        // nodes handle all compact stuff; this is only for narrow nodes
        NeuroNarrowItem *node = dynamic_cast<NeuroNarrowItem *>(linkItem);
        if (node)
        {
            // the node is the FRONT link target, so link to the end of the upward chain, and the beginning of the downward chain
            NeuroCell::NeuroIndex nodeOutgoingIndex = node->cellIndices().last();
            NeuroCell::NeuroIndex nodeIncomingIndex = node->cellIndices().first();

            NeuroCell::NeuroIndex myIncomingIndex = _downward_cells.first();
            NeuroCell::NeuroIndex myOutgoingIndex = _upward_cells.last();

            if (nodeOutgoingIndex != -1 && myIncomingIndex != -1)
                network()->neuronet()->addEdge(myIncomingIndex, nodeOutgoingIndex);
            if (nodeIncomingIndex != -1 && myOutgoingIndex != -1)
                network()->neuronet()->addEdge(nodeIncomingIndex, myOutgoingIndex);
        }

        CompactItem::addOutgoing(linkItem);
        return true;
    }

    bool CompactLinkItem::removeOutgoing(NeuroItem *linkItem)
    {
        Q_ASSERT(linkItem);
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        // nodes handle all compact stuff; this is only for narrow nodes
        NeuroNodeItem *node = dynamic_cast<NeuroNodeItem *>(linkItem);
        if (node)
        {
            // the node is the FRONT link target, so link to the end of the upward chain, and the beginning of the downward chain
            NeuroCell::NeuroIndex nodeOutgoingIndex = node->cellIndices().last();
            NeuroCell::NeuroIndex nodeIncomingIndex = node->cellIndices().first();

            NeuroCell::NeuroIndex myIncomingIndex = _downward_cells.first();
            NeuroCell::NeuroIndex myOutgoingIndex = _upward_cells.last();

            if (nodeOutgoingIndex != -1 && myIncomingIndex != -1)
                network()->neuronet()->removeEdge(myIncomingIndex, nodeOutgoingIndex);
            if (nodeIncomingIndex != -1 && myOutgoingIndex != -1)
                network()->neuronet()->removeEdge(nodeIncomingIndex, myOutgoingIndex);
        }

        // remove the front link target
        if (_frontLinkTarget && linkItem == _frontLinkTarget)
            setFrontLinkTarget(0);

        CompactItem::removeOutgoing(linkItem);
        return true;
    }

    void CompactLinkItem::addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const
    {
        CompactItem::addToShape(drawPath, texts);
        MixinArrow::addLine(drawPath);
    }

    void CompactLinkItem::setPenProperties(QPen &pen) const
    {
        // get weight
        qreal w = qBound(0.0f, qAbs(weight()), 1.0f);

        // get output values
        QList<qreal> steps;

        for (int i = 0; i < _upward_cells.size(); ++i)
        {
            qreal up = 0, down = 0;

            // get upward value
            const NeuroNet::ASYNC_STATE *cell = getCell(_upward_cells[i]);
            if (cell)
                up = qAbs(cell->current().outputValue());

            // get downward value
            cell = getCell(_downward_cells[(_downward_cells.size()-1) - i]);
            if (cell)
                down = qAbs(cell->current().outputValue());

            steps.append(qBound(0.0, qMax(up, down), 1.0));
        }

        if (steps.size() == 1)
        {
            steps.append(steps.last());
        }
        else if (steps.size() == 0)
        {
            steps.append(0);
            steps.append(0);
        }

        // get gradient
        const NeuroNet::ASYNC_STATE *cell = getCell(_upward_cells.last());
        bool frozen = cell && cell->current().frozen();

        QLinearGradient gradient(_line.p1(), _line.p2());

        for (int i = 0; i < steps.size(); ++i)
        {
            qreal t = i * (1.0f / (steps.size() - 1));

            QColor c = lerp(NORMAL_LINE_COLOR, ACTIVE_COLOR, steps[i]);
            c = lerp(Qt::lightGray, c, w);

            if (frozen)
                c = lerp(c, Qt::gray, 0.5f);

            gradient.setColorAt(t, c);
        }

        // set pen
        pen = QPen(gradient, shouldHighlight() ? HOVER_LINE_WIDTH : NORMAL_LINE_WIDTH);
    }

    void CompactLinkItem::setBrushProperties(QBrush &brush) const
    {
        brush.setStyle(Qt::NoBrush);
    }

    QVariant CompactLinkItem::itemChange(GraphicsItemChange change, const QVariant &value)
    {
        switch (change)
        {
        case QGraphicsItem::ItemPositionChange:
            {
                LabScene *labScene = dynamic_cast<LabScene *>(scene());
                if (!_settingLine && labScene && !labScene->moveOnly() && dynamic_cast<CompactLinkItem *>(labScene->itemUnderMouse()) == this)
                    return MixinArrow::changePos(labScene, value);
            }

        default:
            break;
        }

        return _settingLine ? value : CompactItem::itemChange(change, value);
    }

    void CompactLinkItem::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        CompactItem::writeBinary(ds, file_version);
        MixinArrow::writeBinary(ds, file_version);
    }

    void CompactLinkItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        CompactItem::readBinary(ds, file_version);
        MixinArrow::readBinary(ds, file_version);
    }

    void CompactLinkItem::writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        CompactItem::writePointerIds(ds, file_version);
        MixinArrow::writePointerIds(ds, file_version);
    }

    void CompactLinkItem::readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        CompactItem::readPointerIds(ds, file_version);
        MixinArrow::readPointerIds(ds, file_version);
    }

    void CompactLinkItem::idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap)
    {
        CompactItem::idsToPointers(idMap);
        MixinArrow::idsToPointers(idMap);
    }

} // namespace NeuroGui
