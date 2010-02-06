#ifndef NEURONODE_H
#define NEURONODE_H

#include <QSet>
#include <QDataStream>
#include <cmath>

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
            inline void operator() (const NeuroCell & prev, NeuroCell & next, const int & numNeighbors, const NeuroCell * const * const neighbors) const
            {
                NeuroValue input_sum = 0;
                for (int i = 0; i < numNeighbors; ++i)
                    input_sum += neighbors[i]->_value;
                
                if (input_sum < 0)
                    input_sum = 0;
                
                switch (prev._kind)
                {
                case NODE:
                case EXCITORY_LINK:
                    next._value = static_cast<NeuroValue>(1) 
                                  / (static_cast<NeuroValue>(1) 
                                     + ::exp(static_cast<NeuroValue>(4) - static_cast<NeuroValue>(8) * input_sum / prev._input_threshold));
                    break;
                case INHIBITORY_LINK:
                    next._value = prev._input_threshold ? -input_sum : static_cast<NeuroValue>(-1000);
                    
                    break;
                default:
                    return;
                }
            }
        };
        
        void addInput(NeuroNet *network, const NeuroIndex & my_index, const NeuroIndex & input_index);
        void removeInput(NeuroNet *network, const NeuroIndex & my_index, const NeuroIndex & input_index);
        
        /// Output value of the cell.
        const NeuroValue & value() const { return _value; }
        
    private:
        Kind _kind;
                
        NeuroValue _value;
        NeuroValue _input_threshold;
        
        friend NEUROLIBSHARED_EXPORT QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc);
        friend NEUROLIBSHARED_EXPORT QDataStream & operator>> (QDataStream & ds, NeuroCell & nc);
    }; // class NeuroCell
        
    // IO Functions
    NEUROLIBSHARED_EXPORT QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc);
    NEUROLIBSHARED_EXPORT QDataStream & operator>> (QDataStream & ds, NeuroCell & nc);
    
} // namespace NeuroLib

#endif // NEURONODE_H
