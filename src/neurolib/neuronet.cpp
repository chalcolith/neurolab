#include "neuronet.h"

namespace NeuroLib
{

    NeuroNet::NeuroNet()
            : BASE()
    {
    }

    QDataStream & NeuroNet::writeBinary(QDataStream & ds) const
    {
        return BASE::writeBinary(ds);
    }
    
    QDataStream & NeuroNet::readBinary(QDataStream & ds)
    {
        return BASE::readBinary(ds);
    }
    
} // namespace NeuroLib
