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
#include "neurolinkitem.h"
#include "labnetwork.h"
#include "labscene.h"

#include <QVector2D>
#include <QApplication>
#include <QMenu>

#include <QtVariantProperty>

#include <cmath>

using namespace NeuroLib;

namespace NeuroLab
{

    NeuroNodeItemBase::NeuroNodeItemBase(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroNarrowItem(network, scenePos, context)
    {
        Q_ASSERT(network != 0 && network->neuronet() != 0);
        setRect(QRectF(-NODE_WIDTH/2, -NODE_WIDTH/2, NODE_WIDTH, NODE_WIDTH));
    }

    NeuroNodeItemBase::~NeuroNodeItemBase()
    {
    }

    bool NeuroNodeItemBase::canCreateNewOnMe(const QString &typeName, const QPointF &) const
    {
        return typeName.indexOf("LinkItem") >= 0;
    }

    bool NeuroNodeItemBase::canBeAttachedBy(const QPointF &, NeuroItem *item)
    {
        // can only be connected to by links
        return dynamic_cast<NeuroLinkItem *>(item);
    }

    void NeuroNodeItemBase::adjustLinks()
    {
        adjustLinksAux(incoming());
        adjustLinksAux(outgoing());
    }

    void NeuroNodeItemBase::adjustLinksAux(const QList<NeuroItem *> & list)
    {
        QVector2D center(pos());

        for (QListIterator<NeuroItem *> ln(list); ln.hasNext(); ln.next())
        {
            NeuroLinkItem *link = dynamic_cast<NeuroLinkItem *>(ln.peekNext());
            if (link)
            {
                bool frontLink = link->frontLinkTarget() == this;
                bool backLink = link->backLinkTarget() == this;

                QPointF front = link->line().p2();
                QPointF back = link->line().p1();

                QVector2D toPos = QVector2D(link->pos()) - center;

                if (frontLink && backLink)
                {
                    toPos.normalize();
                    double angle = ::atan2(toPos.y(), toPos.x());
                    angle += (frontLink ? 30.0 : -30.0) * M_PI / 180.0;
                    toPos.setX(::cos(angle));
                    toPos.setY(::sin(angle));
                }
                else
                {
                    //if (toPos.x() < toPos.y())
                    //    toPos.setX(toPos.x() * 2);
                    //else
                    //    toPos.setY(toPos.y() * 2);
                    toPos.normalize();
                }

                toPos *= NeuroNarrowItem::NODE_WIDTH/2 + 2;

                QPointF *point = frontLink ? &front : &back;
                *point = (center + toPos).toPointF();

                link->setLine(back, front);
            }
        }
    }
    
    void NeuroNodeItemBase::writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const
    {
        NeuroNarrowItem::writeClipboard(ds, id_map);
        ds << _rect;
    }
    
    void NeuroNodeItemBase::readClipboard(QDataStream &ds, const QMap<int, NeuroItem *> &id_map)
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

        if (file_version.neurolab_version >= NeuroLab::NEUROLAB_FILE_VERSION_OLD)
        {
            ds >> _rect;
        }
        else
        {
            throw new Automata::FileFormatError();
        }
    }


    //////////////////////////////////////////////////////////////////

    NEUROITEM_DEFINE_CREATOR(NeuroNodeItem, QObject::tr("Narrow|Node"));

    NeuroNodeItem::NeuroNodeItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroNodeItemBase(network, scenePos, context),
        _frozen_property(this, &NeuroNodeItem::frozen, &NeuroNodeItem::setFrozen, tr("Frozen")),
        _inputs_property(this, &NeuroNodeItem::inputs, &NeuroNodeItem::setInputs,
                         tr("Inputs"), tr("How many inputs in general will it take to activate the node.")),
        _run_property(this, &NeuroNodeItem::run, &NeuroNodeItem::setRun,
                      tr("1 / Slope"), tr("The range of input values between an output of zero and one.  Conceptually, the inverse of the slope.  Don't set this to zero."))
    {
        if (context == CREATE_UI)
        {
            NeuroCell::NeuroIndex index = network->neuronet()->addNode(NeuroCell(NeuroCell::NODE));
            setCellIndex(index);
        }
    }

    NeuroNodeItem::~NeuroNodeItem()
    {
    }

    bool NeuroNodeItem::frozen() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell();
        return cell ? cell->current().frozen() : false;
    }

    void NeuroNodeItem::setFrozen(const bool & frozen)
    {
        NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            cell->current().setFrozen(frozen);
            cell->former().setFrozen(frozen);
        }
    }

    NeuroCell::NeuroValue NeuroNodeItem::inputs() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell();
        return cell ? cell->current().weight() : 0;
    }

    void NeuroNodeItem::setInputs(const NeuroLib::NeuroCell::NeuroValue & inputs)
    {
        NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            cell->current().setWeight(inputs);
            cell->former().setWeight(inputs);
        }
    }

    NeuroCell::NeuroValue NeuroNodeItem::run() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell();
        return cell ? cell->current().run() : 0;
    }

    void NeuroNodeItem::setRun(const NeuroLib::NeuroCell::NeuroValue & run)
    {
        NeuroNet::ASYNC_STATE *cell = getCell();
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

        const NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            texts.append(TextPathRec(QPointF(-4, 4), QString::number(cell->current().weight())));
        }
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
        if (!scene())
            return;

        for (QListIterator<QGraphicsItem *> i(scene()->selectedItems()); i.hasNext(); i.next())
        {
            NeuroNodeItem *item = dynamic_cast<NeuroNodeItem *>(i.peekNext());
            if (item)
            {
                NeuroNet::ASYNC_STATE *cell = item->getCell();

                if (cell)
                {
                    NeuroCell::NeuroValue val = 0;

                    if (qAbs(cell->current().outputValue()) < 0.1f)
                        val = 1;

                    if (cell->current().weight() < 0)
                        val *= -1;

                    cell->current().setOutputValue(val);
                    cell->former().setOutputValue(val);

                    item->update();
                }
            }
        }
    }

    void NeuroNodeItem::toggleFrozen()
    {
        if (!scene())
            return;

        for (QListIterator<QGraphicsItem *> i(scene()->selectedItems()); i.hasNext(); i.next())
        {
            NeuroNodeItem *item = dynamic_cast<NeuroNodeItem *>(i.peekNext());
            if (item)
            {
                NeuroNet::ASYNC_STATE *cell = item->getCell();
                if (cell)
                {
                    bool val = !cell->current().frozen();

                    cell->current().setFrozen(val);
                    cell->former().setFrozen(val);

                    item->update();
                }
            }
        }
    }


    //////////////////////////////////////////////////////////////////

    NEUROITEM_DEFINE_CREATOR(NeuroOscillatorItem, QObject::tr("Narrow|Oscillator"));

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
            NeuroCell::NeuroIndex index = network->neuronet()->addNode(cell);
            setCellIndex(index);
        }
    }

    NeuroOscillatorItem::~NeuroOscillatorItem()
    {
    }

    NeuroCell::NeuroStep NeuroOscillatorItem::phase() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell();
        return cell ? cell->current().phase() : 0;
    }

    void NeuroOscillatorItem::setPhase(const NeuroLib::NeuroCell::NeuroStep & phase)
    {
        NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            cell->current().setPhase(phase);
            cell->former().setPhase(phase);

            reset();
        }
    }

    NeuroCell::NeuroStep NeuroOscillatorItem::peak() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell();
        return cell ? cell->current().peak() : 0;
    }

    void NeuroOscillatorItem::setPeak(const NeuroLib::NeuroCell::NeuroStep & peak)
    {
        NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            cell->current().setPeak(peak);
            cell->former().setPeak(peak);

            reset();
        }
    }

    NeuroCell::NeuroStep NeuroOscillatorItem::gap() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell();
        return cell ? cell->current().gap() : 0;
    }

    void NeuroOscillatorItem::setGap(const NeuroLib::NeuroCell::NeuroStep & gap)
    {
        NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            cell->current().setGap(gap);
            cell->former().setGap(gap);

            reset();
        }
    }

    void NeuroOscillatorItem::reset()
    {
        NeuroNet::ASYNC_STATE *cell = getCell();
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

        const NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            texts.append(TextPathRec(QPointF(-8, 4), QString("%1/%2").arg(cell->current().peak()).arg(cell->current().gap())));
        }
    }

} // namespace NeuroLab
