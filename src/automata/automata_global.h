#ifndef AUTOMATA_GLOBAL_H
#define AUTOMATA_GLOBAL_H

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

#include <QtGlobal>
#include "../common/common.h"

#if defined(AUTOMATA_LIBRARY)
#  define AUTOMATASHARED_EXPORT Q_DECL_EXPORT
#else
#  define AUTOMATASHARED_EXPORT Q_DECL_IMPORT
#endif

/// Generic asynchronous automata over graphs.
namespace Automata
{

    /// Used to keep track of file versions for automata files.
    struct AUTOMATASHARED_EXPORT AutomataFileVersion
    {
        quint16 automata_version; ///< The version of the \ref Automata::Automaton code.
        quint16 client_version;   ///< Used by subclasses to store version info.
    };

    /// Version number for writing automata.
    enum AutomataVersionNumber
    {
        AUTOMATA_FILE_VERSION_OLD = 0,
        AUTOMATA_FILE_VERSION_1   = 1,
        AUTOMATA_FILE_VERSION_2   = 2,
        AUTOMATA_NUM_FILE_VERSIONS
    };

} // namespace Automata

#endif // AUTOMATA_GLOBAL_H
