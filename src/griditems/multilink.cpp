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

#include "multilink.h"
#include "neurogriditem.h"
#include "../neurogui/labnetwork.h"
#include "../neurogui/labscene.h"

using namespace NeuroGui;
using namespace NeuroLib;

namespace GridItems
{

    NEUROITEM_DEFINE_PLUGIN_CREATOR(MultiLink, QObject::tr("Grid Items"), QObject::tr("Multi-Link"), ":/griditems/icons/multi_link.png", GridItems::VERSION)

    MultiLink::MultiLink(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : MultiItem(network, scenePos, context), MixinArrow(this),
          _width_property(this, &MultiLink::width, &MultiLink::setWidth, tr("Width")),
          _length_property(this, &MultiLink::length, &MultiLink::setLength, tr("Length")),
          _weight_property(this, &MultiLink::weight, &MultiLink::setWeight, tr("Weight"))
    {
        Q_ASSERT(network != 0);
        Q_ASSERT(network->neuronet() != 0);

        _frontward_lines.append(QList<Index>());
        _backward_lines.append(QList<Index>());

        if (context == CREATE_UI)
        {
            _frontward_lines.first().append(network->neuronet()->addNode(NeuroCell(NeuroCell::EXCITORY_LINK, NeuroCell::DEFAULT_LINK_WEIGHT)));
            _backward_lines.first().append(network->neuronet()->addNode(NeuroCell(NeuroCell::EXCITORY_LINK, NeuroCell::DEFAULT_LINK_WEIGHT)));
        }

        setLine(scenePos.x(), scenePos.y(), scenePos.x() + NeuroItem::NODE_WIDTH * 2, scenePos.y() - NeuroItem::NODE_WIDTH * 2);
    }

    MultiLink::~MultiLink()
    {
    }

    void MultiLink::setWidth(const int & w)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        NeuroNet *neuronet = network()->neuronet();
        bool updateValue = false;

        int new_width = w;
        if (new_width < 1)
        {
            new_width = 1;
            updateValue = true;
        }

        if (new_width != width())
        {
            // disconnect connections (from the other end, since the grid item has special handling)
            foreach (NeuroItem *item, connections())
            {
                NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
                if (ni)
                    ni->removeEdges(this);
            }

            // add lines
            if (new_width > width())
            {
                while (new_width > width())
                {
                    Index oldIndex = -1, newIndex;
                    QList<Index> line;
                    for (int i = 0; i < length(); ++i)
                    {
                        line.append(newIndex = neuronet->addNode(NeuroCell(NeuroCell::EXCITORY_LINK, NeuroCell::DEFAULT_LINK_WEIGHT)));
                        if (oldIndex != -1)
                            neuronet->addEdge(newIndex, oldIndex);
                        oldIndex = newIndex;
                    }
                    _frontward_lines.append(line);

                    oldIndex = -1;
                    line.clear();
                    for (int i = 0; i < length(); ++i)
                    {
                        line.append(newIndex = neuronet->addNode(NeuroCell(NeuroCell::EXCITORY_LINK, NeuroCell::DEFAULT_LINK_WEIGHT)));
                        if (oldIndex != -1)
                            neuronet->addEdge(newIndex, oldIndex);
                        oldIndex = newIndex;
                    }
                    _backward_lines.append(line);
                }
            }
            // remove lines
            else
            {
                while (new_width < width())
                {
                    foreach (Index index, _frontward_lines.last())
                        neuronet->removeNode(index);
                    _frontward_lines.removeLast();

                    foreach (Index index, _backward_lines.last())
                        neuronet->removeNode(index);
                    _backward_lines.removeLast();
                }
            }

            // re-connect connections (from the other end, since the grid item has special handling)
            foreach (NeuroItem *item, connections())
            {
                NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
                if (ni)
                    ni->addEdges(this);
            }
        }

        if (updateValue)
            _width_property.setValueInPropertyBrowser(QVariant(new_width));
    }

    void MultiLink::setLength(const int & l)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        NeuroNet *neuronet = network()->neuronet();
        bool updateValue = false;

        int new_len = l;
        if (new_len < 1)
        {
            new_len = 1;
            updateValue = true;
        }

        if (new_len != length())
        {
            // disconnect connections (from the other end)
            foreach (NeuroItem *item, connections())
            {
                NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
                if (ni)
                    ni->removeEdges(this);
            }

            // adjust internal links
            QList<Index> cells_to_delete;

            // add to lines
            if (new_len > length())
            {
                Index oldLast, newLast;

                QMutableListIterator<QList<Index> > i = _frontward_lines;
                while (i.hasNext())
                {
                    QList<Index> & line = i.next();

                    while (line.length() < new_len)
                    {
                        oldLast = line.last();
                        line.append(newLast = neuronet->addNode(NeuroCell(NeuroCell::EXCITORY_LINK, NeuroCell::DEFAULT_LINK_WEIGHT)));
                        neuronet->addEdge(newLast, oldLast);
                    }
                }

                i = _backward_lines;
                while (i.hasNext())
                {
                    QList<Index> & line = i.next();

                    while (line.length() < new_len)
                    {
                        oldLast = line.last();
                        line.append(newLast = neuronet->addNode(NeuroCell(NeuroCell::EXCITORY_LINK, NeuroCell::DEFAULT_LINK_WEIGHT)));
                        neuronet->addEdge(newLast, oldLast);
                    }
                }
            }
            // remove from lines
            else
            {
                QMutableListIterator<QList<Index> > i = _frontward_lines;
                while (i.hasNext())
                {
                    QList<Index> & line = i.next();

                    while (line.length() > new_len)
                    {
                        cells_to_delete.append(line.last());
                        line.removeLast();
                    }
                }

                i = _backward_lines;
                while (i.hasNext())
                {
                    QList<Index> & line = i.next();

                    while (line.length() > new_len)
                    {
                        cells_to_delete.append(line.last());
                        line.removeLast();
                    }
                }
            }

            // delete unused cells
            foreach (Index index, cells_to_delete)
                neuronet->removeNode(index);

            // re-connect connections
            foreach (NeuroItem *item, connections())
            {
                NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
                if (ni)
                    ni->addEdges(this);
            }
        }

        if (updateValue)
            _length_property.setValueInPropertyBrowser(QVariant(new_len));
    }

    MultiLink::Value MultiLink::weight() const
    {
        if (_frontward_lines.size() > 0 && _frontward_lines.first().size() > 0)
        {
            const NeuroNet::ASYNC_STATE *fcell = getCell(_frontward_lines.first().last());

            Q_ASSERT(fcell != 0);
            return fcell->current().weight();
        }

        return 0;
    }

    void MultiLink::setWeight(const Value & w)
    {
        foreach (QList<Index> line, _frontward_lines)
        {
            for (int i = 0; i < line.size(); ++i)
            {
                NeuroNet::ASYNC_STATE *cell = getCell(line[i]);
                cell->current().setWeight(i == line.size() - 1 ? w : NeuroCell::DEFAULT_LINK_WEIGHT);
            }
        }

        foreach (QList<Index> line, _backward_lines)
        {
            for (int i = 0; i < line.size(); ++i)
            {
                NeuroNet::ASYNC_STATE *cell = getCell(line[i]);
                cell->current().setWeight(i == line.size() - 1 ? w : NeuroCell::DEFAULT_LINK_WEIGHT);
            }
        }
    }

    MultiLink::Value MultiLink::outputValue() const
    {
        if (_override_index != -1)
        {
            const NeuroNet::ASYNC_STATE *cell = getCell(_override_index);
            if (cell)
                return cell->current().outputValue();
        }

        Value max = 0;
        foreach (QList<Index> line, _frontward_lines)
        {
            foreach (Index index, line)
            {
                const NeuroNet::ASYNC_STATE *cell = getCell(index);
                if (cell->current().outputValue() > max)
                    max = cell->current().outputValue();
            }
        }
        foreach (QList<Index> line, _backward_lines)
        {
            foreach (Index index, line)
            {
                const NeuroNet::ASYNC_STATE *cell = getCell(index);
                if (cell->current().outputValue() > max)
                    max = cell->current().outputValue();
            }
        }

        return max;
    }

    QList<MultiLink::Index> MultiLink::getIncomingCellsFor(const NeuroItem *item) const
    {
        QList<Index> results;

        if (item == _frontLinkTarget)
        {
            foreach (QList<Index> line, _backward_lines)
                results.append(line.first());
        }
        else if (item == _backLinkTarget)
        {
            foreach (QList<Index> line, _frontward_lines)
                results.append(line.first());
        }

        return results;
    }

    QList<MultiLink::Index> MultiLink::getOutgoingCellsFor(const NeuroItem *item) const
    {
        QList<Index> results;

        if (item == _frontLinkTarget)
        {
            foreach (QList<Index> line, _frontward_lines)
                results.append(line.last());
        }
        else if (item == _backLinkTarget)
        {
            foreach (QList<Index> line, _backward_lines)
                results.append(line.last());
        }

        return results;
    }

    QList<MultiLink::Index> MultiLink::allCells() const
    {
        QList<Index> results;
        foreach (QList<Index> line, _frontward_lines)
            results.append(line);
        foreach (QList<Index> line, _backward_lines)
            results.append(line);
        return results;
    }

    bool MultiLink::canAttachTo(const QPointF &, NeuroItem *item) const
    {
        NeuroGridItem *gi = dynamic_cast<NeuroGridItem *>(item);
        return gi != 0;
    }

    void MultiLink::onAttachTo(NeuroItem *item)
    {
        if (_dragFront)
            setFrontLinkTarget(item);
        else
            setBackLinkTarget(item);

        MultiItem::onAttachTo(item);
    }

    void MultiLink::onDetach(NeuroItem *item)
    {
        if (item == _frontLinkTarget)
            setFrontLinkTarget(0);
        else if (item == _backLinkTarget)
            setBackLinkTarget(0);

        MultiItem::onDetach(item);
    }

    bool MultiLink::handleMove(const QPointF &mousePos, QPointF &movePos)
    {
        MixinArrow::changePos(movePos);
        MixinArrow::breakLinks(mousePos);
        return MultiItem::handleMove(mousePos, movePos);
    }

    static qreal OFFSET = 4;

    void MultiLink::addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const
    {
        MultiItem::addToShape(drawPath, texts);

        QLineF origLine = _line;
        QVector2D p1v(origLine.p1());
        QVector2D p2v(origLine.p2());

        QVector2D toFront(p2v - p1v);
        toFront.normalize();
        QVector2D perp(-toFront.y(), toFront.x());

        const_cast<QLineF &>(_line) = QLineF((p1v + perp*OFFSET).toPointF(), (p2v + perp*OFFSET).toPointF());
        MixinArrow::addLine(drawPath);

        const_cast<QLineF &>(_line) = QLineF(p1v.toPointF(), p2v.toPointF());
        MixinArrow::addLine(drawPath);

        const_cast<QLineF &>(_line) = QLineF((p1v + perp*-OFFSET).toPointF(), (p2v + perp*-OFFSET).toPointF());
        MixinArrow::addLine(drawPath);

        const_cast<QLineF &>(_line) = origLine;
    }

    void MultiLink::setPenProperties(QPen &pen) const
    {
        MultiItem::setPenProperties(pen);
        if (_override_index == -1)
            setPenGradient(pen, _line);
    }

    void MultiLink::setPenGradient(QPen & pen, const QLineF & line) const
    {
        // get weight
        qreal w = qBound(0.0f, qAbs(weight()), 1.0f);

        // get cell values
        QVector<qreal> steps(length());

        for (int i = 0; i < steps.size(); ++i)
        {
            qreal max = 0;
            foreach (QList<Index> line, _frontward_lines)
            {
                const NeuroNet::ASYNC_STATE *cell = getCell(line[i]);
                qreal val = qAbs(cell->current().outputValue());
                if (val > max) max = val;
            }
            foreach (QList<Index> line, _backward_lines)
            {
                const NeuroNet::ASYNC_STATE *cell = getCell(line[(steps.size()-1) - i]);
                qreal val = qAbs(cell->current().outputValue());
                if (val > max) max = val;
            }

            steps[i] = qBound(0.0, max, 1.0);
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
        QLinearGradient gradient(line.p1(), line.p2());

        for (int i = 0; i < steps.size(); ++i)
        {
            qreal t = i * (1.0f / (steps.size() - 1));

            QColor c = lerp(NORMAL_LINE_COLOR, ACTIVE_COLOR, steps[i]);
            c = lerp(Qt::lightGray, c, w);

            gradient.setColorAt(t, c);
        }

        // set pen
        pen = QPen(gradient, pen.width());
    }

    void MultiLink::writeBinary(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version) const
    {
        MultiItem::writeBinary(ds, file_version);
        MixinArrow::writeBinary(ds, file_version);

        ds << static_cast<quint32>(_frontward_lines.size());
        foreach (QList<Index> line, _frontward_lines)
        {
            ds << static_cast<quint32>(line.size());
            foreach (Index index, line)
                ds << static_cast<quint32>(index);
        }

        ds << static_cast<quint32>(_backward_lines.size());
        foreach (QList<Index> line, _backward_lines)
        {
            ds << static_cast<quint32>(line.size());
            foreach (Index index, line)
                ds << static_cast<quint32>(index);
        }
    }

    void MultiLink::readBinary(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version)
    {
        MultiItem::readBinary(ds, file_version);
        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_8)
            MixinArrow::readBinary(ds, file_version);

        quint32 num_lines, line_size, index;

        _frontward_lines.clear();
        ds >> num_lines;
        for (quint32 i = 0; i < num_lines; ++i)
        {
            QList<Index> line;
            ds >> line_size;
            for (quint32 j = 0; j < line_size; ++j)
            {
                ds >> index;
                line.append(index);
            }
            _frontward_lines.append(line);
        }

        _backward_lines.clear();
        ds >> num_lines;
        for (quint32 i = 0; i < num_lines; ++i)
        {
            QList<Index> line;
            ds >> line_size;
            for (quint32 j = 0; j < line_size; ++j)
            {
                ds >> index;
                line.append(index);
            }
            _backward_lines.append(line);
        }
    }

    void MultiLink::writePointerIds(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version) const
    {
        MultiItem::writePointerIds(ds, file_version);
        MixinArrow::writePointerIds(ds, file_version);
    }

    void MultiLink::readPointerIds(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version)
    {
        MultiItem::readPointerIds(ds, file_version);
        MixinArrow::readPointerIds(ds, file_version);
    }

    void MultiLink::idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap)
    {
        MultiItem::idsToPointers(idMap);
        MixinArrow::idsToPointers(idMap);
    }

} // namespace GridItems
