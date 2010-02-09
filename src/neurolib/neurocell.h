#ifndef NEURONODE_H
#define NEURONODE_H

#include <QSet>
#include <QDataStream>

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
        NeuroCell(const Kind & k = NODE, const NeuroValue & input_threshold = 1);
        
        const Kind & kind() const { return _kind; }
        
        struct Update
        {
            void operator() (const NeuroCell & prev, NeuroCell & next, const int & numNeighbors, const NeuroCell * const * const neighbors) const;
        };
        
        void addInput(NeuroNet *network, const NeuroIndex & my_index, const NeuroIndex & input_index);
        void removeInput(NeuroNet *network, const NeuroIndex & my_index, const NeuroIndex & input_index);
        
        /// Output value of the cell.
        const NeuroValue & value() const { return _value; }
        void setValue(const NeuroValue & v) { _value = v; }
        
        const NeuroValue & input_threshold() const { return _input_threshold; }
        void setInputThreshold(const NeuroValue & input_threshold) { _input_threshold = input_threshold; }
        
        const bool & frozen() const { return _frozen; }
        void setFrozen(const bool & frozen) { _frozen = frozen; }
        
    private:
        Kind _kind;
                
        NeuroValue _value;
        NeuroValue _input_threshold;
        
        bool _frozen;
        
        friend NEUROLIBSHARED_EXPORT QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc);
        friend NEUROLIBSHARED_EXPORT QDataStream & operator>> (QDataStream & ds, NeuroCell & nc);
    }; // class NeuroCell
        
    // IO Functions
    NEUROLIBSHARED_EXPORT QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc);
    NEUROLIBSHARED_EXPORT QDataStream & operator>> (QDataStream & ds, NeuroCell & nc);
    
} // namespace NeuroLib

#endif // NEURONODE_H
