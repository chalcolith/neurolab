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

#include "neuronarrowitem.h"
#include "neuronodeitem.h"
#include "neurolinkitem.h"
#include "../labnetwork.h"

#include <QPen>
#include <QGraphicsScene>

#include <QtVariantProperty>

using namespace NeuroLib;

namespace NeuroGui
{

    NeuroNarrowItem::NeuroNarrowItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroNetworkItem(network, scenePos, context)
    {
        _cellIndices.append(-1);
    }

    NeuroNarrowItem::~NeuroNarrowItem()
    {
        if (_ui_delete)
        {
            Q_ASSERT(network());
            Q_ASSERT(network()->neuronet());

            foreach (NeuroCell::Index i, _cellIndices)
                network()->neuronet()->removeNode(i);
        }
    }

    NeuroCell::Value NeuroNarrowItem::outputValue() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.last());
        return cell ? cell->current().outputValue() : 0.0f;
    }

    void NeuroNarrowItem::setOutputValue(const NeuroLib::NeuroCell::Value & value)
    {
        for (int i = 0; i < _cellIndices.size(); ++i)
        {
            NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices[i]);
            if (cell)
            {
                if (i == _cellIndices.size() - 1)
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

    void NeuroNarrowItem::reset()
    {
        setOutputValue(0);
        updateProperties();
    }

    void NeuroNarrowItem::writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const
    {
        NeuroNetworkItem::writeClipboard(ds, id_map);
    }

    void NeuroNarrowItem::readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> & id_map)
    {
        NeuroNetworkItem::readClipboard(ds, id_map);
    }

    void NeuroNarrowItem::writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const
    {
        NeuroNetworkItem::writeBinary(ds, file_version);

        quint16 num_indices = static_cast<quint16>(_cellIndices.size());
        ds << num_indices;
        for (quint16 i = 0; i < num_indices; ++i)
        {
            ds << static_cast<qint32>(_cellIndices[i]);
        }
    }

    void NeuroNarrowItem::readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version)
    {
        NeuroNetworkItem::readBinary(ds, file_version);

        if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_4)
        {
            quint16 num_indices;
            ds >> num_indices;

            _cellIndices.clear();
            if (num_indices > 0)
            {
                for (quint16 i = 0; i < num_indices; ++i)
                {
                    qint32 n;
                    ds >> n;
                    _cellIndices.append(static_cast<NeuroCell::Index>(n));
                }
            }
            else
            {
                _cellIndices.append(-1);
            }
        }
        else
        {
            qint32 n;
            ds >> n;

            _cellIndices.clear();
            _cellIndices.append(static_cast<NeuroCell::Index>(n));
        }
    }

} // namespace NeuroGui
