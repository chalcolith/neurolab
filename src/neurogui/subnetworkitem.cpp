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

#include "subnetworkitem.h"
#include "labtree.h"
#include "labscene.h"
#include "labnetwork.h"
#include "mainwindow.h"

namespace NeuroGui
{

    NEUROITEM_DEFINE_CREATOR(SubNetworkItem, QObject::tr("Misc|Sub-Network"));

    SubNetworkItem::SubNetworkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroItem(network, scenePos, context),
          _treeNodeIdNeeded(static_cast<quint32>(-1)), _treeNode(0)
    {

    }

    SubNetworkItem::~SubNetworkItem()
    {
        // the tree node will be deleted by the tree itself, so we don't need to delete it
    }

    void SubNetworkItem::propertyValueChanged(QtProperty *p, const QVariant & val)
    {
        if (_label_property.isPropertyFor(p) && _treeNode)
        {
            _treeNode->setLabel(val.toString());
        }

        NeuroItem::propertyValueChanged(p, val);
    }

    void SubNetworkItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroItem::addToShape(drawPath, texts);

        drawPath.addRect(-15, -10, 30, 20);
    }

    void SubNetworkItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
    {
        makeSubNetwork();

        if (_treeNode)
            MainWindow::instance()->setSubNetwork(_treeNode);
    }

    void SubNetworkItem::makeSubNetwork()
    {
        MainWindow *mainWindow = MainWindow::instance();
        Q_ASSERT(mainWindow);
        Q_ASSERT(mainWindow->currentNetwork());

        // we have an ID from the file, and we need to find it in the current network
        if (_treeNodeIdNeeded != static_cast<quint32>(-1) && mainWindow->currentNetwork())
        {
            _treeNode = mainWindow->currentNetwork()->findSubNetwork(_treeNodeIdNeeded);
        }

        // we don't have a tree node; make a new one
        if (!_treeNode && mainWindow->currentNetwork())
        {
            _treeNode = mainWindow->currentNetwork()->newSubNetwork();
        }

        // don't look for existing node anymore
        if (_treeNode)
        {
            _treeNodeIdNeeded = static_cast<quint32>(-1);

            setLabel(_treeNode->label());
            connect(_treeNode, SIGNAL(labelChanged(QString)), this, SLOT(setLabel(QString)));
        }
    }

    void SubNetworkItem::writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const
    {
        NeuroItem::writeBinary(ds, file_version);

        quint32 id_to_write = _treeNode ? _treeNode->id() : static_cast<quint32>(-1);
        ds << id_to_write;
    }

    void SubNetworkItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        NeuroItem::readBinary(ds, file_version);

        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_3)
        {
            ds >> _treeNodeIdNeeded;
        }
    }

} // namespace NeuroGui
