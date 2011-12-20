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

#include "textgridioitem.h"
#include "../neurogui/labscene.h"
#include "../neurogui/labview.h"
#include "../neurogui/labtree.h"
#include "../neurogui/labnetwork.h"
#include "../neurolib/neuronet.h"

#include <QTextCodec>

using namespace NeuroGui;

namespace GridItems
{

    UTF8Buffer::UTF8Buffer()
        : _read_pos(0), _bytes_left(-1)
    {
        _buffer.open(QIODevice::ReadWrite);
        _stream.setDevice(&_buffer);
        _stream.setCodec("UTF-8");
    }

    UTF8Buffer::~UTF8Buffer()
    {
        _buffer.close();
    }

    void UTF8Buffer::writeByte(quint8 b)
    {
        _buffer.seek(_buffer.buffer().size());

        char ch = static_cast<char>(b);
        _buffer.write(&ch, 1);

        if ((b & 0xFC) == 0xFC)
            _bytes_left = 5;
        else if ((b & 0xF8) == 0xF8)
            _bytes_left = 4;
        else if ((b & 0xF0) == 0xF0)
            _bytes_left = 3;
        else if ((b & 0xE0) == 0xE0)
            _bytes_left = 2;
        else if ((b & 0xC0) == 0xC0)
            _bytes_left = 1;
        else if ((b & 0x80) == 0x80)
            --_bytes_left;
        else
            _bytes_left = 0;

        _buffer.seek(_read_pos);
    }

    QChar UTF8Buffer::readChar()
    {
        if (_bytes_left == 0 && !_buffer.atEnd())
        {
            QChar ch;
            _stream >> ch;
            _read_pos = _buffer.pos();

            return ch;
        }
        return QChar(0);
    }

    void UTF8Buffer::reset()
    {
        setData(QByteArray());
    }

    void UTF8Buffer::setData(const QByteArray &data)
    {
        _buffer.close();
        _buffer.setData(data);
        _buffer.open(QIODevice::ReadWrite);
        _stream.setDevice(&_buffer);
        _stream.setCodec("UTF-8");
        _read_pos = 0;
        _bytes_left = -1;
    }


    //
    NEUROITEM_DEFINE_PLUGIN_CREATOR(TextGridIOItem, QObject::tr("Grid Items"), QObject::tr("Text IO Item"), ":/griditems/icons/text_io_item.png", GridItems::VERSION())

    const int TextGridIOItem::NUM_CONNECTIONS = 256;

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
          _to_grid(), _from_grid(),
          _input_disp_buffer(10), _input_disp_pos(0), _output_disp_buffer(10), _output_disp_pos(0),
          _cur_step(0)
    {
        _value_property.setVisible(false);
        _width_property.setEditable(false);

        _incoming_cells.reserve(NUM_CONNECTIONS);
        _outgoing_cells.reserve(NUM_CONNECTIONS);

        if (context == NeuroItem::CREATE_UI)
        {
            setWidth(NUM_CONNECTIONS);
        }

        resetInputText();
        connect(&_text_property, SIGNAL(valueInBrowserChanged()), this, SLOT(resetInputText()));
        connect(network, SIGNAL(preStep()), this, SLOT(networkPreStep()));
        connect(network, SIGNAL(postStep()), this, SLOT(networkPostStep()));

        for (int i = 0; i < _input_disp_buffer.size(); ++i)
            _input_disp_buffer[i] = ' ';
        for (int i = 0; i < _output_disp_buffer.size(); ++i)
            _output_disp_buffer[i] = ' ';
    }

    TextGridIOItem::~TextGridIOItem()
    {
        _to_grid.close();
    }

    QString TextGridIOItem::dataValue() const
    {
        return _step_output_text;
    }

    void TextGridIOItem::addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const
    {
        MultiGridIOItem::addToShape(drawPath, texts);

        const int num = qMin(_input_disp_buffer.size(), _output_disp_buffer.size());
        int input_p = _input_disp_pos;
        int output_p = _output_disp_pos;

        input_p = (input_p + 1) % num;
        output_p = (output_p + 1) % num;

        for (int i = 0; i < num; ++i)
        {
            qreal pct = static_cast<qreal>(i) / static_cast<qreal>(num);
            QColor col = lerp(QColor(0xff, 0xff, 0xff), QColor(0, 0, 0), pct);

            qreal buf = static_cast<qreal>(rect().width()) / 8.0;
            qreal w = static_cast<qreal>(rect().width()) - buf*2;
            qreal x = -w/2.0 + w * static_cast<qreal>(i) / static_cast<qreal>(num);
            qreal y = -2;

            QPen pen;
            pen.setColor(col);

            QChar ch = _input_disp_buffer[(input_p + i) % num];
            texts.append(TextPathRec(QPointF(x, y), QString(ch), QApplication::font(), pen));

            y = 8;
            ch = _output_disp_buffer[(output_p + i) % num];
            texts.append(TextPathRec(QPointF(x, y), QString(ch), QApplication::font(), pen));
        }
    }

    void TextGridIOItem::onAttachTo(NeuroItem *item)
    {
        MultiGridIOItem::onAttachTo(item);
    }

    void TextGridIOItem::onDetach(NeuroItem *item)
    {
        MultiGridIOItem::onDetach(item);
    }

    void TextGridIOItem::updateDisplayBuffer(QChar ch, QVector<QChar> &v, int &pos)
    {
        if (v.size() > 0)
        {
            pos = (pos+1) % v.size();
            v[pos] = ch;
        }
    }

    void TextGridIOItem::resetInputText()
    {
        _to_grid.close();

        QTextCodec *codec = QTextCodec::codecForName("UTF-8"); // freed in library
        QByteArray array = codec->fromUnicode(_input_text);
        _to_grid.setData(array);
        _to_grid.open(QIODevice::ReadOnly);
    }

    void TextGridIOItem::networkPreStep()
    {
        // calculate new value
        qreal new_val = 0;
        if ((_cur_step % (_input_spike + _input_gap)) < _input_spike)
            new_val = 1;
        ++_cur_step;

        // get the next UTF-8 byte from the input text, and set the corresponding output cell's value
        if (_repeat_input && _to_grid.atEnd())
            resetInputText();

        char ch;
        if (!_to_grid.atEnd() && _to_grid.read(&ch, 1) == 1)
        {
            quint8 idx = static_cast<quint8>(ch) % _outgoing_cells.size();

#ifdef DEBUG
            if (NUM_CONNECTIONS < 256 && ch >= 'a')
                idx = (static_cast<quint8>(ch) - 'a') % _outgoing_cells.size();
#endif

            NeuroLib::NeuroNet::ASYNC_STATE *cell;
            if ((cell = getCell(_outgoing_cells[idx])))
            {
                cell->current().setOutputValue(new_val);

#ifdef DEBUG
                if (NUM_CONNECTIONS < 256 && idx < 'a')
                    idx += 'a';
#endif

                updateDisplayBuffer(idx, _input_disp_buffer, _input_disp_pos);
            }
        }
    }

    void TextGridIOItem::networkPostStep()
    {
        _step_output_text.clear();

        // calculate average
        const int num = _incoming_cells.size();
        double avg = 0;
        for (int i = 0; i < num; ++i)
        {
            NeuroLib::NeuroNet::ASYNC_STATE *cell = getCell(_incoming_cells[i]);
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
            NeuroLib::NeuroNet::ASYNC_STATE *cell = getCell(_incoming_cells[i]);
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
                }
            }
        }
        deviation = ::sqrt(deviation / num);

        // if there's an outlier, add it
        bool add_space = true;
        if (highest_index != -1 && (highest_output - avg) > deviation)
        {
            quint8 idx = static_cast<quint8>(highest_index % 256);

#ifdef DEBUG
            if (NUM_CONNECTIONS < 256)
                idx += 'a';
#endif

            _from_grid.writeByte(idx);

            if (_from_grid.canRead())
            {
                QChar ch = _from_grid.readChar();
#ifdef DEBUG
                if (ch < ' ')
                    ch = QChar(ch.unicode() + 0x03b1); // alpha
#endif
                _step_output_text.append(ch);
                updateDisplayBuffer(ch, _output_disp_buffer, _output_disp_pos);
                add_space = false;
            }
        }

        if (add_space)
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
        resetInputText();
    }

} // namespace GridItems
