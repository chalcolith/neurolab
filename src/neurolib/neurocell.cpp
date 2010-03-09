#include "neurocell.h"
#include "neuronet.h"
#include "../automata/exception.h"
#include <cmath>

namespace NeuroLib
{
    
    NeuroCell::NeuroCell(const KindOfCell & k, 
                         const NeuroValue & slope_or_weight, 
                         const NeuroValue & current_value)
        : _kind(k), _frozen(false), 
        _slope(slope_or_weight), 
        _current_value(current_value)
    {
    }
        
    void NeuroCell::addInput(NeuroNet *network, const NeuroIndex & my_index, const NeuroIndex & input_index)
    {
        network->addEdge(my_index, input_index);
    }
    
    void NeuroCell::removeInput(NeuroNet *network, const NeuroIndex & my_index, const NeuroIndex & input_index)
    {
        network->removeEdge(my_index, input_index);
    }

    static const NeuroCell::NeuroValue ZERO = static_cast<NeuroCell::NeuroValue>(0);
    static const NeuroCell::NeuroValue ONE = static_cast<NeuroCell::NeuroValue>(1);
    
    void NeuroCell::Update::operator() (Automata::Automaton<NeuroCell, NeuroCell::Update, NeuroCell::NeuroIndex> *, const NeuroIndex &, const NeuroCell & prev, NeuroCell & next, const int & numNeighbors, const NeuroCell * const * const neighbors) const
    {
        next._frozen = prev._frozen;
        
        if (prev._frozen)
        {
            next._current_value = prev._current_value;
            return;
        }
        
        NeuroValue input_sum = 0;
        for (int i = 0; i < numNeighbors; ++i)
            input_sum += neighbors[i]->_current_value;        
        input_sum = qBound(static_cast<NeuroValue>(0), input_sum, static_cast<NeuroValue>(1));

        NeuroValue next_value;
        
        switch (prev._kind)
        {
        case NODE:
            if (input_sum > 0)
                next_value = ONE / (ONE + ::exp(-1 * prev._slope * input_sum));
            else
                next_value = 0;
            break;
        case EXCITORY_LINK:
            next_value = qBound(ZERO, input_sum * prev._weight, ONE);
            break;
        case INHIBITORY_LINK:
            if (input_sum > 0)
                next_value = static_cast<NeuroValue>(-100000000);
            else
                next_value = 0;
            break;
        default:
            break;
        }

        next._current_value = next_value;
    }
    
    
    QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc)
    {
        ds << static_cast<quint8>(nc._kind);
        ds << static_cast<bool>(nc._frozen);

        ds << static_cast<float>(nc._slope);

        ds << static_cast<float>(nc._current_value);

        return ds;
    }
    
    QDataStream & operator>> (QDataStream & ds, NeuroCell & nc)
    {
        quint8 k;
        float n;
        bool f;
        
        ds >> k; nc._kind = static_cast<NeuroCell::KindOfCell>(k);
        ds >> f; nc._frozen = static_cast<bool>(f);

        ds >> n; nc._slope = static_cast<NeuroCell::NeuroValue>(n);

        ds >> n; nc._current_value = static_cast<NeuroCell::NeuroValue>(n);
        
        return ds;
    }
    
} // namespace NeuroLib
