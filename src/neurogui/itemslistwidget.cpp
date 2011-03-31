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

#include "itemslistwidget.h"
#include "neuroitem.h"

#include <QDragMoveEvent>
#include <QApplication>

ItemsListWidget::ItemsListWidget(QWidget *parent)
    : QListWidget(parent)
{
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    this->setDragEnabled(true);
    this->setDragDropMode(QAbstractItemView::DragOnly);
}

void ItemsListWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        _dragStartPos = event->pos();
}

void ItemsListWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (!(e->buttons() & Qt::LeftButton))
        return;
    if ((e->pos() - _dragStartPos).manhattanLength() < QApplication::startDragDistance())
        return;

    QListWidgetItem *item = this->itemAt(_dragStartPos);
    if (item && (item->flags() & Qt::ItemIsEnabled))
    {
        QDrag *drag = new QDrag(this);        

        QString typeName = item->data(Qt::UserRole).toString();

        QByteArray itemData;
        QDataStream itemStream(&itemData, QIODevice::WriteOnly);
        itemStream << typeName;

        QMimeData *mimeData = new QMimeData();
        mimeData->setData("application/x-neurolab-item-dnd", itemData);

        QString pixmap = NeuroGui::NeuroItem::getPixmap(typeName);
        if (!pixmap.isEmpty())
        {
            QPixmap pm(pixmap);
            drag->setPixmap(pm);
            drag->setHotSpot(QPoint(pm.width()/2, pm.height()/2));
        }

        drag->setMimeData(mimeData);
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }
}

void ItemsListWidget::dragMoveEvent(QDragMoveEvent *e)
{
    if (e->source() == this)
        e->ignore();
    else
        e->accept();
}
