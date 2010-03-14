#include "neuronet.h"
#include "../automata/exception.h"

namespace NeuroLib
{

    static const QString NETWORK_COOKIE("NeuroLib NETWORK 010");

    NeuroNet::NeuroNet()
        : BASE(),
        _decay(0.1f),
        _learn(0.1f),
        _learn_time(10.0f)
    {
    }

    QDataStream & NeuroNet::writeBinary(QDataStream & ds) const
    {
        ds << NETWORK_COOKIE;
        ds << static_cast<float>(_decay);
        ds << static_cast<float>(_learn);
        ds << static_cast<float>(_learn_time);
        
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
     
        float n;
        ds >> n; _decay = static_cast<NeuroCell::NeuroValue>(n);
        ds >> n; _learn = static_cast<NeuroCell::NeuroValue>(n);
        ds >> n; _learn_time = static_cast<NeuroCell::NeuroValue>(n);
        
        return BASE::readBinary(ds);
    }
    
} // namespace NeuroLib
