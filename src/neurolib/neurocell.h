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
        enum Kind
        {
            NODE = 0,
            EXCITORY_LINK,
            INHIBITORY_LINK,
            NUM_KINDS
        };
        
        typedef int NeuroIndex;
        typedef double NeuroValue;

        /// The input threshold MUST be > 0, except for inhibitory links.        
        NeuroCell(const Kind & k = NODE, const NeuroValue & input_threshold = 1, const NeuroValue & output_weight = 1, const NeuroValue & output_value = 0);
        
        const Kind & kind() const { return _kind; }
        
        struct Update
        {
            void operator() (Automata::Automaton<NeuroCell, NeuroCell::Update, NeuroCell::NeuroIndex> *neuronet, const NeuroIndex & index, const NeuroCell & prev, NeuroCell & next, const int & numNeighbors, const NeuroCell * const * const neighbors) const;
        };
        
        void addInput(NeuroNet *network, const NeuroIndex & my_index, const NeuroIndex & input_index);
        void removeInput(NeuroNet *network, const NeuroIndex & my_index, const NeuroIndex & input_index);
        
        /// Input threshold (roughly the number of fully active inputs it takes to fully activate the node)
        const NeuroValue & inputThreshold() const { return _input_threshold; }
        void setInputThreshold(const NeuroValue & input_threshold) { _input_threshold = input_threshold; }

        /// Output weight (proportion of full output the cell will output)
        const NeuroValue & outputWeight() const { return _output_weight; }
        void setOutputWeight(const NeuroValue & output_weight) { _output_weight = output_weight; }
        
        /// Output value of the cell.
        const NeuroValue & outputValue() const { return _output_value; }
        void setOutputValue(const NeuroValue & v) { _output_value = v; }
        
        ///
        
        const bool & frozen() const { return _frozen; }
        void setFrozen(const bool & frozen) { _frozen = frozen; }
        
    private:
        Kind _kind;
                
        NeuroValue _input_threshold;
        NeuroValue _output_weight;
        NeuroValue _output_value;
        
        bool _frozen;
        
        friend NEUROLIBSHARED_EXPORT QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc);
        friend NEUROLIBSHARED_EXPORT QDataStream & operator>> (QDataStream & ds, NeuroCell & nc);
    }; // class NeuroCell
        
    // IO Functions
    NEUROLIBSHARED_EXPORT QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc);
    NEUROLIBSHARED_EXPORT QDataStream & operator>> (QDataStream & ds, NeuroCell & nc);
    
} // namespace NeuroLib

#endif // NEURONODE_H
