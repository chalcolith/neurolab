#ifndef AUTOMATON_H
#define AUTOMATON_H

#include <QtGlobal>
#include <QtConcurrentMap>
#include <QVarLengthArray>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

#include "automata_global.h"
#include "graph.h"
#include "asyncstate.h"

namespace Automata
{

    /// An automaton over graphs, with asynchronous update capability.
    /// \param TState A state in the automaton.  Must implement \c bool \c shouldUpdate().
    /// \param TFUpdate A functor by which to update states.  Must implement \c ()(const TState & prev, TState & next, const int & numNeighbors, const TState * const * const neighbors).
    template <typename TState, typename TFUpdate, typename TIndex = qint32, int NUM_PER_LOCK = 1024>
    class Automaton 
        : public Graph<AsyncState<TState, TIndex>, TIndex>
    {
        const TFUpdate fUpdate;
        QVector<const TState *> temp_neighbors;
        QVector<QReadWriteLock *> locks;
        
        struct MapFunctor
        {
            Automaton<TState, TFUpdate, TIndex> & automaton;
            
        public:
            typedef AsyncState<TState, TIndex> & result_type;
            
            MapFunctor(Automaton<TState, TFUpdate, TIndex> & automaton) : automaton(automaton) {}
            
            inline result_type operator() (result_type item)
            {
                return automaton.update(item);
            }
        };

    public:
        Automaton(const int initialCapacity = 0, bool directed = true)
            : Graph<AsyncState<TState, TIndex>, TIndex>(initialCapacity, directed), fUpdate(TFUpdate())
        {
        }

        virtual ~Automaton()
        {
            const int n = locks.size();
            for (int i = 0; i < n; ++i)
                delete locks[i];
        }
        
        virtual void step()
        {
            MapFunctor functor(*this);
            QFuture<void> future = QtConcurrent::map(this->_nodes, functor);
            future.waitForFinished();
        }

        TIndex addNode(const TState & node)
        {
            TIndex result = Graph<AsyncState<TState,TIndex>, TIndex>::addNode(AsyncState<TState,TIndex>(node, node));            
            return result;
        }

        const TState & operator[](const TIndex & index) const
        {
            if (index < this->_nodes.size())
                return this->_nodes[index].q0;
            else
                throw IndexOverflow();
        }

        TState & operator[](const TIndex & index)
        {
            if (index < this->_nodes.size())
                return this->_nodes[index].q0;
            else
                throw IndexOverflow();
        }

        const int & readyState(const TIndex & index) const
        {
            if (index < this->_nodes.size())
                return this->_nodes[index].r;
            else
                throw IndexOverflow();
        }

    protected:
        typedef Automaton<TState, TFUpdate, TIndex> BASE;

        inline AsyncState<TState, TIndex> & update(const TIndex & index)
        {
            return update(this->_nodes[index]);
        }
        
        inline AsyncState<TState, TIndex> & update(AsyncState<TState, TIndex> & state)
        {
            TIndex index = &state - this->_nodes.data();
            
            // write lock
            TIndex lock_index = index/NUM_PER_LOCK;            
            while (locks.size() <= lock_index)
                locks.append(new QReadWriteLock());
            QWriteLocker write(locks[lock_index]);
            
            // update
            if (state.r == 0)
            {
                if (isReady(index, 0))
                {
                    // temporary neighbor array
                    const QVector<TIndex> & neighbors = this->_edges[index];
                    if (neighbors.size() > temp_neighbors.size())
                        temp_neighbors.resize(neighbors.size());
                    
                    // get appropriate states of neighbors
                    const TState ** const temp_ptr = temp_neighbors.data();
                    for (int i = 0; i < neighbors.size(); ++i)
                    {
                        const AsyncState<TState, TIndex> & neighbor = this->_nodes[neighbors[i]];
                        
                        if (neighbor.r == 0)
                            temp_ptr[i] = &neighbor.q0;
                        else if (neighbor.r == 1)
                            temp_ptr[i] = &neighbor.q1;
                    }
                    
                    // update
                    state.q1 = state.q0;
                    fUpdate(this, index, state.q1, state.q0, neighbors.size(), temp_ptr);
                    state.r = 1;                    
                }
            }
            else if (isReady(index, state.r))
            {
                state.r = (state.r+1) % 3;
            }
            
            return state;
        }

        inline bool isReady(const TIndex & index, const int & r) const
        {
            if (index < this->_nodes.size())
            {
                const QVector<TIndex> & neighbors = this->_edges[index];
                
                for (int i = 0; i < neighbors.size(); ++i)
                {
                    if (this->_nodes[neighbors[i]].r == ((r+2) % 3))
                        return false;
                }
                
                return true;
            }
            else
            {
                throw IndexOverflow();
            }
        }
                
    }; // class Automaton
    
} // namespace Automata

#endif // AUTOMATON_H
