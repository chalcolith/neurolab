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
    public:
        NeuroNet();
        
    protected:
        virtual QDataStream & writeBinary(QDataStream & ds) const;
        virtual QDataStream & readBinary(QDataStream & ds);
    };
    
} // namespace NeuroLib

#endif // NEURONET_H
