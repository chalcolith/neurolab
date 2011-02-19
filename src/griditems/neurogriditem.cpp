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
          _num_horiz(1), _num_vert(1)
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

    void NeuroGridItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroItem::addToShape(drawPath, texts);

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
        if (typeName.contains("GridConnection"))
            return true;

        return false;
    }

    bool NeuroGridItem::canBeAttachedBy(const QPointF &, NeuroItem *)
    {
        return false;
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

} // namespace GridItems
