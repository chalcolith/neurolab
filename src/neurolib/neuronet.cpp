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

#include "neuronet.h"
#include "../automata/exception.h"

namespace NeuroLib
{

    static const QString NETWORK_COOKIE("NeuroLib NETWORK 012");

    NeuroNet::NeuroNet()
        : BASE(),
        _decay(1),
        _learn_rate(0),
        _learn_time(10)
    {
    }

    QDataStream & NeuroNet::writeBinary(QDataStream & ds) const
    {
        ds << NETWORK_COOKIE;
        ds << static_cast<float>(_decay);
        ds << static_cast<float>(_learn_rate);
        ds << static_cast<float>(_learn_time);

        return BASE::writeBinary(ds);
    }

    QDataStream & NeuroNet::readBinary(QDataStream & ds)
    {
        QString cookie;
        ds >> cookie;

        if (cookie != NETWORK_COOKIE)
        {
            throw Automata::Exception(QObject::tr("Network file is not compatible with this version of NeuroLib."));
        }

        float n;
        ds >> n; _decay = static_cast<NeuroCell::NeuroValue>(n);
        ds >> n; _learn_rate = static_cast<NeuroCell::NeuroValue>(n);
        ds >> n; _learn_time = static_cast<NeuroCell::NeuroValue>(n);

        return BASE::readBinary(ds);
    }

} // namespace NeuroLib
