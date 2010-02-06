#include "neurocell.h"
#include "neuronet.h"

#include "../automata/exception.h"

namespace NeuroLib
{
    
    NeuroCell::NeuroCell(const Kind & k, const NeuroValue & input_threshold)
        : _kind(k), _value(0), _input_threshold(input_threshold)
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
    
    QDataStream & operator<< (QDataStream & ds, const NeuroCell & nc)
    {
        ds.setVersion(QDataStream::Qt_4_5);
        return ds;
    }
    
    QDataStream & operator>> (QDataStream & ds, NeuroCell & nc)
    {
        ds.setVersion(QDataStream::Qt_4_5);
        return ds;
    }
    
} // namespace NeuroLib
