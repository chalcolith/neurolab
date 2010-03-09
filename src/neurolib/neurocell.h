#ifndef NEURONODE_H
#define NEURONODE_H

#include <QSet>
#include <QDataStream>

#include "../automata/automaton.h"
#include "neurolib_global.h"

namespace NeuroLib
{
    
    class NeuroNet;
    
    /// Base class for elements of the neurocognitive network (nodes or links).
    class NEUROLIBSHARED_EXPORT NeuroCell
    {
    public:
        enum KindOfCell
        {
            NODE = 0,
            EXCITORY_LINK,
            INHIBITORY_LINK,
            NUM_KINDS
        };
        
        typedef int NeuroIndex;
        typedef float NeuroValue;

        /// The input threshold MUST be > 0, except for inhibitory links.        
        NeuroCell(const KindOfCell & k = NeuroCell::NODE, 
                  const NeuroValue & slope_or_weight = 1,
                  const NeuroValue & current_value = 0);
        
        inline const KindOfCell & kind() const { return _kind; }
                
        inline const bool & frozen() const { return _frozen; }
        inline void setFrozen(const bool & frozen) { _frozen = frozen; }
        
        const NeuroValue & slope() const { return _slope; }
        void setSlope(const NeuroValue & slope) { _slope = slope; }
        
        const NeuroValue & weight() const { return _weight; }
        void setWeight(const NeuroValue & weight) { _weight = weight; }

        const NeuroValue & currentValue() const { return _current_value; }
        void setCurrentValue(const NeuroValue & v) { _current_value = v; }
        
        void addInput(NeuroNet *network, const NeuroIndex & my_index, const NeuroIndex & input_index);
        void removeInput(NeuroNet *network, const NeuroIndex & my_index, const NeuroIndex & input_index);        
        
        struct Update
        {
            void operator() (Automata::Automaton<NeuroCell, NeuroCell::Update, NeuroCell::NeuroIndex> *neuronet, const NeuroIndex & index, const NeuroCell & prev, NeuroCell & next, const int & numNeighbors, const NeuroCell * const * const neighbors) const;
        };
        
    private:
        KindOfCell _kind;
        bool _frozen;
        
        union
        {
            NeuroValue _slope;
            NeuroValue _weight;
        };
        
        NeuroValue _current_value;
        
        friend NEUROLIBSHARED_EXPORT QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc);
        friend NEUROLIBSHARED_EXPORT QDataStream & operator>> (QDataStream & ds, NeuroCell & nc);
    }; // class NeuroCell
        
    // IO Functions
    NEUROLIBSHARED_EXPORT QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc);
    NEUROLIBSHARED_EXPORT QDataStream & operator>> (QDataStream & ds, NeuroCell & nc);
    
} // namespace NeuroLib

#endif // NEURONODE_H
