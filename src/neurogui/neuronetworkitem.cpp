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

#include "neuronetworkitem.h"
#include "narrow/neurolinkitem.h"

#include "labnetwork.h"
#include "labscene.h"

#include <QMenu>

using namespace NeuroLib;

namespace NeuroGui
{

    NeuroNetworkItem::NeuroNetworkItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroItem(network, scenePos, context),
          _frozen_property(this, &NeuroNetworkItem::frozen, &NeuroNetworkItem::setFrozen, tr("Frozen")),
          _value_property(this, &NeuroNetworkItem::outputValue, &NeuroNetworkItem::setOutputValue,
              tr("Output Value"),
              tr("The output value of the node or link, calculated from the values of its inputs in the previous step.")),
          _persist_property(this, &NeuroNetworkItem::persist, &NeuroNetworkItem::setPersist,
              tr("Persistance"),
              tr("The number of steps an item's neuro-net cells will stay active before beginning to decay."))
    {
    }

    NeuroNetworkItem::~NeuroNetworkItem()
    {
    }

    int NeuroNetworkItem::persist() const
    {
        QList<Index> cells = allCells();
        const NeuroNet::ASYNC_STATE *cell = cells.size() > 0 ? getCell(cells.first()) : 0;
        return cell ? cell->current().persist() : 1;
    }

    void NeuroNetworkItem::setPersist(const int & _p)
    {
        int p = _p;

        if (p < 1)
            p = 1;
        if (p > 15)
            p = 15;

        foreach (Index index, allCells())
        {
            NeuroNet::ASYNC_STATE *cell = getCell(index);
            if (cell)
                cell->current().setPersist(p);
        }

        _persist_property.setValueInPropertyBrowser(QVariant(p));
    }

    bool NeuroNetworkItem::frozen() const
    {
        QList<Index> cells = allCells();
        const NeuroNet::ASYNC_STATE *cell = cells.size() > 0 ? getCell(cells.first()) : 0;
        return cell ? cell->current().frozen() : false;
    }

    void NeuroNetworkItem::setFrozen(const bool & frozen)
    {
        foreach (Index index, allCells())
        {
            NeuroNet::ASYNC_STATE *cell = getCell(index);
            if (cell)
                cell->current().setFrozen(frozen);
        }

        setChanged(true);
    }

    void NeuroNetworkItem::buildActionMenu(LabScene *, const QPointF &, QMenu & menu)
    {
        menu.addAction(tr("Activate/Deactivate"), this, SLOT(toggleActivated()));
        menu.addAction(tr("Freeze/Unfreeze"), this, SLOT(toggleFrozen()));
    }

    void NeuroNetworkItem::reset()
    {
        if (!frozen())
        {
            setOutputValue(0);
            updateProperties();
            setChanged(true);
        }
    }

    void NeuroNetworkItem::cleanup()
    {
        // we need to do this before the connections are broken
        foreach (NeuroItem *ni, connections())
        {
            removeEdges(ni);
        }

        NeuroItem::cleanup();
    }

    void NeuroNetworkItem::toggleActivated()
    {
        LabScene *sc = dynamic_cast<LabScene *>(scene());
        Q_ASSERT(sc);

        QList<QGraphicsItem *> items = sc->selectedItems();
        if (sc->itemUnderMouse() && !items.contains(sc->itemUnderMouse()))
            items.append(sc->itemUnderMouse());

        foreach (QGraphicsItem *gi, items)
        {
            NeuroNetworkItem *item = dynamic_cast<NeuroNetworkItem *>(gi);
            if (item)
            {
                NeuroCell::Value val = qAbs(item->outputValue()) < 0.01f ? 1 : 0;

                if ((dynamic_cast<NeuroInhibitoryLinkItem *>(item)))
                    val = -val;

                item->setOutputValue(val);
                item->updateProperties();
                item->setChanged(true);
            }
        }
    }

    void NeuroNetworkItem::toggleFrozen()
    {
        Q_ASSERT(scene());

        foreach (QGraphicsItem *gi, scene()->selectedItems())
        {
            NeuroNetworkItem *item = dynamic_cast<NeuroNetworkItem *>(gi);
            if (item)
            {
                item->setFrozen(!item->frozen());
                item->updateProperties();
                item->setChanged(true);
            }
        }
    }

    QString NeuroNetworkItem::dataValue() const
    {
        NeuroCell::Value val = outputValue();
        if (val < NeuroCell::EPSILON)
            val = 0;

        return QString::number(val);
    }

    void NeuroNetworkItem::onDetach(NeuroItem *item)
    {
        removeEdges(item);
        NeuroItem::onDetach(item);
    }

    void NeuroNetworkItem::setPenProperties(QPen &pen) const
    {
        NeuroItem::setPenProperties(pen);

        qreal t = qBound(0.0f, qAbs(outputValue()), 1.0f);
        QColor result = lerp(NORMAL_LINE_COLOR, ACTIVE_COLOR, t);
        pen.setColor(result);
    }

    QList<NeuroNetworkItem::Index> NeuroNetworkItem::getIncomingCellsFor(const NeuroItem *item) const
    {
        _incoming_cells.clear();
        Index idx = getIncomingCellFor(item);
        if (idx != -1)
            _incoming_cells.append(idx);
        return _incoming_cells;
    }

    QList<NeuroNetworkItem::Index> NeuroNetworkItem::getOutgoingCellsFor(const NeuroItem *item) const
    {
        _outgoing_cells.clear();
        Index idx = getOutgoingCellFor(item);
        if (idx != -1)
            _outgoing_cells.append(idx);
        return _outgoing_cells;
    }

    void NeuroNetworkItem::addEdges(NeuroItem *item)
    {
        Q_ASSERT(network());

        NeuroLib::NeuroNet *neuronet = network()->neuronet();
        Q_ASSERT(neuronet);

        NeuroNetworkItem *netItem = dynamic_cast<NeuroNetworkItem *>(item);
        if (netItem)
        {
            // connect the item's outputs to my inputs
            QList<Index> myIns = this->getIncomingCellsFor(item);
            QList<Index> itemOuts = netItem->getOutgoingCellsFor(this);

            foreach (Index myIn, myIns)
            {
                foreach (Index itemOut, itemOuts)
                {
                    if (myIn != -1 && itemOut != -1)
                        neuronet->addEdge(myIn, itemOut);
                }
            }

            // connect my ouputs to the item's inputs
            QList<Index> myOuts = this->getOutgoingCellsFor(item);
            QList<Index> itemIns = netItem->getIncomingCellsFor(this);

            foreach (Index myOut, myOuts)
            {
                foreach (Index itemIn, itemIns)
                {
                    if (myOut != -1 && itemIn != -1)
                        neuronet->addEdge(itemIn, myOut);
                }
            }
        }
    }

    void NeuroNetworkItem::removeEdges(NeuroItem *item)
    {
        Q_ASSERT(network());

        NeuroLib::NeuroNet *neuronet = network()->neuronet();
        Q_ASSERT(neuronet);

        NeuroNetworkItem *netItem = dynamic_cast<NeuroNetworkItem *>(item);
        if (netItem)
        {
            QList<Index> myIns = this->getIncomingCellsFor(item);
            QList<Index> itemOuts = netItem->getOutgoingCellsFor(this);

            foreach (Index myIn, myIns)
            {
                foreach (Index itemOut, itemOuts)
                {
                    if (myIn != -1 && itemOut != -1)
                        neuronet->removeEdge(myIn, itemOut);
                }
            }

            QList<Index> myOuts = this->getOutgoingCellsFor(item);
            QList<Index> itemIns = netItem->getIncomingCellsFor(this);

            foreach (Index myOut, myOuts)
            {
                foreach (Index itemIn, itemIns)
                {
                    if (myOut != -1 && itemIn != -1)
                        network()->neuronet()->removeEdge(itemIn, myOut);
                }
            }
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
