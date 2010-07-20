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

#include "neurotextitem.h"
#include "labscene.h"
#include "labnetwork.h"

namespace NeuroLab
{
    
    NEUROITEM_DEFINE_CREATOR(NeuroTextItem, QObject::tr("Text Item"));
    
    NeuroTextItem::NeuroTextItem(LabNetwork *network, const QPointF & scenePos, const CreateContext & context)
        : NeuroItem(network, scenePos, context),
        _font(QApplication::font()),
        _text(tr("<text label>")),
        _font_property(this, &NeuroTextItem::font, &NeuroTextItem::setFont, tr("Font"), tr("The font to use when displaying the text.")),
        _text_property(this, &NeuroTextItem::text, &NeuroTextItem::setText, tr("Text"), tr("The text to display."))
    {
    }
    
    NeuroTextItem::~NeuroTextItem()
    {
    }
    
    void NeuroTextItem::addToShape(QPainterPath &, QList<TextPathRec> & texts) const
    {
        QPen pen;
        
        setPenProperties(pen);        
        texts.append(TextPathRec(scenePos(), text(), font(), pen));
    }
    
} // namespace NeuroLab
