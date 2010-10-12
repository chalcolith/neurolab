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

#include "neurolib_global.h"
#include "neurocell.h"

#include "../automata/automaton.h"

#include <QDataStream>
#include <QReadWriteLock>

namespace NeuroLib
{

    /// A neurocognitive network.
    class NEUROLIBSHARED_EXPORT NeuroNet
        : public NeuroCell::NEURONET_BASE
    {
        NeuroCell::Value _decay;
        NeuroCell::Value _link_learn_rate;
        NeuroCell::Value _node_learn_rate;
        NeuroCell::Value _node_forget_rate;
        NeuroCell::Value _learn_time;

    public:
        /// Constructor.
        NeuroNet();

        /// The decay rate of nodes in the network per timestep.
        /// \see NeuroNet::setDecay()
        NeuroCell::Value decay() const { return _decay; }

        /// Sets the decay rate of nodes in the network.
        /// \see NeuroNet::decay()
        void setDecay(const NeuroCell::Value & decay) { _decay = decay; }

        /// The learn rate of links in the network.  This is the maximum amount a link's weight will increase via Hebbian learning each timestep.
        /// \see NeuroNet::setLearn()
        NeuroCell::Value linkLearnRate() const { return _link_learn_rate; }

        /// Sets the learn rate of links in the network.
        /// \see NeuroNet::learn()
        void setLinkLearnRate(const NeuroCell::Value & learn) { _link_learn_rate = learn; }

        NeuroCell::Value nodeLearnRate() const { return _node_learn_rate; }
        void setNodeLearnRate(const NeuroCell::Value & rate) { _node_learn_rate = rate; }

        NeuroCell::Value nodeForgetRate() const { return _node_forget_rate; }
        void setNodeForgetRate(const NeuroCell::Value & rate) { _node_forget_rate = rate; }

        /// This is the number of timesteps for which a cell's running average is calculated, for the purposes of Hebbian learning.
        /// \see NeuroNet::setLearnTime()
        NeuroCell::Value learnTime() const { return _learn_time; }

        /// Sets the number of timesteps for which a cell's running average is calculated.
        /// \see NeuroNet::learnTime()
        void setLearnTime(const NeuroCell::Value & learnTime) { _learn_time = learnTime; }

        struct PostUpdateRec
        {
            NeuroCell::Index _index;
            NeuroCell::Value _weight;

            PostUpdateRec(const NeuroCell::Index & index, const NeuroCell::Value & weight)
                : _index(index), _weight(weight)
            {
            }
        };

        void addPostUpdate(const PostUpdateRec &);

        void preUpdate();
        void postUpdate();

        virtual void writeBinary(QDataStream & ds, const Automata::AutomataFileVersion & file_version) const;
        virtual void readBinary(QDataStream & ds, const Automata::AutomataFileVersion & file_version);

    private:
        QList<PostUpdateRec> _postUpdates;
        QReadWriteLock _postUpdatesLock;
    };

} // namespace NeuroLib

#endif // NEURONET_H
