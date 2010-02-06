#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <QString>

#include "automata_global.h"

namespace Automata
{

    /// Base class for exceptions.
    class AUTOMATASHARED_EXPORT Exception
    {
        QString _message;

    public:
        Exception(const QString & message) : _message(message) {}
        const QString & message() const { return _message; }
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
