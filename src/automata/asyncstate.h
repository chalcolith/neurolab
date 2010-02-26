#ifndef ASYNCSTATE_H
#define ASYNCSTATE_H

#include <QtGlobal>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <QDataStream>

namespace Automata
{

    /// State for an asynchronous automaton.
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
    };
    
    template <typename TState, typename TIndex>
    QDataStream & operator<< (QDataStream & ds, const AsyncState<TState, TIndex> & as)
    {
        ds << as.q0;
        ds << as.q1;
        ds << as.r;
        
        return ds;
    }
    
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
