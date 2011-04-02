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
    NEUROITEM_DEFINE_PLUGIN_CREATOR(TextGridIOItem, QObject::tr("Grid Items"), QObject::tr("Text IO Item"), ":/griditems/icons/text_io_item.png", GridItems::VERSION)

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

        _incoming_cells.reserve(NUM_CONNECTIONS);
        _outgoing_cells.reserve(NUM_CONNECTIONS);

        if (context == NeuroItem::CREATE_UI)
        {
            NeuroLib::NeuroCell cell(NeuroLib::NeuroCell::NODE);

            for (int i = 0; i < NUM_CONNECTIONS; ++i)
                _incoming_cells.append(network->neuronet()->addNode(cell));
            for (int i = 0; i < NUM_CONNECTIONS; ++i)
                _outgoing_cells.append(network->neuronet()->addNode(cell));
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
            NeuroLib::NeuroNet::ASYNC_STATE *cell;
            if ((cell = getCell(_outgoing_cells[idx])))
            {
                cell->current().setOutputValue(new_val);
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
