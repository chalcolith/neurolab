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
        NeuroNet();
        
        NeuroCell::NeuroValue decay() const { return _decay; }
        void setDecay(const NeuroCell::NeuroValue & decay) { _decay = decay; }
        
        NeuroCell::NeuroValue learn() const { return _learn; }
        void setLearn(const NeuroCell::NeuroValue & learn) { _learn = learn; }
        
        NeuroCell::NeuroValue learnTime() const { return _learn_time; }
        void setLearnTime(const NeuroCell::NeuroValue & learnTime) { _learn_time = learnTime; }
        
    protected:
        virtual QDataStream & writeBinary(QDataStream & ds) const;
        virtual QDataStream & readBinary(QDataStream & ds);
        
        friend class NeuroCell;
    };
    
} // namespace NeuroLib

#endif // NEURONET_H
