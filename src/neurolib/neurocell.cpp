#include "neurocell.h"
#include "neuronet.h"
#include "../automata/exception.h"
#include <cmath>

namespace NeuroLib
{
    
    NeuroCell::NeuroCell(const KindOfCell & k, const NeuroValue & input_threshold, const NeuroValue & output_weight, const NeuroValue & output_value)
        : _kind(k), _input_threshold(input_threshold), _output_weight(output_weight), _output_value(output_value), _frozen(false)
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

    void NeuroCell::Update::operator() (Automata::Automaton<NeuroCell, NeuroCell::Update, NeuroCell::NeuroIndex> *, const NeuroIndex &, const NeuroCell & prev, NeuroCell & next, const int & numNeighbors, const NeuroCell * const * const neighbors) const
    {
        next._frozen = prev._frozen;
        
        if (prev._frozen)
        {
            next._output_value = prev._output_value;
            return;
        }
        
        NeuroValue input_sum = 0;
        for (int i = 0; i < numNeighbors; ++i)
            input_sum += neighbors[i]->_output_value;        
        input_sum = qBound(static_cast<NeuroValue>(0), input_sum, static_cast<NeuroValue>(1));
        
        NeuroValue sign = prev._output_weight < 0 ? -1 : 1;

        NeuroValue prev_value = qBound(static_cast<NeuroValue>(0), qAbs(prev._output_value) - 1, static_cast<NeuroValue>(1));        
        NeuroValue next_value = 
            (input_sum > 0) 
                ? static_cast<NeuroValue>(1) / (static_cast<NeuroValue>(1) + ::exp(static_cast<NeuroValue>(4) - static_cast<NeuroValue>(8) * input_sum / prev._input_threshold))
                : 0;

        next_value *= prev._output_weight * sign;
        
        next._output_value = qMax(prev_value, next_value) * sign;
    }
    
    
    QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc)
    {
        ds << static_cast<qint32>(nc._kind);
        ds << static_cast<float>(nc._input_threshold);
        ds << static_cast<float>(nc._output_weight);
        ds << static_cast<float>(nc._output_value);
        ds << static_cast<bool>(nc._frozen);
        return ds;
    }
    
    QDataStream & operator>> (QDataStream & ds, NeuroCell & nc)
    {
        qint32 k;
        float n;
        bool f;
        
        ds >> k; nc._kind = static_cast<NeuroCell::KindOfCell>(k);
        ds >> n; nc._input_threshold = static_cast<NeuroCell::NeuroValue>(n);
        ds >> n; nc._output_weight = static_cast<NeuroCell::NeuroValue>(n);
        ds >> n; nc._output_value = static_cast<NeuroCell::NeuroValue>(n);
        ds >> f; nc._frozen = static_cast<bool>(f);
        
        return ds;
    }
    
} // namespace NeuroLib
