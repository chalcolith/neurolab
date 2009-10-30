#ifndef AUTOMATON_H
#define AUTOMATON_H

#include <QtGlobal>
#include <QtConcurrentMap>
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
    template <typename TState, typename TFUpdate, typename TIndex = qint32>
    class Automaton : public Graph<AsyncState<TState, TIndex>, TIndex>
    {
        const TFUpdate fUpdate;
        QVector<const TState *> temp_neighbors;
        
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
        }
        
        virtual void step()
        {
            MapFunctor functor(*this);
            QFuture<void> future = QtConcurrent::map(this->nodes, functor);
            future.waitForFinished();
        }

        TIndex addNode(const TState & node)
        {
            return Graph<AsyncState<TState,TIndex>, TIndex>::addNode(AsyncState<TState,TIndex>(node, node));
        }

        const TState & operator[](const TIndex & index) const
        {
            if (index < this->nodes.size())
                return this->nodes[index].q1;
            else
                throw IndexOverflow();
        }

        TState & operator[](const TIndex & index)
        {
            if (index < this->nodes.size())
                return this->nodes[index].q1;
            else
                throw IndexOverflow();
        }

        const int & readyState(const TIndex & index) const
        {
            if (index < this->nodes.size())
                return this->nodes[index].r;
            else
                throw IndexOverflow();
        }

    protected:
        typedef Automaton<TState, TFUpdate, TIndex> BASE;

        inline AsyncState<TState, TIndex> & update(const TIndex & index)
        {
            return update(this->nodes[index]);
        }
        
        inline AsyncState<TState, TIndex> & update(AsyncState<TState, TIndex> & state)
        {
            //QWriteLocker write(&state.lock);
            
            if (state.r == 0)
            {
                if (isReady(state.index, 0))
                {
                    const QVector<TIndex> & neighbors = this->edges[state.index];
                    if (neighbors.size() > temp_neighbors.size())
                        temp_neighbors.resize(neighbors.size());
                    
                    const TState ** const temp_ptr = temp_neighbors.data();
                    for (int i = 0; i < neighbors.size(); ++i)
                    {
                        const AsyncState<TState, TIndex> & neighbor = this->nodes[neighbors[i]];
                        
                        //QReadLocker read(&neighbor.lock);
                        
                        if (neighbor.r == 0)
                            temp_ptr[i] = &neighbor.q0;
                        else if (neighbor.r == 1)
                            temp_ptr[i] = &neighbor.q1;
                    }
                    
                    // update
                    state.q1 = state.q0;
                    fUpdate(state.q1, state.q0, neighbors.size(), temp_ptr);
                    state.r = 1;                    
                }
            }
            else if (isReady(state.index, state.r))
            {
                state.r = (state.r+1) % 3;
            }
            
            return state;
        }

        inline bool isReady(const TIndex & index, const int & r) const
        {
            if (index < this->nodes.size())
            {
                const QVector<TIndex> & neighbors = this->edges[index];
                
                for (int i = 0; i < neighbors.size(); ++i)
                {
                    if (this->nodes[neighbors[i]].r == ((r+2) % 3))
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
