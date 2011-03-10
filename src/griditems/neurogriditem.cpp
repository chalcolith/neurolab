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

#include "neurogriditem.h"
#include "../neurogui/labnetwork.h"
#include "../neurogui/labscene.h"
#include "../neurogui/labview.h"
#include "../neurogui/labtree.h"
#include "../neurogui/mainwindow.h"
#include "gridedgeitem.h"

using namespace NeuroGui;

namespace GridItems
{

    const QString
#include "../version.txt"
    ;

    NEUROITEM_DEFINE_PLUGIN_CREATOR(NeuroGridItem, QString("Grid Items"), QObject::tr("Grid Item"), GridItems::VERSION)

    NeuroGridItem::NeuroGridItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : SubNetworkItem(network, scenePos, context),
          _horizontal_property(this, &NeuroGridItem::horizontalCols, &NeuroGridItem::setHorizontalCols, tr("Horizontal Repeats")),
          _vertical_property(this, &NeuroGridItem::verticalRows, &NeuroGridItem::setVerticalRows, tr("Vertical Repeats")),
          _num_horiz(1), _num_vert(1), _pattern_dirty(false)
    {
        if (context == NeuroItem::CREATE_UI)
        {
            const int width = NODE_WIDTH*4;
            const int height = NODE_WIDTH*2;
            const int left = -width/2;
            const int top = -height/2;

            setRect(QRectF(left, top, width, height));
            setLabelPos(QPointF(left + width + 5, 0));

            treeNode()->setLabel(tr("Grid Item %1").arg(treeNode()->id()));
        }
    }

    NeuroGridItem::~NeuroGridItem()
    {
    }

    void NeuroGridItem::makeSubNetwork()
    {
        SubNetworkItem::makeSubNetwork();

        LabView *view = MainWindow::instance()->currentNetwork()->view();
        Q_ASSERT(view);

        QRect viewRect = view->viewport()->rect();
        QPointF topLeft = view->mapToScene(viewRect.topLeft());
        QPointF bottomRight = view->mapToScene(viewRect.bottomRight());

        QRect newSceneRect(topLeft.toPoint(), bottomRight.toPoint());

        treeNode()->scene()->setSceneRect(newSceneRect);
    }

    QList<NeuroGridItem::Index> NeuroGridItem::getIncomingCellsFor(const NeuroItem *item) const
    {
        NeuroNetworkItem *ni = const_cast<NeuroNetworkItem *>(dynamic_cast<const NeuroNetworkItem *>(item));

        if (_top_connections.contains(ni))
            return _top_incoming;
        else if (_bottom_connections.contains(ni))
            return _bot_incoming;

        return QList<Index>();
    }

    QList<NeuroGridItem::Index> NeuroGridItem::getOutgoingCellsFor(const NeuroItem *item) const
    {
        NeuroNetworkItem *ni = const_cast<NeuroNetworkItem *>(dynamic_cast<const NeuroNetworkItem *>(item));

        if (_top_connections.contains(ni))
            return _top_outgoing;
        else if (_bottom_connections.contains(ni))
            return _bot_outgoing;

        return QList<Index>();
    }

    void NeuroGridItem::onEnterView()
    {
    }

    void NeuroGridItem::onLeaveView()
    {
        foreach (NeuroItem *item, connections())
            removeEdges(item);

        generateGrid();

        foreach (NeuroItem *item, connections())
            addEdges(item);
    }

    void NeuroGridItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroNetworkItem::addToShape(drawPath, texts);

        const QRectF & r = rect();
        drawPath.addRect(r);

        const int num_vertical = 5;
        for (int i = 0; i < num_vertical; ++i)
        {
            int x = r.left() + (i+1)*r.width()/(num_vertical+1);
            drawPath.moveTo(x, r.top());
            drawPath.lineTo(x, r.top() + r.height());
        }

        const int num_horizontal = 3;
        for (int i = 0; i < num_horizontal; ++i)
        {
            int y = r.top() + (i+1)*r.height()/(num_horizontal+1);
            drawPath.moveTo(r.left(), y);
            drawPath.lineTo(r.left() + r.width(), y);
        }
    }

    bool NeuroGridItem::canCreateNewItem(const QString & typeName, const QPointF &) const
    {
        if (typeName.contains("ExcitoryLinkItem"))
            return true;
        if (typeName.contains("InhibitoryLinkItem"))
            return true;
        if (typeName.contains("NeuroNodeItem"))
            return true;
        if (typeName.contains("NeuroOscillatorItem"))
            return true;
        if (typeName.contains("TextItem"))
            return true;
        if (typeName.contains("GridEdgeItem"))
            return true;

        return false;
    }

    bool NeuroGridItem::canBeAttachedBy(const QPointF & pos, NeuroItem *item) const
    {
        NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
        if (!ni) return false;

        // only allow connections to the top or bottom
        QPointF topLeft = mapToScene(rect().topLeft());
        QPointF bottomRight = mapToScene(rect().bottomRight());

        if (pos.y() > topLeft.y() && pos.y() < bottomRight.y())
            return false;

        return true;
    }

    void NeuroGridItem::onAttachedBy(NeuroItem *item)
    {
        NeuroNetworkItem::onAttachedBy(item); // we don't want any sub-connection items

        NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
        if (ni)
        {
            QPointF mousePos = network()->scene()->lastMousePos();

            if (mousePos.y() < scenePos().y())
                _top_connections.insert(ni);
            else
                _bottom_connections.insert(ni);

        }

        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        if (link)
        {
            MixinRemember::onAttachedBy(link);
        }
    }

    void NeuroGridItem::onDetach(NeuroItem *item)
    {
        NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
        if (ni)
        {
            _top_connections.remove(ni);
            _bottom_connections.remove(ni);
        }

        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        if (link)
        {
            MixinRemember::onDetach(link);
        }

        NeuroNetworkItem::onDetach(item); // no subconnection items
    }

    static void add_cells(QSet<NeuroGridItem::Index> & set, const QList<NeuroGridItem::Index> & list)
    {
        foreach (const NeuroGridItem::Index & idx, list)
            set.insert(idx);
    }

    static QMap<NeuroGridItem::Index, NeuroGridItem::Index> &
    get_to_copy(QVector<QMap<NeuroGridItem::Index, NeuroGridItem::Index> > & all_copies, int row, int col, int num_cols)
    {
        return all_copies[(row * num_cols) + col];
    }

    void NeuroGridItem::generateGrid()
    {
        // get neuronet pointer
        NeuroLib::NeuroNet *neuronet = 0;

        if (network())
            neuronet = network()->neuronet();

        if (!neuronet)
            return;

        // set status; this might take a while
        MainWindow::instance()->setStatus(tr("Generating neural network grid..."));
        QApplication::setOverrideCursor(Qt::WaitCursor);

        // delete all existing used cells from the automaton
        foreach (const Index & index, _all_grid_cells)
        {
            neuronet->removeNode(index);
        }

        // get all cells used by all items
        QSet<Index> all_pattern_cells;

        QSet<Index> top_incoming_pattern_cells;
        QSet<Index> top_outgoing_pattern_cells;

        QSet<Index> bot_incoming_pattern_cells;
        QSet<Index> bot_outgoing_pattern_cells;

        QSet<Index> left_incoming_pattern_cells;
        QSet<Index> left_outgoing_pattern_cells;

        QSet<Index> right_incoming_pattern_cells;
        QSet<Index> right_outgoing_pattern_cells;

        // collect all cells in the pattern and cells on the edges
        foreach (QGraphicsItem *item, treeNode()->scene()->items())
        {
            NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
            if (ni)
            {
                foreach (Index index, ni->allCells())
                {
                    all_pattern_cells.insert(index);
                }
            }

            GridEdgeItem *ei = dynamic_cast<GridEdgeItem *>(item);
            if (ei)
            {
                foreach (NeuroItem *cx, ei->connections())
                {
                    NeuroNetworkItem *cxni = dynamic_cast<NeuroNetworkItem *>(cx);
                    if (cxni)
                    {
                        if (ei->isConnectedToTop(cxni))
                        {
                            add_cells(top_incoming_pattern_cells, cxni->getIncomingCellsFor(ei));
                            add_cells(top_outgoing_pattern_cells, cxni->getOutgoingCellsFor(ei));
                        }
                        else if (ei->isConnectedToBottom(cxni))
                        {
                            add_cells(bot_incoming_pattern_cells, cxni->getIncomingCellsFor(ei));
                            add_cells(bot_outgoing_pattern_cells, cxni->getOutgoingCellsFor(ei));
                        }
                        else if (ei->isConnectedToLeft(cxni))
                        {
                            add_cells(left_incoming_pattern_cells, cxni->getIncomingCellsFor(ei));
                            add_cells(left_outgoing_pattern_cells, cxni->getOutgoingCellsFor(ei));
                        }
                        else if (ei->isConnectedToRight(cxni))
                        {
                            add_cells(right_incoming_pattern_cells, cxni->getIncomingCellsFor(ei));
                            add_cells(right_outgoing_pattern_cells, cxni->getOutgoingCellsFor(ei));
                        }
                    }
                }
            }
        }

        // make copies of the pattern for each grid square
        QVector<QMap<Index, Index> > pattern_to_all_copies(_num_vert * _num_horiz);

        for (int row = 0; row < _num_vert; ++row)
        {
            for (int col = 0; col < _num_horiz; ++col)
            {
                QMap<Index, Index> & pat_to_copy = get_to_copy(pattern_to_all_copies, row, col, _num_horiz);

                // copy pattern cells
                foreach (const Index & pat_index, all_pattern_cells)
                {
                    if (pat_index != -1)
                    {
                        Index copy_index = neuronet->addNode((*neuronet)[pat_index].current());
                        _all_grid_cells.insert(copy_index);
                        pat_to_copy[pat_index] = copy_index;
                    }
                }

            }
        }

        // duplicate internal edges
        for (int row = 0; row < _num_vert; ++row)
        {
            for (int col = 0; col < _num_horiz; ++col)
            {
                const QMap<Index, Index> & pat_to_copy = get_to_copy(pattern_to_all_copies, row, col, _num_horiz);

                foreach (const Index & pat_index, pat_to_copy.keys())
                {
                    const Index & copy_index = pat_to_copy[pat_index];

                    int num;
                    const Index *pat_neighbors = neuronet->neighbors(pat_index, num);
                    for (int i = 0; i < num; ++i)
                    {
                        const Index & copy_neighbor = pat_to_copy[pat_neighbors[i]];
                        neuronet->addEdge(copy_index, copy_neighbor);
                    }
                }
            }
        }

        // now connect up all the copies, connecting the top row and bottom row to the outside...
        // remember that neighbors are incoming cells!
        QSet<Index> top_incoming_temp;
        QSet<Index> top_outgoing_temp;
        QSet<Index> bot_incoming_temp;
        QSet<Index> bot_outgoing_temp;

        for (int row = 0; row < _num_vert; ++row)
        {
            for (int col = 0; col < _num_horiz; ++col)
            {
                const QMap<Index, Index> & pat_to_copy = get_to_copy(pattern_to_all_copies, row, col, _num_horiz);

                int left_col = (col + _num_horiz - 1) % _num_horiz;
                int right_col = (col + 1) % _num_horiz;
                int up_row = row - 1;
                int down_row = row + 1;

                // top
                if (row == 0)
                {
                    foreach (const Index & idx, top_incoming_pattern_cells)
                        top_incoming_temp.insert(pat_to_copy[idx]);
                    foreach (const Index & idx, top_outgoing_pattern_cells)
                        top_outgoing_temp.insert(pat_to_copy[idx]);
                }
                else
                {
                    QMap<Index, Index> & up_copy = get_to_copy(pattern_to_all_copies, up_row, col, _num_horiz);

                    foreach (const Index & idx, top_incoming_pattern_cells)
                        neuronet->addEdge(pat_to_copy[idx], up_copy[idx]);
                    foreach (const Index & idx, top_outgoing_pattern_cells)
                        neuronet->addEdge(up_copy[idx], pat_to_copy[idx]);
                }

                // bottom
                if (row == _num_vert - 1)
                {
                    foreach (const Index & idx, bot_incoming_pattern_cells)
                        bot_incoming_temp.insert(pat_to_copy[idx]);
                    foreach (const Index & idx, bot_outgoing_pattern_cells)
                        bot_outgoing_temp.insert(pat_to_copy[idx]);
                }
                else
                {
                    QMap<Index, Index> & down_copy = get_to_copy(pattern_to_all_copies, down_row, col, _num_horiz);

                    foreach (const Index & idx, bot_incoming_pattern_cells)
                        neuronet->addEdge(pat_to_copy[idx], down_copy[idx]);
                    foreach (const Index & idx, bot_outgoing_pattern_cells)
                        neuronet->addEdge(down_copy[idx], pat_to_copy[idx]);
                }

                // left
                QMap<Index, Index> & left_copy = get_to_copy(pattern_to_all_copies, row, left_col, _num_horiz);

                foreach (const Index & idx, left_incoming_pattern_cells)
                    neuronet->addEdge(pat_to_copy[idx], left_copy[idx]);
                foreach (const Index & idx, left_outgoing_pattern_cells)
                    neuronet->addEdge(left_copy[idx], pat_to_copy[idx]);

                // right
                QMap<Index, Index> & right_copy = get_to_copy(pattern_to_all_copies, row, right_col, _num_horiz);

                foreach (const Index & idx, right_incoming_pattern_cells)
                    neuronet->addEdge(pat_to_copy[idx], right_copy[idx]);
                foreach (const Index & idx, right_outgoing_pattern_cells)
                    neuronet->addEdge(right_copy[idx], pat_to_copy[idx]);
            }
        }

        _top_incoming.clear();
        foreach (const Index & idx, top_incoming_temp)
            _top_incoming.append(idx);

        _top_outgoing.clear();
        foreach (const Index & idx, top_outgoing_temp)
            _top_outgoing.append(idx);

        _bot_incoming.clear();
        foreach (const Index & idx, bot_incoming_temp)
            _bot_incoming.append(idx);

        _bot_outgoing.clear();
        foreach (const Index & idx, bot_outgoing_temp)
            _bot_outgoing.append(idx);

        // done
        QApplication::restoreOverrideCursor();
        MainWindow::instance()->setStatus(tr("Generated neural network grid."));
    }


    void NeuroGridItem::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        SubNetworkItem::writeBinary(ds, file_version);

        ds << _num_horiz;
        ds << _num_vert;
    }

    void NeuroGridItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        SubNetworkItem::readBinary(ds, file_version);

        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_9)
        {
            ds >> _num_horiz;
            ds >> _num_vert;
        }
    }

    void NeuroGridItem::writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        SubNetworkItem::writePointerIds(ds, file_version);

        quint32 num = _top_connections.size();
        ds << num;
        foreach (const NeuroNetworkItem *ni, _top_connections)
            ds << static_cast<IdType>(ni ? ni->id() : 0);

        num = _bottom_connections.size();
        ds << num;
        foreach (const NeuroNetworkItem *ni, _bottom_connections)
            ds << static_cast<IdType>(ni ? ni->id() : 0);
    }

    void NeuroGridItem::readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        SubNetworkItem::readPointerIds(ds, file_version);

        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_9)
        {
            quint32 num;

            _top_connections.clear();
            ds >> num;
            for (quint32 i = 0; i < num; ++i)
            {
                IdType id;
                ds >> id;
                if (id) _top_connections.insert(reinterpret_cast<NeuroNetworkItem *>(id));
            }

            _bottom_connections.clear();
            ds >> num;
            for (quint32 i = 0; i < num; ++i)
            {
                IdType id;
                ds >> id;
                if (id) _bottom_connections.insert(reinterpret_cast<NeuroNetworkItem *>(id));
            }
        }
    }

    void NeuroGridItem::idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap)
    {
        SubNetworkItem::idsToPointers(idMap);

        QSet<NeuroNetworkItem *> toAdd;

        foreach (NeuroNetworkItem *ni, _top_connections)
        {
            IdType wanted_id = reinterpret_cast<IdType>(ni);
            NeuroNetworkItem *wanted_item = dynamic_cast<NeuroNetworkItem *>(idMap[wanted_id]);

            if (wanted_item)
                toAdd.insert(wanted_item);
            else
                throw Common::FileFormatError(tr("Dangling top connection node in file: %1").arg(wanted_id));
        }

        _top_connections = toAdd;

        toAdd.clear();
        foreach (NeuroNetworkItem *ni, _bottom_connections)
        {
            IdType wanted_id = reinterpret_cast<IdType>(ni);
            NeuroNetworkItem *wanted_item = dynamic_cast<NeuroNetworkItem *>(idMap[wanted_id]);

            if (wanted_item)
                toAdd.insert(wanted_item);
            else
                throw Common::FileFormatError(tr("Dangling top connection node in file: %1").arg(wanted_id));
        }

        _bottom_connections = toAdd;
    }

} // namespace GridItems
