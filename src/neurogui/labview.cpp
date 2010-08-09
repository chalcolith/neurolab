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

#include "labview.h"
#include "labscene.h"
#include "neuroitem.h"

namespace NeuroLab
{

    LabView::LabView(LabScene *scene, QWidget *parent)
        : QGraphicsView(scene, parent), _zoom(100)
    {
        setSceneRect(0, 0, 10000, 10000);
        setAlignment(Qt::AlignLeft | Qt::AlignTop);
        setDragMode(QGraphicsView::RubberBandDrag);
        setResizeAnchor(QGraphicsView::AnchorViewCenter);

        setToolTip(tr("Right-click to add new network items."));
    }

    LabView::~LabView()
    {
    }

    void LabView::setZoom(int zoom)
    {
        if (zoom == _zoom)
            return;

        long double prev_zoom = static_cast<long double>(_zoom);
        long double new_zoom = static_cast<long double>(zoom);
        long double long_ratio = new_zoom / prev_zoom;

        qreal ratio = static_cast<qreal>(long_ratio);
        scale(ratio, ratio);
        _zoom = zoom;
    }

    void LabView::updateItemProperties()
    {
        QRect rect = viewport()->rect();

        QList<QGraphicsItem *> in_view = items(rect);
        for (QMutableListIterator<QGraphicsItem *> i(in_view); i.hasNext(); )
        {
            NeuroItem *item = dynamic_cast<NeuroItem *>(i.next());
            if (item)
                item->updateProperties();
        }
    }

    void LabView::writeBinary(QDataStream & ds, const NeuroLabFileVersion &) const
    {
        ds << static_cast<quint32>(_zoom);
        ds << transform();
    }

    void LabView::readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version)
    {
        if (file_version.neurolab_version >= NeuroLab::NEUROLAB_FILE_VERSION_2)
        {
            quint32 zoom;
            ds >> zoom;
            _zoom = zoom;

            QTransform transform;
            ds >> transform;
            setTransform(transform);
        }
    }

} // namespace NeuroLab
