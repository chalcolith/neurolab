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
        : NeuroItem(network, scenePos, context)
    {
    }

    CompactItem::~CompactItem()
    {
    }

    bool CompactItem::addIncoming(NeuroItem *linkItem)
    {
        return NeuroItem::addIncoming(linkItem) && NeuroItem::addOutgoing(linkItem);
    }

    void CompactItem::removeIncoming(NeuroItem *linkItem)
    {
        if (linkItem && incoming().contains(linkItem))
        {
            NeuroItem::removeIncoming(linkItem);
            NeuroItem::removeOutgoing(linkItem);
        }
    }

    bool CompactItem::addOutgoing(NeuroItem *linkItem)
    {
        return NeuroItem::addOutgoing(linkItem) && NeuroItem::addIncoming(linkItem);
    }

    void CompactItem::removeOutgoing(NeuroItem *linkItem)
    {
        if (linkItem && outgoing().contains(linkItem))
        {
            NeuroItem::removeOutgoing(linkItem);
            NeuroItem::removeIncoming(linkItem);
        }
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

} // namespace NeuroGuid
