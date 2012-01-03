#ifndef NEUROGRIDITEM_H
#define NEUROGRIDITEM_H

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

#include "griditems_global.h"
#include "../neurogui/subnetwork/subnetworkitem.h"

#include <QVector>

namespace GridItems
{

    class MultiGridIOItem;

    class GRIDITEMSSHARED_EXPORT NeuroGridItem
        : public NeuroGui::SubNetworkItem
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

        Property<NeuroGridItem, QVariant::Int, qint32, qint32> _horizontal_property;
        Property<NeuroGridItem, QVariant::Int, qint32, qint32> _vertical_property;

        qint32 _num_horiz;
        qint32 _num_vert;

        bool _pattern_changed;

        QSet<NeuroNetworkItem *> _top_connections, _bottom_connections;

        QSet<Index> _all_grid_cells; // all automaton cells used by the grid
        QList<Index> _top_incoming, _top_outgoing; // cells to use for getIncomingCellsFor etc.
        QList<Index> _bot_incoming, _bot_outgoing;

        QMap<NeuroNetworkItem *, QMap<Index, Index> > _edges;

        QVector<float> _gl_line_array;
        QVector<float> _gl_point_array;

        QMap<Index, int> _gl_line_colors;
        QMap<Index, int> _gl_point_colors;

        QVector<float> _gl_line_color_array;
        QVector<float> _gl_point_color_array;

    public:
        NeuroGridItem(NeuroGui::LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~NeuroGridItem();

        qint32 horizontalCols() const { return _num_horiz; }
        void setHorizontalCols(const qint32 & num) { _num_horiz = num; }

        qint32 verticalRows() const { return _num_vert; }
        void setVerticalRows(const qint32 & num) { _num_vert = num; }

        const QVector<float> & glLineArray() const { return _gl_line_array; }
        const QVector<float> & glPointArray() const { return _gl_point_array; }
        const QVector<float> & glLineColorArray() const { return _gl_line_color_array; }
        const QVector<float> & glPointColorArray() const { return _gl_point_color_array; }

        virtual QList<Index> getIncomingCellsFor(const NeuroItem *item) const;
        virtual QList<Index> getOutgoingCellsFor(const NeuroItem *item) const;

        virtual void addEdges(NeuroItem *);
        virtual void removeEdges(NeuroItem *);

        virtual void adjustLinks();

    signals:
        void gridChanged();

    public slots:
        void networkChanged();
        void propertyChanged();
        void networkStepClicked();
        void networkPostStep();
        void networkStepFinished();

        void generateGrid();

        void resizeScene();
        void copyColors();

    protected:
        void startGenerateGrid(NeuroLib::NeuroNet *neuronet);
        void collectPatternCells(NeuroLib::NeuroNet *neuronet,
                                 QVector<Index> &all_pattern_cells,
                                 QVector<QMap<Index, QVector<Index> > > &pattern_connections,
                                 QVector<Index> &pattern_top_incoming, QVector<Index> &pattern_top_outgoing,
                                 QVector<Index> &pattern_bot_incoming, QVector<Index> &pattern_bot_outgoing,
                                 QMap<Index, QLineF> &pattern_cells_to_lines,
                                 QMap<Index, QPointF> &pattern_cells_to_points,
                                 bool &hasTopEdge, bool &hasBottomEdge,
                                 bool &hasLeftEdge, bool &hasRightEdge);
        void recordPatternPositions(NeuroGui::NeuroNetworkItem *ni,
                                    QMap<Index, QLineF> & pattern_cells_to_lines,
                                    QMap<Index, QPointF> & pattern_cells_to_points);
        void getMinMaxCoords(QMap<Index, QLineF> & pattern_cells_to_lines,
                             QMap<Index, QPointF> & pattern_cells_to_points,
                             float & min_x, float & max_x, float & min_y, float & max_y,
                             bool &hasTopEdge, bool &hasBottomEdge, bool &hasLeftEdge, bool &hasRightEdge);
        void makeCopies(NeuroLib::NeuroNet *neuronet,
                        QVector<Index> & all_pattern_cells,
                        QMap<Index, QLineF> & pattern_cells_to_lines,
                        QMap<Index, QPointF> & pattern_cells_to_points,
                        QVector<QMap<Index, Index> > & all_copies,
                        float min_x, float max_x, float min_y, float max_y);
        void connectCopies(NeuroLib::NeuroNet *neuronet,
                           QVector<QMap<Index, Index> > & all_copies,
                           QVector<QMap<Index, QVector<Index> > > & pattern_connections,
                           QVector<Index> & pattern_top_incoming, QVector<Index> & pattern_top_outgoing,
                           QVector<Index> & pattern_bot_incoming, QVector<Index> & pattern_bot_outgoing);
        void finishGenerateGrid(NeuroLib::NeuroNet *neuronet);

        void generateGridOld();

        virtual void onEnterView();
        virtual void onLeaveView();

        virtual void makeSubNetwork();

        virtual QPointF targetPointFor(const NeuroItem *, bool front) const;
        virtual void addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const;

        virtual bool allowZoom() const { return false; }
        virtual bool canCreateNewItem(const QString &, const QPointF &) const;
        virtual bool canBeAttachedBy(const QPointF &, NeuroItem *) const;

        virtual void onSelected();
        virtual void onAttachedBy(NeuroItem *);
        virtual void onDetach(NeuroItem *);

        virtual void writeBinary(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version) const;
        virtual void readBinary(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version);

        virtual void writePointerIds(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version) const;
        virtual void readPointerIds(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version);
        virtual void idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap);

        virtual void postLoad();

    private:
        void adjustIOItem(MultiGridIOItem *gi, bool top);

        void addAllEdges(NeuroItem *except);
        void removeAllEdges();
    };

}

#endif // NEUROGRIDITEM_H
