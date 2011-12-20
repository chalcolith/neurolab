#ifndef TEXTGRIDIOITEM_H
#define TEXTGRIDIOITEM_H

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

#include "griditems_global.h"
#include "multigridioitem.h"

namespace GridItems
{

    class GRIDITEMSSHARED_EXPORT UTF8Buffer
    {
        QBuffer _buffer;
        QTextStream _stream;
        qint64 _read_pos;
        int _bytes_left;

    public:
        UTF8Buffer();
        virtual ~UTF8Buffer();

        void writeByte(quint8 b);
        bool atEnd() const { return _buffer.atEnd(); }
        bool canRead() const { return _bytes_left == 0 && !_buffer.atEnd(); }
        QChar readChar();

        void setData(const QByteArray &data);
        void reset();
    };

    class GRIDITEMSSHARED_EXPORT TextGridIOItem
            : public MultiGridIOItem
    {
        Q_OBJECT
        NEUROITEM_DECLARE_CREATOR

        static const int NUM_CONNECTIONS;

        bool _repeat_input;
        qint16 _input_spike;
        qint16 _input_gap;

        qreal _output_threshold;

        QString _input_text;

        Property<TextGridIOItem, QVariant::Bool, bool, bool> _repeat_property;
        Property<TextGridIOItem, QVariant::Int, qint32, qint32> _spike_property;
        Property<TextGridIOItem, QVariant::Int, qint32, qint32> _gap_property;

        Property<TextGridIOItem, QVariant::Double, qreal, qreal> _threshold_property;

        Property<TextGridIOItem, QVariant::String, QString, QString> _text_property;

        QBuffer _to_grid;
        UTF8Buffer _from_grid;

        QString _step_output_text;

        QVector<QChar> _input_disp_buffer;
        mutable int _input_disp_pos;

        QVector<QChar> _output_disp_buffer;
        mutable int _output_disp_pos;

        mutable int _cur_step;

    public:
        TextGridIOItem(NeuroGui::LabNetwork *network, const QPointF & scenePos, const CreateContext & context);
        virtual ~TextGridIOItem();

        bool repeatInput() const { return _repeat_input; }
        void setRepeatInput(const bool & r) { _repeat_input = r; }

        int inputSpike() const { return _input_spike; }
        void setInputSpike(const int & s) { _input_spike = s; }

        int inputGap() const { return _input_gap; }
        void setInputGap(const int & g) { _input_gap = g; }

        qreal outputThreshold() const { return _output_threshold; }
        void setOutputThreshold(const qreal & d) { _output_threshold = d; }

        QString inputText() const { return _input_text; }
        void setInputText(const QString & s) { _input_text = s;  }

        virtual QString dataValue() const;

        virtual void addToShape(QPainterPath &drawPath, QList<TextPathRec> &texts) const;

        virtual void onAttachTo(NeuroItem *item);
        virtual void onDetach(NeuroItem *item);

    public slots:
        virtual void resetInputText();
        virtual void networkPreStep();
        virtual void networkPostStep();

    protected:
        virtual void writeBinary(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version) const;
        virtual void readBinary(QDataStream &ds, const NeuroGui::NeuroLabFileVersion &file_version);
        virtual void postLoad();

    private:
        void updateDisplayBuffer(QChar ch, QVector<QChar> & v, int & pos);
    };

} // namespace GridItems

#endif // TEXTGRIDIOITEM_H
