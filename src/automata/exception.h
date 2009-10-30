#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <QString>

#include "automata_global.h"

namespace Automata
{

    /// Base class for exceptions.
    class AUTOMATASHARED_EXPORT Exception
    {
        QString message;

    public:
        Exception(const QString & message) : message(message) {}

        const QString & Message() { return message; }
    };

    /// Index overflow exceptions.
    class AUTOMATASHARED_EXPORT IndexOverflow
        : public Exception
    {
    public:
        IndexOverflow() : Exception("Index overflow") {}
    };
    
    /// IO Exceptions
    class AUTOMATASHARED_EXPORT IOError
        : public Exception
    {
    public:
        IOError() : Exception("IO Error") {}
    };

} // namespace Automata

#endif // EXCEPTION_H
