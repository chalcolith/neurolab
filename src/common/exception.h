#ifndef NEUROLAB_EXCEPTION_H
#define NEUROLAB_EXCEPTION_H

/*
Neurocognitive Linguistics Lab
Copyright (c) 2010,2011 Gordon Tisher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

 - Neither the name of the Neurocognitive Linguistics Lab nor the
   names of its contributors may be used to endorse or promote
   products derived from this software without specific prior
   written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include "common.h"
#include <QtCore>
#include <QString>

namespace Common
{

    /// Base class for exceptions.
    class COMMONSHARED_EXPORT Exception
        : public QtConcurrent::Exception
    {
        QString _message;

    public:
        /// Constructor.
        /// \param message A messsage to display.
        Exception(const QString & message);
        Exception(const Exception & e);
        virtual ~Exception() throw () {}

        const QString & message() const { return _message; }

        void raise() const { throw *this; }
        QtConcurrent::Exception *clone() const { return new Exception(*this); }
    };

    /// Index overflow exception.
    class COMMONSHARED_EXPORT IndexOverflow
        : public Exception
    {
    public:
        IndexOverflow() : Exception(QObject::tr("Index overflow")) {}
        IndexOverflow(const IndexOverflow & e) : Exception(e) {}
        virtual ~IndexOverflow() throw() {}

        void raise() const { throw *this; }
        QtConcurrent::Exception *clone() const { return new IndexOverflow(*this); }
    };

    /// IO Exception.
    class COMMONSHARED_EXPORT IOError
        : public Exception
    {
    public:
        IOError() : Exception(QObject::tr("IO Error")) {}
        IOError(const QString & message) : Exception(message) {}
        IOError(const IOError & e) : Exception(e) {}
        virtual ~IOError() throw() {}

        void raise() const { throw *this; }
        QtConcurrent::Exception *clone() const { return new IOError(*this); }
    };

    /// File format exception.
    class COMMONSHARED_EXPORT FileFormatError
        : public IOError
    {
    public:
        FileFormatError() : IOError(QObject::tr("Unrecognized file format")) {}
        FileFormatError(const QString & message) : IOError(message) {}
        FileFormatError(const FileFormatError & e) : IOError(e) {}
        virtual ~FileFormatError() throw() {}

        void raise() const { throw *this; }
        QtConcurrent::Exception *clone() const { return new FileFormatError(*this); }
    };

} // namespace Common

#endif
