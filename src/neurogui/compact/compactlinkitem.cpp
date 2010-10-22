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

    NEUROITEM_DEFINE_CREATOR(CompactLinkItem, QObject::tr("Abstract"), QObject::tr("Link"));

    CompactLinkItem::CompactLinkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : CompactItem(network, scenePos, context), MixinArrow(this),
        _length_property(this, &CompactLinkItem::length, &CompactLinkItem::setLength, tr("Length")),
        _weight_property(this, &CompactLinkItem::weight, &CompactLinkItem::setWeight, tr("Weight")),
        _frontward_output_property(this, &CompactLinkItem::frontwardOutputValue, &CompactLinkItem::setFrontwardOutputValue,
                                   tr("Frontward Output Value")),
        _backward_output_property(this, &CompactLinkItem::backwardOutputValue, &CompactLinkItem::setBackwardOutputValue,
                                  tr("Backward Output Value"))
    {
        Q_ASSERT(network != 0);
        Q_ASSERT(network->neuronet() != 0);

        if (context == CREATE_UI)
        {
            _frontward_cells.clear();
            addNewCell(true);

            _backward_cells.clear();
            addNewCell(false);
        }
        else
        {
            _frontward_cells.append(-1);
            _backward_cells.append(-1);
        }

        setLine(scenePos.x(), scenePos.y(), scenePos.x() + NeuroItem::NODE_WIDTH * 2, scenePos.y() - NeuroItem::NODE_WIDTH * 2);
    }

    CompactLinkItem::~CompactLinkItem()
    {
        if (_ui_delete)
        {
            Q_ASSERT(network());
            Q_ASSERT(network()->neuronet());

            foreach (NeuroCell::Index i, _frontward_cells)
                network()->neuronet()->removeNode(i);

            foreach (NeuroCell::Index i, _backward_cells)
                network()->neuronet()->removeNode(i);
        }
    }

    void CompactLinkItem::addNewCell(bool frontwards)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        NeuroCell::Index index = network()->neuronet()->addNode(NeuroCell(NeuroCell::EXCITORY_LINK, NeuroCell::DEFAULT_LINK_WEIGHT));
        if (frontwards)
            _frontward_cells.append(index);
        else
            _backward_cells.append(index);
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
            // disconnect connections
            foreach (NeuroItem *item, connections())
                removeEdges(item);

            // adjust the internal cell links
            QList<NeuroCell::Index> cellsToDelete;

            if (newLength > length())
            {
                NeuroCell::Index oldUp, newUp, oldDown, newDown;

                while (length() < newLength)
                {
                    oldUp = _frontward_cells.last();
                    addNewCell(true);
                    newUp = _frontward_cells.last();
                    neuronet->addEdge(newUp, oldUp);

                    oldDown = _backward_cells.last();
                    addNewCell(false);
                    newDown = _backward_cells.last();
                    neuronet->addEdge(newDown, oldDown);
                }
            }
            else
            {
                while (length() > newLength)
                {
                    NeuroCell::Index index = _frontward_cells.last();
                    cellsToDelete.append(index);
                    _frontward_cells.removeLast();

                    index = _backward_cells.last();
                    cellsToDelete.append(index);
                    _backward_cells.removeLast();
                }
            }

            // delete the unused cells
            foreach (NeuroCell::Index i, cellsToDelete)
                neuronet->removeNode(i);

            // re-connect
            foreach (NeuroItem *item, connections())
                addEdges(item);
        }

        if (updateValue)
            _length_property.setValue(QVariant(newLength));
    }

    NeuroCell::Value CompactLinkItem::weight() const
    {
        const NeuroNet::ASYNC_STATE *fcell = getCell(_frontward_cells.last());
        const NeuroNet::ASYNC_STATE *bcell = getCell(_backward_cells.last());

        return (fcell != 0 && bcell != 0) ? qMax(fcell->current().weight(), bcell->current().weight()) : 0;
    }

    void CompactLinkItem::setWeight(const NeuroLib::NeuroCell::Value &value)
    {
        setWeightAux(_frontward_cells, value);
        setWeightAux(_backward_cells, value);
    }

    void CompactLinkItem::setWeightAux(QList<NeuroLib::NeuroCell::Index> &cells, const NeuroLib::NeuroCell::Value &value)
    {
        for (int i = 0; i < cells.size(); ++i)
        {
            NeuroNet::ASYNC_STATE *cell = getCell(cells[i]);
            if (cell)
            {
                if (i == (cells.size() - 1))
                {
                    cell->current().setWeight(value);
                    cell->former().setWeight(value);
                }
                else
                {
                    cell->current().setWeight(1);
                    cell->former().setWeight(1);
                }
            }
        }
    }

    NeuroCell::Value CompactLinkItem::outputValueAux(const QList<NeuroCell::Index> & cells) const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell(cells.last());
        return cell ? cell->current().outputValue() : 0;
    }

    void CompactLinkItem::setOutputValueAux(QList<NeuroLib::NeuroCell::Index> &cells, const NeuroLib::NeuroCell::Value &value)
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

    NeuroCell::Index CompactLinkItem::getIncomingCellFor(const NeuroItem *item) const
    {
        if (item == _frontLinkTarget)
            return _backward_cells.first();
        else if (item == _backLinkTarget)
            return _frontward_cells.first();
        return -1;
    }

    NeuroCell::Index CompactLinkItem::getOutgoingCellFor(const NeuroItem *item) const
    {
        if (item == _frontLinkTarget)
            return _frontward_cells.last();
        else if (item == _backLinkTarget)
            return _backward_cells.last();
        return -1;
    }

    void CompactLinkItem::reset()
    {
        setFrontwardOutputValue(0);
        setBackwardOutputValue(0);
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

    void CompactLinkItem::adjustLinks()
    {
        QVector2D center = QVector2D(scenePos()) + ((c1 + c2) * 0.5f);

        foreach (NeuroItem *ni, _incoming)
        {
            MixinArrow *link = dynamic_cast<MixinArrow *>(ni);
            if (link)
                link->setLine(link->line().p1(), center.toPointF());
        }
    }

    void CompactLinkItem::addEdges(NeuroItem *item)
    {
        NeuroInhibitoryLinkItem *inhibit = dynamic_cast<NeuroInhibitoryLinkItem *>(item);

        if (inhibit)
        {
            NeuroCell::Index linkOut = inhibit->getOutgoingCellFor(this);
            NeuroCell::Index myIn = _frontward_cells.first();
            if (linkOut != -1 && myIn != -1)
                network()->neuronet()->addEdge(myIn, linkOut);

            myIn = _backward_cells.first();
            if (linkOut != -1 && myIn != -1)
                network()->neuronet()->addEdge(myIn, linkOut);
        }
        else
        {
            CompactItem::addEdges(item);
        }
    }

    void CompactLinkItem::removeEdges(NeuroItem *item)
    {
        NeuroInhibitoryLinkItem *inhibit = dynamic_cast<NeuroInhibitoryLinkItem *>(item);

        if (inhibit)
        {
            NeuroCell::Index linkOut = inhibit->getOutgoingCellFor(this);

            NeuroCell::Index myIn = _frontward_cells.first();
            if (linkOut != -1 && myIn != -1)
                network()->neuronet()->removeEdge(myIn, linkOut);

            myIn = _backward_cells.first();
            if (linkOut != -1 && myIn != -1)
                network()->neuronet()->removeEdge(myIn, linkOut);
        }
        else
        {
            CompactItem::removeEdges(item);
        }
    }

    bool CompactLinkItem::canAttachTo(const QPointF &, NeuroItem *)
    {
        return true;
    }

    bool CompactLinkItem::canBeAttachedBy(const QPointF &, NeuroItem *item)
    {
        NeuroInhibitoryLinkItem *inhibit = dynamic_cast<NeuroInhibitoryLinkItem *>(item);
        return inhibit != 0;
    }

    void CompactLinkItem::onAttachTo(NeuroItem *item)
    {
        if (_dragFront)
            setFrontLinkTarget(item);
        else
            setBackLinkTarget(item);

        CompactItem::onAttachTo(item);
    }

    void CompactLinkItem::onAttachedBy(NeuroItem *item)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        _incoming.insert(item);
        CompactItem::onAttachedBy(item);
        addEdges(item);
        adjustLinks();
    }

    void CompactLinkItem::onDetach(NeuroItem *item)
    {
        if (_dragFront && item == _frontLinkTarget)
            setFrontLinkTarget(0);
        else if (!_dragFront && item == _backLinkTarget)
            setBackLinkTarget(0);

        _incoming.remove(item);
        CompactItem::onDetach(item);
    }

    void CompactLinkItem::addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const
    {
        CompactItem::addToShape(drawPath, texts);
        MixinArrow::addLine(drawPath);
    }

    void CompactLinkItem::setPenProperties(QPen &pen) const
    {
        CompactItem::setPenProperties(pen);
        setPenGradient(pen, _line);
    }

    void CompactLinkItem::setPenGradient(QPen &pen, const QLineF &line) const
    {
        // get weight
        qreal w = qBound(0.0f, qAbs(weight()), 1.0f);

        // get output values
        QList<qreal> steps;

        for (int i = 0; i < _frontward_cells.size(); ++i)
        {
            qreal up = 0, down = 0;

            // get upward value
            const NeuroNet::ASYNC_STATE *cell = getCell(_frontward_cells[i]);
            if (cell)
                up = qAbs(cell->current().outputValue());

            // get downward value
            cell = getCell(_backward_cells[(_backward_cells.size()-1) - i]);
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
        const NeuroNet::ASYNC_STATE *cell = getCell(_frontward_cells.last());
        bool frozen = cell && cell->current().frozen();

        QLinearGradient gradient(line.p1(), line.p2());

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
        pen = QPen(gradient, pen.width());
    }

    void CompactLinkItem::writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const
    {
        CompactItem::writeClipboard(ds, id_map);
        MixinArrow::writeClipboard(ds, id_map);
    }

    void CompactLinkItem::readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> &id_map)
    {
        CompactItem::readClipboard(ds, id_map);
        MixinArrow::readClipboard(ds, id_map);
    }

    void CompactLinkItem::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        CompactItem::writeBinary(ds, file_version);
        MixinArrow::writeBinary(ds, file_version);

        quint32 num = _frontward_cells.size();
        ds << num;
        for (quint32 i = 0; i < num; ++i)
        {
            quint32 index = _frontward_cells[i];
            ds << index;
        }

        num = _backward_cells.size();
        ds << num;
        for (quint32 i = 0; i < num; ++i)
        {
            quint32 index = _backward_cells[i];
            ds << index;
        }
    }

    void CompactLinkItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        CompactItem::readBinary(ds, file_version);
        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_8)
            MixinArrow::readBinary(ds, file_version);

        quint32 num;

        ds >> num;
        _frontward_cells.clear();
        for (quint32 i = 0; i < num; ++i)
        {
            quint32 index;
            ds >> index;
            _frontward_cells.append(static_cast<NeuroCell::Index>(index));
        }

        ds >> num;
        _backward_cells.clear();
        for (quint32 i = 0; i < num; ++i)
        {
            quint32 index;
            ds >> index;
            _backward_cells.append(static_cast<NeuroCell::Index>(index));
        }

        if (file_version.neurolab_version < NeuroGui::NEUROLAB_FILE_VERSION_8)
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
