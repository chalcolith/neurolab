#include "neurocell.h"

namespace NeuroLib
{
    
    NeuroCell::NeuroCell()
    {
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
