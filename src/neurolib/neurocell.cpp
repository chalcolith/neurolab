#include "neurocell.h"
#include "neuronet.h"
#include "../automata/exception.h"
#include <cmath>

namespace NeuroLib
{
    
    NeuroCell::NeuroCell(const Kind & k, const NeuroValue & input_threshold)
        : _kind(k), _value(0), _input_threshold(input_threshold), _frozen(false)
    {
        if (input_threshold == 0 && _kind != INHIBITORY_LINK)
            throw Automata::Exception(QObject::tr("Input threshold must be greater than zero."));
    }
        
    void NeuroCell::addInput(NeuroNet *network, const NeuroIndex & my_index, const NeuroIndex & input_index)
    {
        network->addEdge(my_index, input_index);
    }
    
    void NeuroCell::removeInput(NeuroNet *network, const NeuroIndex & my_index, const NeuroIndex & input_index)
    {
        network->removeEdge(my_index, input_index);
    }

    void NeuroCell::Update::operator() (const NeuroCell & prev, NeuroCell & next, const int & numNeighbors, const NeuroCell * const * const neighbors) const
    {
        next._frozen = prev._frozen;
        
        if (prev._frozen)
        {
            next._value = prev._value;
            return;
        }
        
        NeuroValue input_sum = 0;
        for (int i = 0; i < numNeighbors; ++i)
            input_sum += neighbors[i]->_value;        
        input_sum = qBound(static_cast<NeuroValue>(0), input_sum, static_cast<NeuroValue>(1));
        
        NeuroValue next_value, prev_value;
        
        switch (prev._kind)
        {
        case NODE:
        case EXCITORY_LINK:
            prev_value = qBound(static_cast<NeuroValue>(0), prev._value - 1, static_cast<NeuroValue>(1));            
            next_value = static_cast<NeuroValue>(1) 
                         / (static_cast<NeuroValue>(1) 
                            + ::exp(static_cast<NeuroValue>(4) - static_cast<NeuroValue>(8) * input_sum / prev._input_threshold));
            
            if (prev._kind == NODE)
                next._value = qMax(prev_value, next_value);
            else
                next._value = next_value;
            break;
        case INHIBITORY_LINK:
            next._value = prev._input_threshold ? -input_sum : static_cast<NeuroValue>(-1000);                    
            break;
        default:
            return;
        }
    }
    
    
    QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc)
    {
        ds.setVersion(QDataStream::Qt_4_5);
        ds << static_cast<qint32>(nc._kind);
        ds << static_cast<double>(nc._value);
        ds << static_cast<double>(nc._input_threshold);
        ds << static_cast<bool>(nc._frozen);
        return ds;
    }
    
    QDataStream & operator>> (QDataStream & ds, NeuroCell & nc)
    {
        qint32 k;
        double n;
        bool f;
        
        ds.setVersion(QDataStream::Qt_4_5);
        ds >> k; nc._kind = static_cast<NeuroCell::Kind>(k);
        ds >> n; nc._value = static_cast<NeuroCell::NeuroValue>(n);
        ds >> n; nc._input_threshold = static_cast<NeuroCell::NeuroValue>(n);
        ds >> f; nc._frozen = static_cast<bool>(f);
        
        return ds;
    }
    
} // namespace NeuroLib
