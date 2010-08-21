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

    /// \deprecated
    static const QString NETWORK_COOKIE_OLD("NeuroLib NETWORK 012");

    /// Magic cookie for neurolib files.
    static const QString NETWORK_COOKIE_NEW("NeuroLib NETWORK");

    NeuroNet::NeuroNet()
        : BASE(),
        _decay(1),
        _link_learn_rate(0),
        _node_learn_rate(0),
        _node_forget_rate(0),
        _learn_time(10)
    {
    }

    void NeuroNet::preUpdate()
    {
        _postUpdates.clear();
    }

    void NeuroNet::postUpdate()
    {
        for (QListIterator<PostUpdateRec> i(_postUpdates); i.hasNext(); )
        {
            const PostUpdateRec & rec = i.next();
            _nodes[rec._index].current().setWeight(rec._weight);
        }
    }

    void NeuroNet::addPostUpdate(const PostUpdateRec & rec)
    {
        QWriteLocker wl(&_postUpdatesLock);
        _postUpdates.append(rec);
    }

    void NeuroNet::writeBinary(QDataStream & ds, const Automata::AutomataFileVersion & file_version) const
    {
        Automata::AutomataFileVersion & fv = const_cast<Automata::AutomataFileVersion &>(file_version);

        fv.automata_version = Automata::AUTOMATA_NUM_FILE_VERSIONS - 1;
        fv.client_version = NeuroLib::NEUROLIB_NUM_FILE_VERSIONS - 1;

        ds << NETWORK_COOKIE_NEW;
        ds << static_cast<quint16>(fv.automata_version);
        ds << static_cast<quint16>(fv.client_version);
        ds << static_cast<float>(_decay);
        ds << static_cast<float>(_link_learn_rate);
        ds << static_cast<float>(_node_learn_rate);
        ds << static_cast<float>(_node_forget_rate);
        ds << static_cast<float>(_learn_time);

        BASE::writeBinary(ds, fv);
    }

    void NeuroNet::readBinary(QDataStream & ds, const Automata::AutomataFileVersion & file_version)
    {
        Automata::AutomataFileVersion & fv = const_cast<Automata::AutomataFileVersion &>(file_version);

        QString cookie;
        ds >> cookie;

        if (cookie == NETWORK_COOKIE_OLD)
        {
            fv.automata_version = Automata::AUTOMATA_FILE_VERSION_OLD;
            fv.client_version = NeuroLib::NEUROLIB_FILE_VERSION_OLD;

            float n;
            ds >> n; _decay = static_cast<NeuroCell::NeuroValue>(n);
            ds >> n; _link_learn_rate = static_cast<NeuroCell::NeuroValue>(n);
            ds >> n; _learn_time = static_cast<NeuroCell::NeuroValue>(n);

            BASE::readBinary(ds, fv);
        }
        else if (cookie == NETWORK_COOKIE_NEW)
        {
            quint16 ver;
            ds >> ver; fv.automata_version = ver;
            ds >> ver; fv.client_version = ver;

            float n;
            ds >> n; _decay = static_cast<NeuroCell::NeuroValue>(n);
            ds >> n; _link_learn_rate = static_cast<NeuroCell::NeuroValue>(n);

            if (fv.client_version >= NeuroLib::NEUROLIB_FILE_VERSION_2)
            {
                ds >> n; _node_learn_rate = static_cast<NeuroCell::NeuroValue>(n);
            }

            if (fv.client_version >= NeuroLib::NEUROLIB_FILE_VERSION_3)
            {
                ds >> n; _node_forget_rate = static_cast<NeuroCell::NeuroValue>(n);
            }

            ds >> n; _learn_time = static_cast<NeuroCell::NeuroValue>(n);

            BASE::readBinary(ds, fv);
        }
        else
        {
            throw new Automata::FileFormatError();
        }
    }

} // namespace NeuroLib
