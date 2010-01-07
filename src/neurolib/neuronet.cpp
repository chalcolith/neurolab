#include "neuronet.h"
#include "../automata/exception.h"

namespace NeuroLib
{

    static const QString NETWORK_COOKIE("NeuroLib 0 NETWORK");

    NeuroNet::NeuroNet()
            : BASE()
    {
    }

    QDataStream & NeuroNet::writeBinary(QDataStream & ds) const
    {
        ds << NETWORK_COOKIE;
        return BASE::writeBinary(ds);
    }
    
    QDataStream & NeuroNet::readBinary(QDataStream & ds)
    {
        QString cookie;
        ds >> cookie;

        if (cookie != NETWORK_COOKIE)
        {
            throw Automata::Exception(QObject::tr("Network file is not compatible with this version of NeuroLib."));
        }
                
        return BASE::readBinary(ds);
    }
    
} // namespace NeuroLib
