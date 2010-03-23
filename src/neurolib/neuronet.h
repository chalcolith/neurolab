#ifndef NEURONET_H
#define NEURONET_H

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
        NeuroCell::NeuroValue _learn;
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
        NeuroCell::NeuroValue learn() const { return _learn; }

        /// Sets the learn rate of links in the network.
        /// \see NeuroNet::learn()
        void setLearn(const NeuroCell::NeuroValue & learn) { _learn = learn; }

        /// This is the number of timesteps for which a cell's running average is calculated, for the purposes of Hebbian learning.
        /// \see NeuroNet::setLearnTime()
        NeuroCell::NeuroValue learnTime() const { return _learn_time; }

        /// Sets the number of timesteps for which a cell's running average is calculated.
        /// \see NeuroNet::learnTime()
        void setLearnTime(const NeuroCell::NeuroValue & learnTime) { _learn_time = learnTime; }

    protected:
        virtual QDataStream & writeBinary(QDataStream & ds) const;
        virtual QDataStream & readBinary(QDataStream & ds);

        friend class NeuroCell;
    };

} // namespace NeuroLib

#endif // NEURONET_H
