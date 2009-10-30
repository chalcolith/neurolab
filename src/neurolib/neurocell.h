#ifndef NEURONODE_H
#define NEURONODE_H

#include <QDataStream>

#include "neurolib_global.h"

namespace NeuroLib
{
    
    /// Base class for elements of the neurocognitive network (nodes or links).
    class NEUROLIBSHARED_EXPORT NeuroCell
    {
    public:
        typedef int NeuroIndex;
        
        NeuroCell();
        
        struct Update
        {
            void operator() (const NeuroCell & prev, NeuroCell & next, const int & numNeighbors, const NeuroCell * const * const neighbors) const
            {
            }
        };
        
        friend QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc);
        friend QDataStream & operator>> (QDataStream & ds, NeuroCell & nc);
    };
    
    // IO Functions
    NEUROLIBSHARED_EXPORT QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc);
    NEUROLIBSHARED_EXPORT QDataStream & operator>> (QDataStream & ds, NeuroCell & nc);
    
} // namespace NeuroLib

#endif // NEURONODE_H
