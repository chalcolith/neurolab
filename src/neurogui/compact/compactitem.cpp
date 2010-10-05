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

#include "compactitem.h"
#include "../labnetwork.h"

using namespace NeuroLib;

namespace NeuroGui
{

    CompactItem::CompactItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroItem(network, scenePos, context),
        _upward_output_property(this, &CompactItem::upwardOutputValue, &CompactItem::setUpwardOutputValue, tr("Upward Output Value")),
        _downward_output_property(this, &CompactItem::downwardOutputValue, &CompactItem::setDownwardOutputValue, tr("Downward Output Value"))
    {
        _upward_cells.append(-1);
        _downward_cells.append(-1);
    }

    CompactItem::~CompactItem()
    {
        if (_ui_delete && network() && network()->neuronet())
        {
            for (QListIterator<NeuroCell::NeuroIndex> i(_upward_cells); i.hasNext(); )
                network()->neuronet()->removeNode(i.next());

            for (QListIterator<NeuroCell::NeuroIndex> i(_downward_cells); i.hasNext(); )
                network()->neuronet()->removeNode(i.next());
        }
    }

    NeuroCell::NeuroValue CompactItem::outputValue(const QList<NeuroCell::NeuroIndex> & cells) const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell(cells.last());
        return cell ? cell->current().outputValue() : 0;
    }

    void CompactItem::setOutputValue(QList<NeuroLib::NeuroCell::NeuroIndex> &cells, const NeuroLib::NeuroCell::NeuroValue &value)
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

    bool CompactItem::addIncoming(NeuroItem *linkItem)
    {
        return NeuroItem::addIncoming(linkItem) && NeuroItem::addOutgoing(linkItem);
    }

    bool CompactItem::removeIncoming(NeuroItem *linkItem)
    {
        return NeuroItem::removeIncoming(linkItem) && NeuroItem::removeOutgoing(linkItem);
    }

    bool CompactItem::addOutgoing(NeuroItem *linkItem)
    {
        return NeuroItem::addOutgoing(linkItem) && NeuroItem::addIncoming(linkItem);
    }

    bool CompactItem::removeOutgoing(NeuroItem *linkItem)
    {
        return NeuroItem::removeOutgoing(linkItem) && NeuroItem::removeIncoming(linkItem);
    }

    const NeuroNet::ASYNC_STATE *CompactItem::getCell(const NeuroLib::NeuroCell::NeuroIndex &index) const
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        return index != -1 ? &((*network()->neuronet())[index]) : 0;
    }

    NeuroNet::ASYNC_STATE *CompactItem::getCell(const NeuroLib::NeuroCell::NeuroIndex &index)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        return index != -1 ? &((*network()->neuronet())[index]) : 0;
    }

    void CompactItem::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        NeuroItem::writeBinary(ds, file_version);

        quint32 num = _upward_cells.size();
        ds << num;
        for (quint32 i = 0; i < num; ++i)
        {
            quint32 index = _upward_cells[i];
            ds << index;
        }

        num = _downward_cells.size();
        ds << num;
        for (quint32 i = 0; i < num; ++i)
        {
            quint32 index = _downward_cells[i];
            ds << index;
        }
    }

    void CompactItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        NeuroItem::readBinary(ds, file_version);

        quint32 num;

        ds >> num;
        _upward_cells.clear();
        for (quint32 i = 0; i < num; ++i)
        {
            quint32 index;
            ds >> index;
            _upward_cells.append(static_cast<NeuroCell::NeuroIndex>(index));
        }

        ds >> num;
        _downward_cells.clear();
        for (quint32 i = 0; i < num; ++i)
        {
            quint32 index;
            ds >> index;
            _downward_cells.append(static_cast<NeuroCell::NeuroIndex>(index));
        }
    }

} // namespace NeuroGuid
