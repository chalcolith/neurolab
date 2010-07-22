#ifndef NEURONET_H
#define NEURONET_H

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

#include <QDataStream>

#include "neurolib_global.h"
#include "neurocell.h"

#include "../automata/automaton.h"

namespace NeuroLib
{

    /// A neurocognitive network.
    class NEUROLIBSHARED_EXPORT NeuroNet
        : public Automata::Automaton<NeuroCell, NeuroCell::Update, NeuroCell::NeuroIndex>
    {
        NeuroCell::NeuroValue _decay;
        NeuroCell::NeuroValue _link_learn_rate;
        NeuroCell::NeuroValue _node_learn_rate;
        NeuroCell::NeuroValue _node_forget_rate;
        NeuroCell::NeuroValue _learn_time;

    public:
        /// Constructor.
        NeuroNet();

        /// The decay rate of nodes in the network per timestep.
        /// \see NeuroNet::setDecay()
        NeuroCell::NeuroValue decay() const { return _decay; }

        /// Sets the decay rate of nodes in the network.
        /// \see NeuroNet::decay()
        void setDecay(const NeuroCell::NeuroValue & decay) { _decay = decay; }

        /// The learn rate of links in the network.  This is the maximum amount a link's weight will increase via Hebbian learning each timestep.
        /// \see NeuroNet::setLearn()
        NeuroCell::NeuroValue linkLearnRate() const { return _link_learn_rate; }

        /// Sets the learn rate of links in the network.
        /// \see NeuroNet::learn()
        void setLinkLearnRate(const NeuroCell::NeuroValue & learn) { _link_learn_rate = learn; }

        NeuroCell::NeuroValue nodeLearnRate() const { return _node_learn_rate; }
        void setNodeLearnRate(const NeuroCell::NeuroValue & rate) { _node_learn_rate = rate; }

        NeuroCell::NeuroValue nodeForgetRate() const { return _node_forget_rate; }
        void setNodeForgetRate(const NeuroCell::NeuroValue & rate) { _node_forget_rate = rate; }

        /// This is the number of timesteps for which a cell's running average is calculated, for the purposes of Hebbian learning.
        /// \see NeuroNet::setLearnTime()
        NeuroCell::NeuroValue learnTime() const { return _learn_time; }

        /// Sets the number of timesteps for which a cell's running average is calculated.
        /// \see NeuroNet::learnTime()
        void setLearnTime(const NeuroCell::NeuroValue & learnTime) { _learn_time = learnTime; }

        virtual void writeBinary(QDataStream & ds, const Automata::AutomataFileVersion & file_version) const;
        virtual void readBinary(QDataStream & ds, const Automata::AutomataFileVersion & file_version);
    };

} // namespace NeuroLib

#endif // NEURONET_H
