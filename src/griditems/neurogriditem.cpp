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
#include "../neurogui/narrow/neurolinkitem.h"
#include "gridedgeitem.h"
#include "multigridioitem.h"

#include <QtAlgorithms>

#include <cmath>
#include <cfloat>

using namespace NeuroGui;

namespace GridItems
{

    const QString & VERSION()
    {
        static const QString
#include "../version.txt"
        ;

        return VERSION;
    }

    NEUROITEM_DEFINE_PLUGIN_CREATOR(NeuroGridItem, QString("Grid Items"), QObject::tr("Grid Item"), ":/griditems/icons/grid_item.png", GridItems::VERSION())

    NeuroGridItem::NeuroGridItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : SubNetworkItem(network, scenePos, context),
          _horizontal_property(this, &NeuroGridItem::horizontalCols, &NeuroGridItem::setHorizontalCols, tr("Width")),
          _vertical_property(this, &NeuroGridItem::verticalRows, &NeuroGridItem::setVerticalRows, tr("Height")),
          _num_horiz(1), _num_vert(1), _pattern_changed(true)
    {
        if (context == NeuroItem::CREATE_UI)
        {
            const int width = NODE_WIDTH*4;
            const int height = NODE_WIDTH*2;
            const int left = -width/2;
            const int top = -height/2;

            setRect(QRectF(left, top, width, height));
            setLabelPos(QPointF(rect().left() + rect().width() + 5, 0));
            treeNode()->setLabel(tr("Grid Item %1").arg(treeNode()->id()));
        }

        connect(network, SIGNAL(networkChanged()), this, SLOT(networkChanged()));
        connect(network, SIGNAL(stepClicked()), this, SLOT(networkStepClicked()));
        connect(network, SIGNAL(postStep()), this, SLOT(networkPostStep()));
        connect(network, SIGNAL(stepFinished()), this, SLOT(networkStepFinished()));
        connect(&_horizontal_property, SIGNAL(valueInBrowserChanged()), this, SLOT(propertyChanged()));
        connect(&_vertical_property, SIGNAL(valueInBrowserChanged()), this, SLOT(propertyChanged()));
    }

    NeuroGridItem::~NeuroGridItem()
    {
    }

    void NeuroGridItem::makeSubNetwork()
    {
        SubNetworkItem::makeSubNetwork();
        resizeScene();
    }

    void NeuroGridItem::propertyChanged()
    {
        _pattern_changed = true;
    }

    void NeuroGridItem::networkChanged()
    {
        _pattern_changed = true;
    }

    void NeuroGridItem::networkStepClicked()
    {
        generateGrid();
    }

    void NeuroGridItem::networkPostStep()
    {
    }

    void NeuroGridItem::copyColors()
    {
        // copy colors
        foreach (Index cell_index, _all_grid_cells)
        {
            if (_gl_line_colors.contains(cell_index))
            {
                NeuroLib::NeuroNet::ASYNC_STATE *cell = getCell(cell_index);
                if (cell)
                {
                    int color_index = _gl_line_colors[cell_index];

                    qreal t = qBound(0.0f, cell->current().outputValue(), 1.0f);
                    QColor color = lerp(NORMAL_LINE_COLOR, ACTIVE_COLOR, t);

                    // two vertices for lines
                    _gl_line_color_array[color_index++] = color.redF();
                    _gl_line_color_array[color_index++] = color.greenF();
                    _gl_line_color_array[color_index++] = color.blueF();
                    _gl_line_color_array[color_index++] = color.redF();
                    _gl_line_color_array[color_index++] = color.greenF();
                    _gl_line_color_array[color_index++] = color.blueF();
                }
            }
            else if (_gl_point_colors.contains(cell_index))
            {
                NeuroLib::NeuroNet::ASYNC_STATE *cell = getCell(cell_index);
                if (cell)
                {
                    int color_index = _gl_line_colors[cell_index];

                    qreal t = qBound(0.0f, cell->current().outputValue(), 1.0f);
                    QColor color = lerp(NORMAL_LINE_COLOR, ACTIVE_COLOR, t);

                    // two vertices for lines
                    _gl_line_color_array[color_index++] = color.redF();
                    _gl_line_color_array[color_index++] = color.greenF();
                    _gl_line_color_array[color_index++] = color.blueF();
                }
            }
        }
    }

    void NeuroGridItem::networkStepFinished()
    {
        // this gets set by networkChanged(), but since we've just finished a step,
        // we don't want to re-generate the grid unless something else changes
        _pattern_changed = false;

#ifdef DEBUG
        // dump graph
        {
            QFile file("neuronet_after_step.gv");
            if (file.open(QIODevice::WriteOnly))
            {
                QTextStream ts(&file);
                network()->neuronet()->dumpGraph(ts, true);
            }
        }
#endif
    }

    void NeuroGridItem::resizeScene()
    {
        LabView *view = network()->view();
        Q_ASSERT(view);

        QRect viewRect = view->viewport()->rect();
        QPointF topLeft = view->mapToScene(viewRect.topLeft());
        QPointF bottomRight = view->mapToScene(viewRect.bottomRight());

        QRect newSceneRect(topLeft.toPoint(), bottomRight.toPoint());

        treeNode()->scene()->setSceneRect(newSceneRect);

        emit gridChanged();
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

    void NeuroGridItem::addEdges(NeuroItem *)
    {
        // we need to adjust ALL items, not just this one!
        removeAllEdges();
        addAllEdges(0);
    }

    void NeuroGridItem::removeEdges(NeuroItem *item)
    {
        // we need to adjust ALL edges!
        removeAllEdges();
        addAllEdges(item);
    }

    struct ItemLessThan
    {
        NeuroGridItem *_gridItem;

        ItemLessThan(NeuroGridItem *gridItem)
            : _gridItem(gridItem)
        {
        }

        bool operator() (const NeuroItem *a, const NeuroItem *b)
        {
            int ax = a->scenePos().x();
            const MixinArrow *la = dynamic_cast<const MixinArrow *>(a);
            if (la)
            {
                if (la->frontLinkTarget() == _gridItem)
                    ax = la->line().p2().x();
                else if (la->backLinkTarget() == _gridItem)
                    ax = la->line().p1().x();
            }

            int bx = b->scenePos().x();
            const MixinArrow *lb = dynamic_cast<const MixinArrow *>(b);
            if (lb)
            {
                if (lb->frontLinkTarget() == _gridItem)
                    bx = lb->line().p2().x();
                else if (lb->backLinkTarget() == _gridItem)
                    bx = lb->line().p1().x();
            }

            return ax < bx;
        }
    };

    static void connect_cell_groups(NeuroLib::NeuroNet *neuronet, const QList<NeuroNetworkItem::Index> & outgoing, const QList<NeuroNetworkItem::Index> & incoming)
    {
        const int num_groups = qMin(outgoing.size(), incoming.size());
        if (num_groups < 1)
            return;

        const int out_step = outgoing.size() / num_groups;
        const int in_step = incoming.size() / num_groups;

        for (int i = 0; i < num_groups; ++i)
        {
            for (int j = 0; j < out_step; ++j)
            {
                for (int k = 0; k < in_step; ++k)
                {
                    int out_index = i * out_step + j;
                    int in_index = i * in_step + k;

                    if (out_index < outgoing.size() && in_index < incoming.size())
                        neuronet->addEdge(incoming[in_index], outgoing[out_index]);
                }
            }
        }
    }

    void NeuroGridItem::addAllEdges(NeuroItem *except)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());
        NeuroLib::NeuroNet *neuronet = network()->neuronet();

        if (connections().size() == 0)
            return;

        // get connections and sort by X coordinate
        QList<NeuroNetworkItem *> top_connected, bottom_connected;

        foreach (NeuroItem *item, connections())
        {
            if (item != except)
            {
                NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);

                if (_top_connections.contains(ni))
                    top_connected.append(ni);
                else if (_bottom_connections.contains(ni))
                    bottom_connected.append(ni);
            }
        }

        qSort(top_connected.begin(), top_connected.end(), ItemLessThan(this));
        qSort(bottom_connected.begin(), bottom_connected.end(), ItemLessThan(this));

        // get top cells
        QList<Index> top_incoming, top_outgoing;
        foreach (NeuroNetworkItem *ni, top_connected)
        {
            top_incoming.append(ni->getIncomingCellsFor(this));
            top_outgoing.append(ni->getOutgoingCellsFor(this));
        }

        connect_cell_groups(neuronet, this->_top_outgoing, top_incoming);
        connect_cell_groups(neuronet, top_outgoing, this->_top_incoming);

        // get bottom cells
        QList<Index> bottom_incoming, bottom_outgoing;
        foreach (NeuroNetworkItem *ni, bottom_connected)
        {
            bottom_incoming.append(ni->getIncomingCellsFor(this));
            bottom_outgoing.append(ni->getOutgoingCellFor(this));
        }

        connect_cell_groups(neuronet, this->_bot_outgoing, bottom_incoming);
        connect_cell_groups(neuronet, bottom_outgoing, this->_bot_incoming);
    }

    void NeuroGridItem::removeAllEdges()
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());
        NeuroLib::NeuroNet *neuronet = network()->neuronet();

        foreach (NeuroItem *item, this->connections())
        {
            NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
            if (ni && _edges.contains(ni))
            {
                QMap<Index, Index> & item_edges = _edges[ni];
                QMap<Index, Index>::const_iterator i = item_edges.constBegin(), end = item_edges.constEnd();
                while (i != end)
                {
                    neuronet->removeEdge(i.key(), i.value());
                    ++i;
                }
                _edges.remove(ni);
            }
        }
    }

    void NeuroGridItem::onEnterView()
    {
        resizeScene();

        emit gridChanged();
    }

    void NeuroGridItem::onLeaveView()
    {
        emit gridChanged();
    }

    QPointF NeuroGridItem::targetPointFor(const NeuroItem *item, bool front) const
    {
        const MixinArrow *link = dynamic_cast<const MixinArrow *>(item);
        const NeuroNetworkItem *ni = dynamic_cast<const NeuroNetworkItem *>(item);
        if (link && ni)
        {
            QPointF pos = front ? link->line().p2() : link->line().p1();

            if (_top_connections.contains(const_cast<NeuroNetworkItem *>(ni)))
                return QPointF(pos.x(), pos.y() + 20);
            else if (_bottom_connections.contains(const_cast<NeuroNetworkItem *>(ni)))
                return QPointF(pos.x(), pos.y() - 20);
        }

        return scenePos();
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
        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        MultiItem *mi = dynamic_cast<MultiItem *>(item);

        if (!link && !mi) return false;

        // only allow connections to the top or bottom
        QPointF topLeft = mapToScene(rect().topLeft());
        QPointF bottomRight = mapToScene(rect().bottomRight());

        if (pos.y() > topLeft.y() && pos.y() < bottomRight.y())
            return false;

        if (pos.y() <= topLeft.y())
        {
            foreach (NeuroNetworkItem *ni, _top_connections)
            {
                mi = dynamic_cast<MultiItem *>(ni);
                if (mi)
                    return false;
            }
        }
        else if (pos.y() >= bottomRight.y())
        {
            foreach (NeuroNetworkItem *ni, _bottom_connections)
            {
                mi = dynamic_cast<MultiItem *>(ni);
                if (mi)
                    return false;
            }
        }

        return true;
    }

    void NeuroGridItem::onSelected()
    {
        generateGrid();
        emit gridChanged();
    }

    void NeuroGridItem::onAttachedBy(NeuroItem *item)
    {
        NeuroNetworkItem::onAttachedBy(item); // we don't want any sub-connection items

        _pattern_changed = true;

        NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
        if (ni)
        {
            QPointF mousePos = network()->scene()->lastMousePos();

            bool top = mousePos.y() < scenePos().y();
            if (top)
                _top_connections.insert(ni);
            else
                _bottom_connections.insert(ni);

            MultiGridIOItem *gi = dynamic_cast<MultiGridIOItem *>(ni);
            if (gi)
                adjustIOItem(gi, top);
        }

        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        if (link)
        {
            MixinRemember::onAttachedBy(link);
        }

        emit gridChanged();
    }

    void NeuroGridItem::onDetach(NeuroItem *item)
    {
        _pattern_changed = true;

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

        emit gridChanged();
    }

    void NeuroGridItem::adjustLinks()
    {
        SubNetworkItem::adjustLinks();

        foreach (NeuroNetworkItem *item, _top_connections)
        {
            MultiGridIOItem *gi = dynamic_cast<MultiGridIOItem *>(item);
            if (gi)
                adjustIOItem(gi, true);
        }

        foreach (NeuroNetworkItem *item, _bottom_connections)
        {
            MultiGridIOItem *gi = dynamic_cast<MultiGridIOItem *>(item);
            if (gi)
                adjustIOItem(gi, false);
        }
    }

    void NeuroGridItem::adjustIOItem(MultiGridIOItem *gi, bool top)
    {
        if (top)
            gi->setPos(scenePos().x(), scenePos().y() - rect().height()/2 - gi->rect().height()/2);
        else
            gi->setPos(scenePos().x(), scenePos().y() + rect().height()/2 + gi->rect().height()/2);
    }

    static QMap<NeuroGridItem::Index, NeuroGridItem::Index> &
    get_to_copy(QVector<QMap<NeuroGridItem::Index, NeuroGridItem::Index> > & all_copies, int row, int col, int num_cols)
    {
        return all_copies[(row * num_cols) + col];
    }

    static double GL_WIDTH = 2;
    static double GL_HEIGHT = 3;

    static void set_gl_vertex(int & vertex_index, QVector<float> & vertices, QVector<float> & colors,
                              QMap<NeuroGridItem::Index, int> & color_index,
                              NeuroGridItem::Index cell_index, const QPointF & pt,
                              float min_x, float max_x, float min_y, float max_y,
                              int col, int row, int num_cols, int num_rows)
    {
        float x, y, z;

        float pat_x = (pt.x() - min_x) / (max_x - min_x);
        float pat_y = (pt.y() - min_y) / (max_y - min_y);

        if (num_cols > 1)
        {
            float cyl_phi = (((float)col + pat_x)/(float)num_cols) * M_PI * 2;

            x = ::cos(cyl_phi) * GL_WIDTH;
            z = -::sin(cyl_phi) * GL_WIDTH;
        }
        else
        {
            x = pat_x * GL_WIDTH - GL_WIDTH/2;
            z = 0;
        }

        y = GL_HEIGHT/2 - (((float)row + pat_y)/(float)num_rows) * GL_HEIGHT;

        if (cell_index != -1)
            color_index[cell_index] = vertex_index;

        vertices[vertex_index + 0] = x;
        vertices[vertex_index + 1] = y;
        vertices[vertex_index + 2] = z;

        colors[vertex_index + 0] = 0;
        colors[vertex_index + 1] = 0;
        colors[vertex_index + 2] = 0;

        vertex_index += 3;
    }

    void NeuroGridItem::generateGrid()
    {
        if (!_pattern_changed)
            return;

        // get neuronet pointer
        NeuroLib::NeuroNet *neuronet = 0;

        if (network())
            neuronet = network()->neuronet();

        if (!neuronet)
            return;

        // set status; this might take a while
        MainWindow::instance()->setStatus(tr("Generating neural network grid..."));
        QApplication::setOverrideCursor(Qt::WaitCursor);

        // clear outside edges
        removeAllEdges();

        // delete all existing used cells from the automaton (this will also delete edges)
        foreach (const Index & index, _all_grid_cells)
        {
            neuronet->removeNode(index);
        }
        _all_grid_cells.clear();

        // get all cells used by all items
        QSet<Index> all_pattern_cells;

        typedef QPair<QList<Index>, QList<Index> > ConnectSets;

        QList<ConnectSets> top_outgoing_to_bottom_incoming;
        QList<ConnectSets> bottom_outgoing_to_top_incoming;
        QList<ConnectSets> left_outgoing_to_right_incoming;
        QList<ConnectSets> right_outgoing_to_left_incoming;

        // map pattern cells to 2d pattern positions
        QMap<Index, QLineF> pattern_cells_to_lines;
        QMap<Index, QPointF> pattern_cells_to_points;

        // collect all cells in the pattern and cells on the edges
        bool hasLeftEdge = false, hasRightEdge = false, hasTopEdge = false, hasBottomEdge = false;

        foreach (QGraphicsItem *item, treeNode()->scene()->items())
        {
            NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
            if (ni)
            {
                QVector2D back, front;
                MixinArrow *link = dynamic_cast<MixinArrow *>(item);
                if (link)
                {
                    QLineF line = link->line();

                    back = QVector2D(line.p1());
                    front = QVector2D(line.p2());
                }

                const int num_cells = ni->allCells().size();

                int i = 0;
                foreach (Index index, ni->allCells())
                {
                    if (index != -1)
                    {
                        all_pattern_cells.insert(index);

                        if (link)
                        {
                            QVector2D my_back = back + (front - back) * ((double)i / (double)num_cells);
                            QVector2D my_front = back + (front - back) * ((double)(i+1) / (double)num_cells);

                            pattern_cells_to_lines[index] = QLineF(my_back.toPointF(), my_front.toPointF());
                        }
                        else
                        {
                            pattern_cells_to_points[index] = item->scenePos();
                        }
                    }

                    ++i;
                }
            }

            GridEdgeItem *ei = dynamic_cast<GridEdgeItem *>(item);
            if (ei)
            {
                top_outgoing_to_bottom_incoming.append(ConnectSets());
                bottom_outgoing_to_top_incoming.append(ConnectSets());
                left_outgoing_to_right_incoming.append(ConnectSets());
                right_outgoing_to_left_incoming.append(ConnectSets());

                foreach (NeuroItem *cx, ei->connections())
                {
                    NeuroNetworkItem *cxni = dynamic_cast<NeuroNetworkItem *>(cx);
                    if (cxni)
                    {
                        NeuroLinkItem *cl = dynamic_cast<NeuroLinkItem *>(cx);

                        if (ei->isVertical())
                        {
                            if (ei->isConnectedToTop(cxni))
                            {
                                hasTopEdge = true;

                                if (!cl || cl->line().p2().y() < cl->line().p1().y())
                                    foreach (const Index & idx, cxni->getOutgoingCellsFor(ei))
                                        top_outgoing_to_bottom_incoming.back().first.append(idx);
                                if (!cl || cl->line().p2().y() > cl->line().p1().y())
                                    foreach (const Index & idx, cxni->getIncomingCellsFor(ei))
                                        bottom_outgoing_to_top_incoming.back().second.append(idx);
                            }
                            if (ei->isConnectedToBottom(cxni))
                            {
                                hasBottomEdge = true;

                                if (!cl || cl->line().p2().y() > cl->line().p1().y())
                                    foreach (const Index & idx, cxni->getOutgoingCellsFor(ei))
                                        bottom_outgoing_to_top_incoming.back().first.append(idx);
                                if (!cl || cl->line().p2().y() < cl->line().p1().y())
                                    foreach (const Index & idx, cxni->getIncomingCellsFor(ei))
                                        top_outgoing_to_bottom_incoming.back().second.append(idx);
                            }
                        }
                        else
                        {
                            if (ei->isConnectedToLeft(cxni))
                            {
                                hasLeftEdge = true;

                                if (!cl || cl->line().p2().x() < cl->line().p1().x())
                                    foreach (const Index & idx, cxni->getOutgoingCellsFor(ei))
                                        left_outgoing_to_right_incoming.back().first.append(idx);
                                if (!cl || cl->line().p2().x() > cl->line().p1().x())
                                    foreach (const Index & idx, cxni->getIncomingCellsFor(ei))
                                        right_outgoing_to_left_incoming.back().second.append(idx);
                            }
                            if (ei->isConnectedToRight(cxni))
                            {
                                hasRightEdge = true;

                                if (!cl || cl->line().p2().x() > cl->line().p1().x())
                                    foreach (const Index & idx, cxni->getOutgoingCellsFor(ei))
                                        right_outgoing_to_left_incoming.back().first.append(idx);
                                if (!cl || cl->line().p2().x() < cl->line().p1().x())
                                    foreach (const Index & idx, cxni->getIncomingCellsFor(ei))
                                        left_outgoing_to_right_incoming.back().second.append(idx);
                            }
                        }
                    }
                }
            }
        }

        // get min and max extents of pattern positions
        float min_x = FLT_MAX, max_x = -min_x;
        float min_y = FLT_MAX, max_y = -min_y;

        foreach (QLineF line, pattern_cells_to_lines)
        {
            QPointF back = line.p1();
            min_x = qMin(min_x, (float)back.x());
            max_x = qMax(max_x, (float)back.x());
            min_y = qMin(min_y, (float)back.y());
            max_y = qMax(max_y, (float)back.y());

            QPointF front = line.p2();
            min_x = qMin(min_x, (float)front.x());
            max_x = qMax(max_x, (float)front.x());
            min_y = qMin(min_y, (float)front.y());
            max_y = qMax(max_y, (float)front.y());
        }

        foreach (QPointF pt, pattern_cells_to_points)
        {
            min_x = qMin(min_x, (float)pt.x());
            max_x = qMax(max_x, (float)pt.x());
            min_y = qMin(min_y, (float)pt.y());
            max_y = qMax(max_y, (float)pt.y());
        }

        if ((max_x - min_x) < 1)
        {
            max_x = min_x + 1;
            min_x = max_x - 2;
        }
        if ((max_y - min_y) < 1)
        {
            max_y = min_y + 1;
            min_y = max_y - 2;
        }

        if (!hasLeftEdge)
            min_x -= 20;
        if (!hasRightEdge)
            max_x += 20;
        if (!hasTopEdge)
            min_y -= 20;
        if (!hasBottomEdge)
            max_y += 20;

        // make copies of the pattern for each grid square
        _gl_line_array.resize(_num_vert * _num_horiz * pattern_cells_to_lines.size() * 6);
        _gl_line_color_array.resize(_num_vert * _num_horiz * pattern_cells_to_lines.size() * 6);
        _gl_point_array.resize(_num_vert * _num_horiz * pattern_cells_to_points.size() * 3);
        _gl_point_color_array.resize(_num_vert * _num_horiz * pattern_cells_to_points.size() * 3);

        int line_index = 0;
        int point_index = 0;

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

                        // we want the overall height to be 4, and the width to be 2
                        if (pattern_cells_to_lines.contains(pat_index))
                        {
                            const QLineF ln = pattern_cells_to_lines[pat_index];
                            set_gl_vertex(line_index, _gl_line_array, _gl_line_color_array,
                                          _gl_line_colors, copy_index, ln.p1(),
                                          min_x, max_x, min_y, max_y, col, row, _num_horiz, _num_vert);
                            set_gl_vertex(line_index, _gl_line_array, _gl_line_color_array,
                                          _gl_line_colors, -1, ln.p2(),
                                          min_x, max_x, min_y, max_y, col, row, _num_horiz, _num_vert);
                        }
                        else if (pattern_cells_to_points.contains(pat_index))
                        {
                            const QPointF pt = pattern_cells_to_points[pat_index];
                            set_gl_vertex(point_index, _gl_point_array, _gl_point_color_array,
                                          _gl_point_colors, copy_index, pt,
                                          min_x, max_x, min_y, max_y, col, row, _num_horiz, _num_vert);
                        }
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

                    const QVector<Index> & pat_neighbors = neuronet->neighbors(pat_index);
                    foreach (const Index & nbr_idx, pat_neighbors)
                    {
                        if (pat_to_copy.contains(nbr_idx))
                        {
                            const Index & copy_neighbor = pat_to_copy[nbr_idx];
                            neuronet->addEdge(copy_index, copy_neighbor);
                        }
                    }
                }
            }
        }

        // now connect up all the copies, connecting the top row and bottom row to the outside...
        // remember that neighbors in the neuronet are incoming cells, so the edges seem backwards!
        QList<Index> top_incoming_temp;
        QList<Index> top_outgoing_temp;
        QList<Index> bot_incoming_temp;
        QList<Index> bot_outgoing_temp;

        for (int row = 0; row < _num_vert; ++row)
        {
            for (int col = 0; col < _num_horiz; ++col)
            {
                const QMap<Index, Index> & pat_to_copy = get_to_copy(pattern_to_all_copies, row, col, _num_horiz);

                int left_col = (col + _num_horiz - 1) % _num_horiz;
                int right_col = (col + 1) % _num_horiz;
                int up_row = row - 1;
                int down_row = row + 1;

                // upwards
                if (row == 0)
                {
                    foreach (const ConnectSets & cnx, top_outgoing_to_bottom_incoming)
                    {
                        foreach (const Index & out, cnx.first)
                            top_outgoing_temp.append(pat_to_copy[out]);
                    }

                    foreach (const ConnectSets & cnx, bottom_outgoing_to_top_incoming)
                    {
                        foreach (const Index & in, cnx.second)
                            top_incoming_temp.append(pat_to_copy[in]);
                    }
                }
                else
                {
                    const QMap<Index, Index> & copy_above = get_to_copy(pattern_to_all_copies, up_row, col, _num_horiz);

                    foreach (const ConnectSets & cnx, top_outgoing_to_bottom_incoming)
                    {
                        foreach (const Index & out, cnx.first)
                        {
                            foreach (const Index & in, cnx.second)
                            {
                                neuronet->addEdge(copy_above[in], pat_to_copy[out]);
                            }
                        }
                    }
                }

                // downwards
                if (row == _num_vert - 1)
                {
                    foreach (const ConnectSets & cnx, bottom_outgoing_to_top_incoming)
                    {
                        foreach (const Index & out, cnx.first)
                            bot_outgoing_temp.append(pat_to_copy[out]);
                    }

                    foreach (const ConnectSets & cnx, top_outgoing_to_bottom_incoming)
                    {
                        foreach (const Index & in, cnx.second)
                            bot_incoming_temp.append(pat_to_copy[in]);
                    }
                }
                else
                {
                    const QMap<Index, Index> & copy_below = get_to_copy(pattern_to_all_copies, down_row, col, _num_horiz);

                    foreach (const ConnectSets & cnx, bottom_outgoing_to_top_incoming)
                    {
                        foreach (const Index & out, cnx.first)
                        {
                            foreach (const Index & in, cnx.second)
                            {
                                neuronet->addEdge(copy_below[in], pat_to_copy[out]);
                            }
                        }
                    }
                }

                // leftwards
                const QMap<Index, Index> & copy_on_left = get_to_copy(pattern_to_all_copies, row, left_col, _num_horiz);

                foreach (const ConnectSets & cnx, left_outgoing_to_right_incoming)
                {
                    foreach (const Index & out, cnx.first)
                    {
                        foreach (const Index & in, cnx.second)
                        {
                            neuronet->addEdge(copy_on_left[in], pat_to_copy[out]);
                        }
                    }
                }

                // rightwards
                const QMap<Index, Index> & copy_on_right = get_to_copy(pattern_to_all_copies, row, right_col, _num_horiz);

                foreach (const ConnectSets & cnx, right_outgoing_to_left_incoming)
                {
                    foreach (const Index & out, cnx.first)
                    {
                        foreach (const Index & in, cnx.second)
                        {
                            neuronet->addEdge(copy_on_right[in], pat_to_copy[out]);
                        }
                    }
                }
            }
        }

        _top_incoming = top_incoming_temp;
        _top_outgoing = top_outgoing_temp;
        _bot_incoming = bot_incoming_temp;
        _bot_outgoing = bot_outgoing_temp;

        addAllEdges(0);

        // done
        copyColors();
        QApplication::restoreOverrideCursor();
        MainWindow::instance()->setStatus(tr("Generated neural network grid."));
        _pattern_changed = false;

        emit gridChanged();

#ifdef DEBUG
        // dump graph
        {
            QFile file("neuronet_after_gen.gv");
            if (file.open(QIODevice::WriteOnly))
            {
                QTextStream ts(&file);
                neuronet->dumpGraph(ts, true);
            }
        }
#endif
    }

    void NeuroGridItem::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        const_cast<NeuroGridItem *>(this)->generateGrid();

        SubNetworkItem::writeBinary(ds, file_version);

        ds << static_cast<quint32>(_num_horiz);
        ds << static_cast<quint32>(_num_vert);

        ds << static_cast<quint32>(_edges.size());
        QMap<NeuroNetworkItem *, QMap<Index, Index> >::const_iterator i = _edges.constBegin(), i_end = _edges.constEnd();
        while (i != i_end)
        {
            if (i.key())
            {
                ds << static_cast<IdType>(i.key()->id());

                ds << static_cast<quint32>(i.value().size());
                QMap<Index, Index>::const_iterator j = i.value().constBegin(), j_end = i.value().constEnd();
                while (j != j_end)
                {
                    ds << j.key();
                    ds << j.value();
                    ++j;
                }
            }
            ++i;
        }
    }

    void NeuroGridItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        SubNetworkItem::readBinary(ds, file_version);

        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_9)
        {
            quint32 num;
            ds >> num; _num_horiz = num;
            ds >> num; _num_vert = num;

            _edges.clear();
            if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_11)
            {
                quint32 num_i;
                ds >> num_i;
                for (quint32 i = 0; i < num_i; ++i)
                {
                    IdType id;
                    ds >> id;
                    NeuroNetworkItem *id_ptr = reinterpret_cast<NeuroNetworkItem *>(id);
                    QMap<Index, Index> & edge_map = _edges[id_ptr];

                    quint32 num_j;
                    ds >> num_j;
                    for (quint32 j = 0; j < num_j; ++j)
                    {
                        Index key, val;
                        ds >> key;
                        ds >> val;
                        edge_map.insert(key, val);
                    }
                }
            }
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
            IdType wanted_id = static_cast<NeuroItem::IdType>(reinterpret_cast<quint64>(ni));
            NeuroNetworkItem *wanted_item = dynamic_cast<NeuroNetworkItem *>(idMap[wanted_id]);

            if (wanted_item)
                toAdd.insert(wanted_item);
            else
                throw Common::FileFormatError(tr("Dangling top connection node in file: %1").arg(static_cast<quint32>(wanted_id)));
        }

        _top_connections = toAdd;

        toAdd.clear();
        foreach (NeuroNetworkItem *ni, _bottom_connections)
        {
            IdType wanted_id = static_cast<NeuroItem::IdType>(reinterpret_cast<quint64>(ni));
            NeuroNetworkItem *wanted_item = dynamic_cast<NeuroNetworkItem *>(idMap[wanted_id]);

            if (wanted_item)
                toAdd.insert(wanted_item);
            else
                throw Common::FileFormatError(tr("Dangling top connection node in file: %1").arg(static_cast<quint32>(wanted_id)));
        }

        _bottom_connections = toAdd;

        // edges
        QMap<NeuroNetworkItem *, QMap<Index, Index> > new_edges;
        QMap<NeuroNetworkItem *, QMap<Index, Index> >::const_iterator i = _edges.constBegin(), end = _edges.constEnd();
        while (i != end)
        {
            IdType wanted_id = static_cast<NeuroItem::IdType>(reinterpret_cast<quint64>(i.key()));
            NeuroNetworkItem *wanted_item = dynamic_cast<NeuroNetworkItem *>(idMap[wanted_id]);
            if (wanted_item)
                new_edges[wanted_item] = i.value();
            else
                throw Common::FileFormatError(tr("Dangling edge node in file: %1").arg(static_cast<quint32>(wanted_id)));

            ++i;
        }

        _edges = new_edges;
    }

    void NeuroGridItem::postLoad()
    {
        SubNetworkItem::postLoad();
        generateGrid();
    }

} // namespace GridItems
