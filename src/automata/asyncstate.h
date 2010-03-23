#ifndef ASYNCSTATE_H
#define ASYNCSTATE_H

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

        AsyncState & operator= (const AsyncState & state)
        {
            q0 = state.q0;
            q1 = state.q1;
            r = state.r;
            return *this;
        }

        const TState & current() const { return r == 0 ? q0 : q1; }
        TState & current() { return r == 0 ? q0 : q1; }

        const TState & former() const { return r == 0 ? q0 : q1; }
        TState & former() { return r == 0 ? q0 : q1; }
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
