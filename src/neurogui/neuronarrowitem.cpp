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
#include "labnetwork.h"

#include <QPen>
#include <QGraphicsScene>

#include <QtVariantProperty>

using namespace NeuroLib;

namespace NeuroLab
{

    NeuroNarrowItem::NeuroNarrowItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroItem(network, scenePos, context),
        _value_property(this, &NeuroNarrowItem::outputValue, &NeuroNarrowItem::setOutputValue,
                        tr("Output Value"), tr("The output value of the node or link, calculated from the values of its inputs in the previous step.")),
        _cellIndex(-1)
    {
    }

    NeuroNarrowItem::~NeuroNarrowItem()
    {
    }

    NeuroCell::NeuroValue NeuroNarrowItem::outputValue() const
    {
        const NeuroNet::ASYNC_STATE *cell = getCell();
        return cell ? cell->current().outputValue() : 0.0f;
    }

    void NeuroNarrowItem::setOutputValue(const NeuroLib::NeuroCell::NeuroValue & value)
    {
        NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            cell->current().setOutputValue(value);
            cell->former().setOutputValue(value);
        }
    }

    bool NeuroNarrowItem::addIncoming(NeuroItem *item)
    {
        if (!network() || !network()->neuronet())
            return false;

        NeuroNarrowItem *linkItem;

        if (NeuroItem::addIncoming(item) && (linkItem = dynamic_cast<NeuroNarrowItem *>(item)))
        {
            if (_cellIndex != -1 && linkItem->_cellIndex != -1)
                network()->neuronet()->addEdge(_cellIndex, linkItem->_cellIndex);
            return true;
        }

        return false;
    }

    bool NeuroNarrowItem::removeIncoming(NeuroItem *item)
    {
        if (!network() || !network()->neuronet())
            return false;

        NeuroNarrowItem *linkItem;

        if (NeuroItem::removeIncoming(item) && (linkItem = dynamic_cast<NeuroNarrowItem *>(item)))
        {
            if (_cellIndex != -1 && linkItem->_cellIndex != -1)
                network()->neuronet()->removeEdge(_cellIndex, linkItem->_cellIndex);
            return true;
        }

        return false;
    }

    void NeuroNarrowItem::setPenProperties(QPen & pen) const
    {
        NeuroItem::setPenProperties(pen);

        const NeuroNet::ASYNC_STATE *cell = getCell();
        if (cell)
        {
            qreal t = qBound(static_cast<qreal>(0), qAbs(static_cast<qreal>(cell->current().outputValue())), static_cast<qreal>(1));

            QColor result = lerp(NORMAL_LINE_COLOR, ACTIVE_COLOR, t);

            if (cell->current().frozen())
                pen.setColor(lerp(result, Qt::gray, 0.5f));
            else
                pen.setColor(result);
        }
    }

    const NeuroNet::ASYNC_STATE *NeuroNarrowItem::getCell() const
    {
        if (!network() || !network()->neuronet())
            return 0;

        return _cellIndex != -1 ? &((*network()->neuronet())[_cellIndex]) : 0;
    }

    NeuroNet::ASYNC_STATE *NeuroNarrowItem::getCell()
    {
        if (!network() || !network()->neuronet())
            return 0;

        return _cellIndex != -1 ? &((*network()->neuronet())[_cellIndex]) : 0;
    }

    void NeuroNarrowItem::reset()
    {
        setOutputValue(0);
        updateProperties();
    }

    void NeuroNarrowItem::writeClipboard(QDataStream &ds, const QMap<int, int> &id_map) const
    {
        NeuroItem::writeClipboard(ds, id_map);

        //ds.setVersion(QDataStream::Qt_4_6);
        // cell index is set by derived type constructor
    }

    void NeuroNarrowItem::readClipboard(QDataStream &ds, const QMap<int, int> & id_map)
    {
        NeuroItem::readClipboard(ds, id_map);
        
        //ds.setVersion(QDataStream::Qt_4_6);
        // cell index is set by derived type constructor
    }

    void NeuroNarrowItem::writeBinary(QDataStream & ds, const NeuroLabFileVersion & file_version) const
    {
        NeuroItem::writeBinary(ds, file_version);
        ds << static_cast<qint32>(_cellIndex);
    }

    void NeuroNarrowItem::readBinary(QDataStream & ds, const NeuroLabFileVersion & file_version)
    {
        NeuroItem::readBinary(ds, file_version);

        if (file_version.neurolab_version >= NeuroLab::NEUROLAB_FILE_VERSION_OLD)
        {
            qint32 n;
            ds >> n; _cellIndex = static_cast<NeuroCell::NeuroIndex>(n);
        }
        else
        {
            throw new Automata::FileFormatError();
        }
    }

} // namespace NeuroLab
