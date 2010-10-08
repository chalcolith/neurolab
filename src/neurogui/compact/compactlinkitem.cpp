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
#include "../labexception.h"
#include "../subnetwork/subconnectionitem.h"
#include "../subnetwork/subnetworkitem.h"

using namespace NeuroLib;

namespace NeuroGui
{

    NEUROITEM_DEFINE_CREATOR(CompactLinkItem, QObject::tr("Abstract|Link"));

    CompactLinkItem::CompactLinkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : CompactItem(network, scenePos, context), MixinArrow(this),
        _length_property(this, &CompactLinkItem::length, &CompactLinkItem::setLength, tr("Length")),
        _weight_property(this, &CompactLinkItem::weight, &CompactLinkItem::setWeight, tr("Weight")),
        _upward_output_property(this, &CompactLinkItem::upwardOutputValue, &CompactLinkItem::setUpwardOutputValue, tr("Output Value A")),
        _downward_output_property(this, &CompactLinkItem::downwardOutputValue, &CompactLinkItem::setDownwardOutputValue, tr("Output Value B"))
    {
        Q_ASSERT(network != 0);
        Q_ASSERT(network->neuronet() != 0);

        if (context == CREATE_UI)
        {
            _upward_cells.clear();
            addNewCell(true);

            _downward_cells.clear();
            addNewCell(false);
        }
        else
        {
            _upward_cells.append(-1);
            _downward_cells.append(-1);
        }

        setLine(scenePos.x(), scenePos.y(), scenePos.x() + NeuroItem::NODE_WIDTH * 2, scenePos.y() - NeuroItem::NODE_WIDTH * 2);
    }

    CompactLinkItem::~CompactLinkItem()
    {
        if (_ui_delete && network() && network()->neuronet())
        {
            for (QListIterator<NeuroCell::NeuroIndex> i(_upward_cells); i.hasNext(); )
                network()->neuronet()->removeNode(i.next());

            for (QListIterator<NeuroCell::NeuroIndex> i(_downward_cells); i.hasNext(); )
                network()->neuronet()->removeNode(i.next());
        }
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

#if 0 // this is broken
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
#endif

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

    QString CompactLinkItem::dataValue() const
    {
        NeuroCell::NeuroValue v = qMax(upwardOutputValue(), downwardOutputValue());
        return QString::number(v);
    }

    NeuroCell::NeuroValue CompactLinkItem::outputValue(const QList<NeuroCell::NeuroIndex> & cells) const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell(cells.last());
        return cell ? cell->current().outputValue() : 0;
    }

    void CompactLinkItem::setOutputValue(QList<NeuroLib::NeuroCell::NeuroIndex> &cells, const NeuroLib::NeuroCell::NeuroValue &value)
    {
        for (int i = 0; i < cells.size(); ++i)
        {
            NeuroNet::ASYNC_STATE *cell = getCell(cells[i]);
            if (cell)
            {
                if (i == cells.size() - 1)
                {
                    cell->current().setOutputValue(value);
                    cell->former().setOutputValue(value);
                }
                else
                {
                    cell->current().setOutputValue(0);
                    cell->former().setOutputValue(0);
                }
            }
        }
    }

    void CompactLinkItem::reset()
    {
        setUpwardOutputValue(0);
        setDownwardOutputValue(0);
    }

    bool CompactLinkItem::handleMove(const QPointF &mousePos, QPointF &movePos)
    {
        // move line
        MixinArrow::changePos(movePos);

        // break links
        MixinArrow::breakLinks(mousePos);

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

    void CompactLinkItem::getMyIndices(NeuroItem *linkItem,
                                       NeuroLib::NeuroCell::NeuroIndex &myIncomingIndex,
                                       NeuroLib::NeuroCell::NeuroIndex &myOutgoingIndex)
    {
        Q_ASSERT(linkItem);

        bool isFront = false;
        bool isBack = false;

        if (_frontLinkTarget && _frontLinkTarget == linkItem)
        {
            isFront = true;
        }
        else if (_backLinkTarget && _backLinkTarget == linkItem)
        {
            isBack = true;
        }
        else // special case for subconnection items
        {
            // check the link item's other connections; if one is a subconnection
            // item, check which end the subconnection item's parent is attached to
            for (QSetIterator<NeuroItem *> i(linkItem->incoming()); i.hasNext(); )
            {
                SubConnectionItem *subItem = dynamic_cast<SubConnectionItem *>(i.next());
                if (subItem && subItem->parentSubnetwork() == _frontLinkTarget)
                {
                    isFront = true;
                    break;
                }
                else if (subItem && subItem->parentSubnetwork() == _backLinkTarget)
                {
                    isBack = true;
                    break;
                }
            }

            if (!isFront && !isBack)
            {
                for (QSetIterator<NeuroItem *> i(linkItem->outgoing()); i.hasNext(); )
                {
                    SubConnectionItem *subItem = dynamic_cast<SubConnectionItem *>(i.next());
                    if (subItem && subItem->parentSubnetwork() == _frontLinkTarget)
                    {
                        isFront = true;
                        break;
                    }
                    else if (subItem && subItem->parentSubnetwork() == _backLinkTarget)
                    {
                        isBack = true;
                        break;
                    }
                }
            }
        }

        // get the appropriate indices for connecting or disconnecting
        if (isFront)
        {
            myIncomingIndex = _downward_cells.first();
            myOutgoingIndex = _upward_cells.last();
        }
        else if (isBack)
        {
            myIncomingIndex = _upward_cells.first();
            myOutgoingIndex = _downward_cells.last();
        }
        else
        {
            throw LabException(tr("CompactLinkItem trying to get indices for unknown link connection."));
        }
    }

    bool CompactLinkItem::addIncoming(NeuroItem *linkItem)
    {
        Q_ASSERT(linkItem);
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        if (CompactItem::addIncoming(linkItem))
        {
            // the nodes will handle all compact stuff; this is only for narrow nodes
            NeuroNarrowItem *node = dynamic_cast<NeuroNarrowItem *>(linkItem);
            if (node)
            {
                NeuroCell::NeuroIndex nodeOutgoingIndex = node->cellIndices().last();
                NeuroCell::NeuroIndex nodeIncomingIndex = node->cellIndices().first();

                NeuroCell::NeuroIndex myIncomingIndex, myOutgoingIndex;
                getMyIndices(linkItem, myIncomingIndex, myOutgoingIndex);

                if (nodeOutgoingIndex != -1 && myIncomingIndex != -1)
                    network()->neuronet()->addEdge(myIncomingIndex, nodeOutgoingIndex);
                if (nodeIncomingIndex != -1 && myOutgoingIndex != -1)
                    network()->neuronet()->addEdge(nodeIncomingIndex, myOutgoingIndex);
            }

            return true;
        }

        return false;
    }

    void CompactLinkItem::removeIncoming(NeuroItem *linkItem)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        if (linkItem && incoming().contains(linkItem))
        {
            // nodes handle all compact stuff; this is only for narrow nodes
            NeuroNarrowItem *node = dynamic_cast<NeuroNarrowItem *>(linkItem);
            if (node)
            {
                NeuroCell::NeuroIndex nodeOutgoingIndex = node->cellIndices().last();
                NeuroCell::NeuroIndex nodeIncomingIndex = node->cellIndices().first();

                NeuroCell::NeuroIndex myIncomingIndex, myOutgoingIndex;
                getMyIndices(linkItem, myIncomingIndex, myOutgoingIndex);

                if (nodeOutgoingIndex != -1 && myIncomingIndex != -1)
                    network()->neuronet()->removeEdge(myIncomingIndex, nodeOutgoingIndex);
                if (nodeIncomingIndex != -1 && myOutgoingIndex != -1)
                    network()->neuronet()->removeEdge(nodeIncomingIndex, myOutgoingIndex);
            }

            CompactItem::removeIncoming(linkItem);
        }
    }

    bool CompactLinkItem::addOutgoing(NeuroItem *linkItem)
    {
        Q_ASSERT(linkItem);
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        if (CompactItem::addOutgoing(linkItem))
        {
            // nodes handle all compact stuff; this is only for narrow nodes
            NeuroNarrowItem *node = dynamic_cast<NeuroNarrowItem *>(linkItem);
            if (node)
            {
                NeuroCell::NeuroIndex nodeOutgoingIndex = node->cellIndices().last();
                NeuroCell::NeuroIndex nodeIncomingIndex = node->cellIndices().first();

                NeuroCell::NeuroIndex myIncomingIndex, myOutgoingIndex;
                getMyIndices(linkItem, myIncomingIndex, myOutgoingIndex);

                if (nodeOutgoingIndex != -1 && myIncomingIndex != -1)
                    network()->neuronet()->addEdge(myIncomingIndex, nodeOutgoingIndex);
                if (nodeIncomingIndex != -1 && myOutgoingIndex != -1)
                    network()->neuronet()->addEdge(nodeIncomingIndex, myOutgoingIndex);
            }

            return true;
        }

        return false;
    }

    void CompactLinkItem::removeOutgoing(NeuroItem *linkItem)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        if (linkItem && outgoing().contains(linkItem))
        {
            // nodes handle all compact stuff; this is only for narrow nodes
            NeuroNodeItem *node = dynamic_cast<NeuroNodeItem *>(linkItem);
            if (node)
            {
                NeuroCell::NeuroIndex nodeOutgoingIndex = node->cellIndices().last();
                NeuroCell::NeuroIndex nodeIncomingIndex = node->cellIndices().first();

                NeuroCell::NeuroIndex myIncomingIndex, myOutgoingIndex;
                getMyIndices(linkItem, myIncomingIndex, myOutgoingIndex);

                if (nodeOutgoingIndex != -1 && myIncomingIndex != -1)
                    network()->neuronet()->removeEdge(myIncomingIndex, nodeOutgoingIndex);
                if (nodeIncomingIndex != -1 && myOutgoingIndex != -1)
                    network()->neuronet()->removeEdge(nodeIncomingIndex, myOutgoingIndex);
            }

            CompactItem::removeOutgoing(linkItem);
        }
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

    void CompactLinkItem::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        CompactItem::writeBinary(ds, file_version);

        quint32 num = _upward_cells.size();
        ds << num;
        for (quint32 i = 0; i < num; ++i)
        {
            quint32 index = _upward_cells[i];
            ds << index;
        }

        num = _downward_cells.size();
        ds << num;
        for (quint32 i = 0; i < num; ++i)
        {
            quint32 index = _downward_cells[i];
            ds << index;
        }

        MixinArrow::writeBinary(ds, file_version);
    }

    void CompactLinkItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        CompactItem::readBinary(ds, file_version);

        quint32 num;

        ds >> num;
        _upward_cells.clear();
        for (quint32 i = 0; i < num; ++i)
        {
            quint32 index;
            ds >> index;
            _upward_cells.append(static_cast<NeuroCell::NeuroIndex>(index));
        }

        ds >> num;
        _downward_cells.clear();
        for (quint32 i = 0; i < num; ++i)
        {
            quint32 index;
            ds >> index;
            _downward_cells.append(static_cast<NeuroCell::NeuroIndex>(index));
        }

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
