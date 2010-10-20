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

#include "neuronodeitem.h"
#include "../mixins/mixinarrow.h"
#include "../labnetwork.h"
#include "../labscene.h"

#include <QVector2D>
#include <QApplication>
#include <QMenu>

#include <QtVariantProperty>

#include <cmath>

using namespace NeuroLib;

namespace NeuroGui
{

    NeuroNodeItemBase::NeuroNodeItemBase(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroNarrowItem(network, scenePos, context), MixinRemember(this)
    {
        Q_ASSERT(network != 0 && network->neuronet() != 0);
        setRect(QRectF(-NODE_WIDTH/2, -NODE_WIDTH/2, NODE_WIDTH, NODE_WIDTH));
    }

    NeuroNodeItemBase::~NeuroNodeItemBase()
    {
    }

    bool NeuroNodeItemBase::canCreateNewOnMe(const QString & typeName, const QPointF &) const
    {
        return typeName.indexOf("LinkItem") >= 0;
    }

    bool NeuroNodeItemBase::canBeAttachedBy(const QPointF &, NeuroItem *)
    {
        return true;
    }

    void NeuroNodeItemBase::onAttachedBy(NeuroItem *item)
    {
        NeuroNarrowItem::onAttachedBy(item);

        // remember
        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        if (link)
            MixinRemember::onAttachedBy(link);

        // connect
        addEdges(item);
    }

    void NeuroNodeItemBase::onDetach(NeuroItem *item)
    {
        // disconnect automaton
        removeEdges(item);

        // disremember
        MixinArrow *link = dynamic_cast<MixinArrow *>(item);
        if (link)
            MixinRemember::onDetach(link);

        // special case for self-links
        if (!link || !(link->frontLinkTarget() == this && link->backLinkTarget() == this))
            NeuroNarrowItem::onDetach(item);
    }

    void NeuroNodeItemBase::adjustLinks()
    {
        MixinRemember::adjustLinks();
    }

    QVector2D NeuroNodeItemBase::getAttachPos(const QVector2D & pos)
    {
        return pos.normalized() * (NeuroNarrowItem::NODE_WIDTH/2.0 + 2.0);
    }

    void NeuroNodeItemBase::setBrushProperties(QBrush &brush) const
    {
        NeuroNarrowItem::setBrushProperties(brush);
        brush.setStyle(Qt::SolidPattern);
    }

    void NeuroNodeItemBase::postLoad()
    {
        QVector2D center(scenePos());
        rememberItems(connections(), center);
    }

    void NeuroNodeItemBase::writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const
    {
        NeuroNarrowItem::writeClipboard(ds, id_map);
        ds << _rect;
    }

    void NeuroNodeItemBase::readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> & id_map)
    {
        NeuroNarrowItem::readClipboard(ds, id_map);
        ds >> _rect;
    }

    void NeuroNodeItemBase::writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const
    {
        NeuroNarrowItem::writeBinary(ds, file_version);
        ds << _rect;
    }

    void NeuroNodeItemBase::readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version)
    {
        NeuroNarrowItem::readBinary(ds, file_version);

        //if (file_version.neurolab_version >= NeuroGui::NEUROLAB_FILE_VERSION_OLD)
        {
            ds >> _rect;
        }
    }


    //////////////////////////////////////////////////////////////////

    NEUROITEM_DEFINE_CREATOR(NeuroNodeItem, QObject::tr("Narrow"), QObject::tr("Node"));

    NeuroNodeItem::NeuroNodeItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroNodeItemBase(network, scenePos, context),
        _frozen_property(this, &NeuroNodeItem::frozen, &NeuroNodeItem::setFrozen, tr("Frozen")),
        _inputs_property(this, &NeuroNodeItem::inputs, &NeuroNodeItem::setInputs,
                         tr("Input Threshold"), tr("How large an input signal will it take to fully activate the node.")),
        _run_property(this, &NeuroNodeItem::run, &NeuroNodeItem::setRun,
                      tr("1 / Slope"), tr("The range of input values between an output of zero and one.  Conceptually, the inverse of the slope.  Don't set this to zero."))
    {
        if (context == CREATE_UI)
        {
            NeuroCell::Index index = network->neuronet()->addNode(NeuroCell(NeuroCell::NODE));
            _cellIndices.clear();
            _cellIndices.append(index);
        }
    }

    NeuroNodeItem::~NeuroNodeItem()
    {
    }

    bool NeuroNodeItem::frozen() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
        return cell ? cell->current().frozen() : false;
    }

    void NeuroNodeItem::setFrozen(const bool & frozen)
    {
        NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
        if (cell)
        {
            cell->current().setFrozen(frozen);
            cell->former().setFrozen(frozen);
        }
    }

    NeuroCell::Value NeuroNodeItem::inputs() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
        return cell ? cell->current().weight() : 0;
    }

    void NeuroNodeItem::setInputs(const NeuroLib::NeuroCell::Value & inputs)
    {
        NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
        if (cell)
        {
            cell->current().setWeight(inputs);
            cell->former().setWeight(inputs);
        }
    }

    NeuroCell::Value NeuroNodeItem::run() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
        return cell ? cell->current().run() : 0;
    }

    void NeuroNodeItem::setRun(const NeuroLib::NeuroCell::Value & run)
    {
        NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
        if (cell)
        {
            cell->current().setRun(run);
            cell->former().setRun(run);
        }
    }

    void NeuroNodeItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroNarrowItem::addToShape(drawPath, texts);

        drawPath.addEllipse(rect());
        texts.append(TextPathRec(QPointF(-4, 4), QString::number(inputs())));
    }

    void NeuroNodeItem::setPenProperties(QPen &pen) const
    {
        NeuroNodeItemBase::setPenProperties(pen);

        if (frozen())
            pen.setColor(lerp(pen.color(), Qt::gray, 0.5f));
    }

    void NeuroNodeItem::buildActionMenu(LabScene *, const QPointF &, QMenu & menu)
    {
        menu.addAction(tr("Activate/Deactivate"), this, SLOT(toggleActivated()));
        menu.addAction(tr("Freeze/Unfreeze"), this, SLOT(toggleFrozen()));
    }

    void NeuroNodeItem::reset()
    {
        if (!frozen())
        {
            setOutputValue(0);
            updateProperties();
        }
    }

    void NeuroNodeItem::toggleActivated()
    {
        LabScene *sc = dynamic_cast<LabScene *>(scene());
        Q_ASSERT(sc);

        QList<QGraphicsItem *> items = sc->selectedItems();
        if (sc->itemUnderMouse() && !items.contains(sc->itemUnderMouse()))
            items.append(sc->itemUnderMouse());

        foreach (QGraphicsItem *gi, items)
        {
            NeuroNodeItem *item = dynamic_cast<NeuroNodeItem *>(gi);
            if (item)
            {
                NeuroNet::ASYNC_STATE *cell = item->getCell(_cellIndices.last());

                if (cell)
                {
                    NeuroCell::Value val = 0;

                    if (qAbs(cell->current().outputValue()) < 0.01f)
                        val = 1;

                    if (cell->current().weight() < 0)
                        val *= -1;

                    cell->current().setOutputValue(val);
                    cell->former().setOutputValue(val);

                    item->updateProperties();
                }
            }
        }

        sc->network()->setChanged();
    }

    void NeuroNodeItem::toggleFrozen()
    {
        Q_ASSERT(scene());

        foreach (QGraphicsItem *gi, scene()->selectedItems())
        {
            NeuroNodeItem *item = dynamic_cast<NeuroNodeItem *>(gi);
            if (item)
            {
                NeuroNet::ASYNC_STATE *cell = item->getCell(_cellIndices.last());
                if (cell)
                {
                    bool val = !cell->current().frozen();

                    cell->current().setFrozen(val);
                    cell->former().setFrozen(val);

                    item->updateProperties();
                }
            }
        }
    }


    //////////////////////////////////////////////////////////////////

    NEUROITEM_DEFINE_CREATOR(NeuroOscillatorItem, QObject::tr("Narrow"), QObject::tr("Oscillator"));

    NeuroOscillatorItem::NeuroOscillatorItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroNodeItemBase(network, scenePos, context),
        _phase_property(this, &NeuroOscillatorItem::phase, &NeuroOscillatorItem::setPhase,
                        tr("Phase"), tr("The number of steps before the oscillator will begin firing.")),
        _peak_property(this, &NeuroOscillatorItem::peak, &NeuroOscillatorItem::setPeak,
                       tr("Spike"), tr("The number of steps during which the oscillator will fire (have an output of 1).")),
        _gap_property(this, &NeuroOscillatorItem::gap, &NeuroOscillatorItem::setGap,
                      tr("Gap"), tr("The number of steps after the oscillator has fired that it will not fire (have an output of 0)."))
    {
        Q_ASSERT(network != 0 && network->neuronet() != 0);

        NeuroCell cell(NeuroCell::OSCILLATOR);
        cell.setGap(1);
        cell.setPeak(1);
        cell.setPhase(0);
        cell.setStep(0);
        cell.setOutputValue(1);

        if (context == CREATE_UI)
        {
            NeuroCell::Index index = network->neuronet()->addNode(cell);
            _cellIndices.clear();
            _cellIndices.append(index);
        }
    }

    NeuroOscillatorItem::~NeuroOscillatorItem()
    {
    }

    NeuroCell::Step NeuroOscillatorItem::phase() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
        return cell ? cell->current().phase() : 0;
    }

    void NeuroOscillatorItem::setPhase(const NeuroLib::NeuroCell::Step & phase)
    {
        NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
        if (cell)
        {
            cell->current().setPhase(phase);
            cell->former().setPhase(phase);

            reset();
        }
    }

    NeuroCell::Step NeuroOscillatorItem::peak() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
        return cell ? cell->current().peak() : 0;
    }

    void NeuroOscillatorItem::setPeak(const NeuroLib::NeuroCell::Step & peak)
    {
        NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
        if (cell)
        {
            cell->current().setPeak(peak);
            cell->former().setPeak(peak);

            reset();
        }
    }

    NeuroCell::Step NeuroOscillatorItem::gap() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
        return cell ? cell->current().gap() : 0;
    }

    void NeuroOscillatorItem::setGap(const NeuroLib::NeuroCell::Step & gap)
    {
        NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
        if (cell)
        {
            cell->current().setGap(gap);
            cell->former().setGap(gap);

            reset();
        }
    }

    void NeuroOscillatorItem::reset()
    {
        NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
        if (cell)
        {
            cell->current().setStep(0);
            cell->former().setStep(0);
            cell->current().setOutputValue(cell->current().phase() == 0 ? 1 : 0);
            cell->former().setOutputValue(cell->current().phase() == 0 ? 1 : 0);

            updateProperties();
        }
    }

    void NeuroOscillatorItem::addToShape(QPainterPath & drawPath, QList<TextPathRec> & texts) const
    {
        NeuroNarrowItem::addToShape(drawPath, texts);
        drawPath.addEllipse(rect());

        const NeuroNet::ASYNC_STATE *cell = getCell(_cellIndices.first());
        if (cell)
        {
            texts.append(TextPathRec(QPointF(-8, 4), QString("%1/%2").arg(cell->current().peak()).arg(cell->current().gap())));
        }
    }

} // namespace NeuroGui
