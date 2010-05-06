#ifndef ASYNCSTATE_H
#define ASYNCSTATE_H

/*
Neurocognitive Linguistics Lab
Copyright (c) 2010, Gordon Tisher
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
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <QDataStream>

namespace Automata
{

    /// Internal state for an asynchronous automaton, which requires two copies of a cell's state,
    /// one for the previous state, and one for the next state.
    /// \param TState A cell's state.
    /// \param TIndex The index type used by the automaton.
    template <typename TState, typename TIndex>
    struct AsyncState
    {
        TState q0, q1;
        int r;

        AsyncState() : r(0) {}
        AsyncState(const TState & s0, const TState & s1) : q0(s0), q1(s1), r(0) {}
        AsyncState(const AsyncState & state) : q0(state.q0), q1(state.q1), r(state.r) {}
        virtual ~AsyncState() {}

        AsyncState & operator= (const AsyncState & state)
        {
            q0 = state.q0;
            q1 = state.q1;
            r = state.r;
            return *this;
        }

        //@{
        /// The current state of the cell.
        const TState & current() const { return r == 0 ? q0 : q1; }
        TState & current() { return r == 0 ? q0 : q1; }
        //@}

        //@{
        /// The former state of the cell.
        const TState & former() const { return r == 0 ? q0 : q1; }
        TState & former() { return r == 0 ? q0 : q1; }
        //@}
    };

    /// Writes the asynchronous cell to a QDataStream.  Writes both the previous and current state objects,
    /// so the automaton may be saved or loaded in the middle of an asynchronous update.
    template <typename TState, typename TIndex>
    QDataStream & operator<< (QDataStream & ds, const AsyncState<TState, TIndex> & as)
    {
        ds << as.q0;
        ds << as.q1;
        ds << as.r;

        return ds;
    }

    /// Reads the asynchronous cell from a QDataStream.  Reads both the previous and current state objects,
    /// so the automaton may be saved or loaded in the middle of an asynchronous update.
    template <typename TState, typename TIndex>
    QDataStream & operator>> (QDataStream & ds, AsyncState<TState, TIndex> & as)
    {
        ds >> as.q0;
        ds >> as.q1;
        ds >> as.r;

        return ds;
    }

} // namespace Automata


#endif // ASYNCSTATE_H
