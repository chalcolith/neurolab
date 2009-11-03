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
        const TIndex index;
        TState q0, q1;
        int r;
        
        mutable QReadWriteLock lock;

        AsyncState() : index(static_cast<TIndex>(-1)), r(0) {}
        AsyncState(const TState & s0, const TState & s1) : index(static_cast<TIndex>(-1)), q0(s0), q1(s1), r(0) {}
        AsyncState(const AsyncState & state) : index(state.index), q0(state.q0), q1(state.q1), r(state.r) {}

        AsyncState & operator= (const AsyncState & state)
        {
            *const_cast<TIndex*>(&index) = state.index;
            q0 = state.q0;
            q1 = state.q1;
            r = state.r;
            return *this;
        }

        void setIndex(const TIndex & index)
        {
            *const_cast<TIndex*>(&this->index) = index;
        }
    };
    
    template <typename TState, typename TIndex>
    QDataStream & operator<< (QDataStream & ds, const AsyncState<TState, TIndex> & as)
    {
        QReadLocker read(&as.lock);
        
        ds.setVersion(QDataStream::Qt_4_5);
        
        ds << as.index;
        ds << as.q0;
        ds << as.q1;
        ds << as.r;
        
        return ds;
    }
    
    template <typename TState, typename TIndex>
    QDataStream & operator>> (QDataStream & ds, AsyncState<TState, TIndex> & as)
    {
        QWriteLocker write(&as.lock);
        
        ds.setVersion(QDataStream::Qt_4_5);
        
        qint32 & idx = const_cast<qint32 &>(as.index);
        
        ds >> idx;
        ds >> as.q0;
        ds >> as.q1;
        ds >> as.r;
        
        return ds;
    }
    
} // namespace Automata


#endif // ASYNCSTATE_H
