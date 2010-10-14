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

#include "neuronetworkitem.h"
#include "labnetwork.h"

using namespace NeuroLib;

namespace NeuroGui
{

    NeuroNetworkItem::NeuroNetworkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroItem(network, scenePos, context),
        _value_property(this, &NeuroNetworkItem::outputValue, &NeuroNetworkItem::setOutputValue,
                        tr("Output Value"),
                        tr("The output value of the node or link, calculated from the values of its inputs in the previous step."))
    {
    }

    NeuroNetworkItem::~NeuroNetworkItem()
    {
    }

    void NeuroNetworkItem::setPenProperties(QPen &pen) const
    {
        NeuroItem::setPenProperties(pen);

        qreal t = qBound(0.0f, qAbs(outputValue()), 1.0f);
        QColor result = lerp(NORMAL_LINE_COLOR, ACTIVE_COLOR, t);
        pen.setColor(result);
    }

    void NeuroNetworkItem::addEdges(NeuroItem *item)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        NeuroNetworkItem *netItem = dynamic_cast<NeuroNetworkItem *>(item);
        if (netItem)
        {
            NeuroCell::Index myIn = this->getIncomingCellFor(item);
            NeuroCell::Index itemOut = netItem->getOutgoingCellFor(this);

            if (myIn != -1 && itemOut != -1)
                network()->neuronet()->addEdge(myIn, itemOut);

            NeuroCell::Index myOut = this->getOutgoingCellFor(item);
            NeuroCell::Index itemIn = netItem->getIncomingCellFor(this);

            if (myOut != -1 && itemIn != -1)
                network()->neuronet()->addEdge(itemIn, myOut);
        }
    }

    void NeuroNetworkItem::removeEdges(NeuroItem *item)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        NeuroNetworkItem *netItem = dynamic_cast<NeuroNetworkItem *>(item);
        if (netItem)
        {
            NeuroCell::Index myIn = this->getIncomingCellFor(item);
            NeuroCell::Index itemOut = netItem->getOutgoingCellFor(this);

            if (myIn != -1 && itemOut != -1)
                network()->neuronet()->removeEdge(myIn, itemOut);

            NeuroCell::Index myOut = this->getOutgoingCellFor(item);
            NeuroCell::Index itemIn = netItem->getIncomingCellFor(this);

            if (myOut != -1 && itemIn != -1)
                network()->neuronet()->removeEdge(itemIn, myOut);
        }
    }

    const NeuroLib::NeuroNet::ASYNC_STATE *NeuroNetworkItem::getCell(const NeuroLib::NeuroCell::Index & index) const
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        return index != -1 ? &((*network()->neuronet())[index]) : 0;
    }

    NeuroLib::NeuroNet::ASYNC_STATE *NeuroNetworkItem::getCell(const NeuroLib::NeuroCell::Index & index)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->neuronet());

        return index != -1 ? &((*network()->neuronet())[index]) : 0;
    }

} // namespace NeuroGui
