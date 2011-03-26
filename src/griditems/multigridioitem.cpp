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

#include "multigridioitem.h"
#include "neurogriditem.h"
#include "../neurogui/labscene.h"
#include "../neurogui/labview.h"
#include "../neurogui/labtree.h"
#include "../neurogui/labnetwork.h"
#include "../neurolib/neuronet.h"

#include <QTextCodec>

using namespace NeuroGui;

namespace GridItems
{

    MultiGridIOItem::MultiGridIOItem(NeuroGui::LabNetwork *network, const QPointF &scenePos, const CreateContext &context)
        : NeuroNetworkItem(network, scenePos, context),
          _top_item(0), _bottom_item(0)
    {
        if (context == NeuroItem::CREATE_UI)
        {
            const int width = NODE_WIDTH * 4;
            const int height = NODE_WIDTH;

            setRect(QRectF(-width/2, -height/2, width, height));
            setLabelPos(QPointF(rect().left() + rect().width() + 5, 0));
        }
    }

    MultiGridIOItem::~MultiGridIOItem()
    {
    }

    bool MultiGridIOItem::handleMove(const QPointF &mousePos, QPointF &movePos)
    {
        Q_ASSERT(network());
        Q_ASSERT(network()->scene());

        // we want to connect with the whole bounding rect, not the mouse pos
        QPointF topLeft = mapToScene(rect().left(), rect().top());
        QRectF myRect(topLeft.x(), topLeft.y(), rect().width(), rect().height());

        bool attached = false;
        foreach (QGraphicsItem *item, network()->scene()->items(myRect, Qt::IntersectsItemShape, Qt::DescendingOrder))
        {
            NeuroItem *ni = dynamic_cast<NeuroItem *>(item);
            if (ni && ni != this
                && !connections().contains(ni)
                && canAttachTo(mousePos, ni) && ni->canBeAttachedBy(mousePos, this))
            {
                this->onAttachTo(ni);
                ni->onAttachedBy(this);
                movePos = scenePos();
                attached = true;
                break;
            }
        }

        // break if no intersect
        if (!attached)
        {
            if (_top_item && !_top_item->containsScenePos(mousePos))
            {
                _top_item->onDetach(this);
                onDetach(_top_item);
            }
            else if (_bottom_item && !_bottom_item->containsScenePos(mousePos))
            {
                _bottom_item->onDetach(this);
                onDetach(_bottom_item);
            }
        }

        // if we're still connected to a grid item, tell it to move us to the right position
        NeuroGridItem *gi = _top_item ? dynamic_cast<NeuroGridItem *>(_top_item) : dynamic_cast<NeuroGridItem *>(_bottom_item);
        if (gi)
        {
            gi->adjustLinks();
            movePos = scenePos();
        }

        return true;
    }

    bool MultiGridIOItem::canAttachTo(const QPointF &, NeuroItem *item) const
    {
        NeuroGridItem *gi = dynamic_cast<NeuroGridItem *>(item);
        return gi && !_top_item && !_bottom_item;
    }

    void MultiGridIOItem::onAttachTo(NeuroItem *item)
    {
        _top_item = 0;
        _bottom_item = 0;

        NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(item);
        if (ni)
        {
            if (ni->scenePos().y() < scenePos().y())
                _top_item = ni;
            else
                _bottom_item = ni;
        }
    }

    void MultiGridIOItem::onDetach(NeuroItem *)
    {
        _top_item = 0;
        _bottom_item = 0;
        updateShape();
    }

    QList<MultiGridIOItem::Index> MultiGridIOItem::getIncomingCellsFor(const NeuroItem *item) const
    {
        const NeuroNetworkItem *ni = dynamic_cast<const NeuroNetworkItem *>(item);
        if (ni && (ni == _top_item || ni == _bottom_item))
            return _incoming_cells;
        else
            return QList<Index>();
    }

    QList<MultiGridIOItem::Index> MultiGridIOItem::getOutgoingCellsFor(const NeuroItem *item) const
    {
        const NeuroNetworkItem *ni = dynamic_cast<const NeuroNetworkItem *>(item);
        if (ni && (ni == _top_item || ni == _bottom_item))
            return _outgoing_cells;
        else
            return QList<Index>();
    }

    QList<MultiGridIOItem::Index> MultiGridIOItem::allCells() const
    {
        return _incoming_cells + _outgoing_cells;
    }

    void MultiGridIOItem::addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const
    {
        NeuroNetworkItem::addToShape(drawPath, texts);
        drawPath.addRect(_rect);

        if (_top_item || _bottom_item)
        {
            qreal y = _top_item ? _rect.top() : _rect.bottom();

            const int num = 7;
            for (int i = 0; i < num; ++i)
            {
                qreal x = _rect.left() + (i+1)*(static_cast<qreal>(_rect.width()) / (num+1));
                drawPath.moveTo(x, y-3);
                drawPath.lineTo(x, y+3);
            }
        }
    }

    void MultiGridIOItem::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        NeuroNetworkItem::writeBinary(ds, file_version);
        ds << _rect;

        quint32 num = _incoming_cells.size();
        ds << num;
        foreach (const Index & idx, _incoming_cells)
            ds << idx;

        num = _outgoing_cells.size();
        ds << num;
        foreach (const Index & idx, _outgoing_cells)
            ds << idx;
    }

    void MultiGridIOItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        NeuroNetworkItem::readBinary(ds, file_version);
        ds >> _rect;
        setLabelPos(QPointF(rect().left() + rect().width() + 5, 0));

        quint32 num;
        _incoming_cells.clear();
        ds >> num;
        for (quint32 i = 0; i < num; ++i)
        {
            Index idx;
            ds >> idx;
            _incoming_cells.append(idx);
        }

        _outgoing_cells.clear();
        ds >> num;
        for (quint32 i = 0; i < num; ++i)
        {
            Index idx;
            ds >> idx;
            _outgoing_cells.append(idx);
        }
    }

    void MultiGridIOItem::writePointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        NeuroNetworkItem::writePointerIds(ds, file_version);

        IdType id = _top_item ? _top_item->id() : 0;
        ds << id;

        id = _bottom_item ? _bottom_item->id() : 0;
        ds << id;
    }

    void MultiGridIOItem::readPointerIds(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        NeuroNetworkItem::readPointerIds(ds, file_version);

        IdType id;
        ds >> id;
        _top_item = reinterpret_cast<NeuroNetworkItem *>(id);

        ds >> id;
        _bottom_item = reinterpret_cast<NeuroNetworkItem *>(id);
    }

    void MultiGridIOItem::idsToPointers(const QMap<NeuroItem::IdType, NeuroItem *> &idMap)
    {
        IdType wanted_id = reinterpret_cast<IdType>(_top_item);
        if (wanted_id != 0)
        {
            NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(idMap[wanted_id]);
            if (ni)
                _top_item = ni;
        }

        wanted_id = reinterpret_cast<IdType>(_bottom_item);
        if (wanted_id != 0)
        {
            NeuroNetworkItem *ni = dynamic_cast<NeuroNetworkItem *>(idMap[wanted_id]);
            if (ni)
                _bottom_item = ni;
        }
    }


    //

    NEUROITEM_DEFINE_PLUGIN_CREATOR(TextGridIOItem, QObject::tr("Grid Items"), QObject::tr("Text IO Item"), GridItems::VERSION)

    TextGridIOItem::TextGridIOItem(NeuroGui::LabNetwork *network, const QPointF &scenePos, const CreateContext &context)
        : MultiGridIOItem(network, scenePos, context),
          _repeat_input(true), _input_spike(1), _input_gap(0), _output_threshold(0.99),
          _repeat_property(this, &TextGridIOItem::repeatInput, &TextGridIOItem::setRepeatInput,
                           tr("Repeat Input"), tr("Input string will be repeated.")),
          _spike_property(this, &TextGridIOItem::inputSpike, &TextGridIOItem::setInputSpike,
                          tr("Input Steps"), tr("Number of steps to activate input for.")),
          _gap_property(this, &TextGridIOItem::inputGap, &TextGridIOItem::setInputGap,
                        tr("Input Gap"), tr("Number of steps between input activations.")),
          _threshold_property(this, &TextGridIOItem::outputThreshold, &TextGridIOItem::setOutputThreshold,
                              tr("Output Threshold"), tr("Output cells must be above this value to trigger output.")),
          _text_property(this, &TextGridIOItem::inputText, &TextGridIOItem::setInputText,
                         tr("Input Text"), tr("Text for input to the IO item.")),
          _input_buffer(), _input_array(), _output_buffer(), _output_buffer_stream(),
          _input_disp_buffer(10), _input_disp_pos(0), _output_disp_buffer(10), _output_disp_pos(0),
          _cur_step(0)
    {
        _incoming_cells.reserve(256);
        _outgoing_cells.reserve(256);

        if (context == NeuroItem::CREATE_UI)
        {
            NeuroLib::NeuroCell cell(NeuroLib::NeuroCell::NODE);

            for (int i = 0; i < 256; ++i)
                _incoming_cells.append(network->neuronet()->addNode(cell));
            for (int i = 0; i < 256; ++i)
                _outgoing_cells.append(network->neuronet()->addNode(cell));
        }

        _text_property.setEditable(false);
        updateInputBuffer();
        connect(&_text_property, SIGNAL(valueInBrowserChanged()), this, SLOT(updateInputBuffer()));

        _output_buffer.open(QIODevice::ReadWrite);
        _output_buffer_stream.setDevice(&_output_buffer);
        _output_buffer_stream.setCodec("UTF-8");

        connect(network, SIGNAL(preStep()), this, SLOT(networkPreStep()));
        connect(network, SIGNAL(postStep()), this, SLOT(networkPostStep()));

        for (int i = 0; i < _input_disp_buffer.size(); ++i)
            _input_disp_buffer[i] = ' ';
        for (int i = 0; i < _output_disp_buffer.size(); ++i)
            _output_disp_buffer[i] = ' ';
    }

    TextGridIOItem::~TextGridIOItem()
    {
        _input_buffer.close();
        _output_buffer.close();
    }

    QString TextGridIOItem::dataValue() const
    {
        return _step_output_text;
    }

    void TextGridIOItem::addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const
    {
        MultiGridIOItem::addToShape(drawPath, texts);

        const QVector<QChar> & v = _bottom_item ? _input_disp_buffer : _output_disp_buffer;
        const int num = v.size();
        int p = _bottom_item ? _input_disp_pos : _output_disp_pos;
        p = (p + 1) % num;

        for (int i = 0; i < num; ++i)
        {
            QChar ch = v[(p + i) % num];
            QString str(ch);

            qreal pct = static_cast<qreal>(i) / static_cast<qreal>(num);
            QColor col = lerp(QColor(0xff, 0xff, 0xff), QColor(0, 0, 0), pct);

            qreal buf = static_cast<qreal>(rect().width()) / 8.0;
            qreal w = static_cast<qreal>(rect().width()) - buf*2;
            qreal y = 4;
            qreal x = -w/2.0 + w * static_cast<qreal>(i) / static_cast<qreal>(num);

            QPen pen;
            pen.setColor(col);
            texts.append(TextPathRec(QPointF(x, y), str, QApplication::font(), pen));
        }
    }

    void TextGridIOItem::onAttachTo(NeuroItem *item)
    {
        MultiGridIOItem::onAttachTo(item);
        _text_property.setEditable(_bottom_item != 0);
    }

    void TextGridIOItem::onDetach(NeuroItem *item)
    {
        MultiGridIOItem::onDetach(item);
        _text_property.setEditable(false);
    }

    void TextGridIOItem::updateDisplayBuffer(QChar ch, QVector<QChar> &v, int &pos)
    {
        if (v.size() > 0)
        {
            pos = (pos+1) % v.size();
            v[pos] = ch;
        }
    }

    void TextGridIOItem::updateInputBuffer()
    {
        _input_buffer.close();

        QTextCodec *codec = QTextCodec::codecForName("UTF-8"); // freed in library
        _input_array = codec->fromUnicode(_input_text);
        _input_buffer.setData(_input_array);
        _input_buffer.open(QIODevice::ReadOnly);
    }

    void TextGridIOItem::networkPreStep()
    {
        // calculate new value
        qreal new_val = 0;
        if ((_cur_step % (_input_spike + _input_gap)) < _input_spike)
            new_val = 1;
        ++_cur_step;

        // get the next UTF-8 byte from the input, and set the corresponding output cell's value
        if (_input_buffer.isOpen())
        {
            if (_repeat_input && _input_buffer.atEnd())
                _input_buffer.seek(0);

            if (!_input_buffer.atEnd())
            {
                char ch;
                if (_input_buffer.read(&ch, 1))
                {
                    quint8 idx = static_cast<quint8>(ch);
                    if (idx < _outgoing_cells.size())
                    {
                        NeuroLib::NeuroNet::ASYNC_STATE *cell = getCell(_outgoing_cells[idx]);
                        if (cell)
                        {
                            cell->current().setOutputValue(new_val);
                            //cell->former().setOutputValue(new_val);
                            updateDisplayBuffer(idx, _input_disp_buffer, _input_disp_pos);
                        }
                    }
                }
            }
        }
    }

    void TextGridIOItem::networkPostStep()
    {
        _step_output_text.clear();

        // calculate average
        const int num = _incoming_cells.size();
        double avg = 0;
        for (int i = 0; i < _incoming_cells.size(); ++i)
        {
            NeuroLib::NeuroNet::ASYNC_STATE *cell = getCell(i);
            if (cell)
            {
                double val = cell->current().outputValue();
                avg += val;
            }
        }
        avg /= num;

        // calculate standard deviation and max value
        double deviation = 0;
        double highest_output = 0;
        int highest_index = -1;
        for (int i = 0; i < num; ++i)
        {
            NeuroLib::NeuroNet::ASYNC_STATE *cell = getCell(i);
            if (cell)
            {
                double val = cell->current().outputValue();
                double diff = val - avg;
                deviation += diff * diff;

                if (val >= _output_threshold)
                {
                    if (val > highest_output)
                    {
                        highest_output = val;
                        highest_index = i;
                    }

                    if (i >= ' ' && i < 127)
                        _step_output_text.append(static_cast<QChar>(i));
                }
            }
        }
        deviation = ::sqrt(deviation / num);

        // if there's an outlier, add it
        if (highest_index != -1 && (highest_output - avg) > deviation)
        {
            char ch = static_cast<char>(highest_index % 256);
            _output_buffer.write(&ch, 1);

            // TODO: if the high bit is not set, read the next char from _output_buffer_stream

            if (ch >= ' ' && ch < 127)
                updateDisplayBuffer(static_cast<quint8>(ch), _output_disp_buffer, _output_disp_pos);
        }
        else
        {
            updateDisplayBuffer(' ', _output_disp_buffer, _output_disp_pos);
        }
    }

    void TextGridIOItem::writeBinary(QDataStream &ds, const NeuroLabFileVersion &file_version) const
    {
        MultiGridIOItem::writeBinary(ds, file_version);

        ds << _repeat_input;
        ds << _input_spike;
        ds << _input_gap;
        ds << false; //_accum_output;
        ds << _output_threshold;
        ds << _input_text;
    }

    void TextGridIOItem::readBinary(QDataStream &ds, const NeuroLabFileVersion &file_version)
    {
        MultiGridIOItem::readBinary(ds, file_version);

        bool temp;
        ds >> _repeat_input;
        ds >> _input_spike;
        ds >> _input_gap;
        ds >> temp; //_accum_output;
        ds >> _output_threshold;
        ds >> _input_text;
    }

    void TextGridIOItem::postLoad()
    {
        MultiGridIOItem::postLoad();
        _text_property.setEditable(_bottom_item != 0);
        updateInputBuffer();
    }

} // namespace GridItems
