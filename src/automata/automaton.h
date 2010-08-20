#ifndef AUTOMATON_H
#define AUTOMATON_H

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
#include <QtConcurrentMap>
#include <QVarLengthArray>
#include <QReadWriteLock>

#include "automata_global.h"
#include "pool.h"
#include "graph.h"
#include "asyncstate.h"

namespace Automata
{

    /// An automaton over graphs, with asynchronous update capability.
    /// \param TState A cell's state in the automaton.
    /// \param TFUpdate A functor by which to update states.  Must implement <tt>void operator() (Automata::Automaton<NeuroCell, NeuroCell::Update, NeuroCell::NeuroIndex> *neuronet, const NeuroIndex & index, const NeuroCell & prev, NeuroCell & next, const QVector<int> & neighbor_indices, const NeuroCell * const * const neighbors) const</tt>.
    /// \param TIndex The type used to index cells in the automaton.
    /// \param NUM_PER_LOCK The number of cells per write lock.
    template <typename TState, typename TFUpdate, typename TIndex = quint32, int NUM_PER_LOCK = 1024>
    class Automaton
        : public Graph<AsyncState<TState, TIndex>, TIndex>
    {
        const TFUpdate fUpdate;
        Pool<QVector<const TState *> > temp_neighbor_pool;
        QVector<QReadWriteLock *> locks;

        /// \internal Used in the call to QtConcurrent::map()
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
        typedef AsyncState<TState, TIndex> ASYNC_STATE;

        /// Constructor.
        /// \param initialCapacity The number of cells for which the automaton will initially reserve memory.
        /// \param directed Whether or not the automaton's graph is directed.
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

        /// Causes the asynchronous automaton to be advanced by one-third of a timestep.
        virtual void step()
        {
            // the asynchronous algo take three updates to fully run one step
            MapFunctor functor(*this);
            QFuture<void> future1 = QtConcurrent::map(this->_nodes, functor);
            future1.waitForFinished();
        }

        /// Causes the asynchronous automaton to be advanced by one-third of a timestep.
        virtual QFuture<void> stepAsync()
        {
            MapFunctor functor(*this);
            return QtConcurrent::map(this->_nodes, functor);
        }

        /// Adds a cell to the automaton.
        /// \return The index of the newly-created cell.
        TIndex addNode(const TState & node)
        {
            TIndex result = Graph<AsyncState<TState,TIndex>, TIndex>::addNode(AsyncState<TState,TIndex>(node, node));
            return result;
        }

        /// \return The asynchronous ready state of a cell in the automaton.
        int readyState(const TIndex & index) const
        {
            if (index < this->_nodes.size())
                return this->_nodes[index].r;
            else
                throw IndexOverflow();
        }

        /// \return The lock that governs a cell with the given index.
        QReadWriteLock *getLock(const TIndex & index)
        {
            TIndex lock_index = index/NUM_PER_LOCK;
            while (locks.size() <= lock_index)
                locks.append(new QReadWriteLock(QReadWriteLock::Recursive));
            return locks[lock_index];
        }

    protected:
        typedef Automaton<TState, TFUpdate, TIndex> BASE;

        //@{
        /// Implements the asynchronous update operation on a cell in the automaton.
        inline AsyncState<TState, TIndex> & update(const TIndex & index)
        {
            return update(this->_nodes[index]);
        }

        inline AsyncState<TState, TIndex> & update(AsyncState<TState, TIndex> & state)
        {
            TIndex index = &state - this->_nodes.data();

            // write lock
            getLock(index)->lockForWrite();

            try
            {
                // update
                if (state.r == 0)
                {
                    if (isReady(index, 0))
                    {
                        // temporary neighbor array
                        typename Pool<QVector<const TState *> >::Item tni(temp_neighbor_pool);
                        QVector<const TState *> *temp_neighbors = tni.data;

                        const QVector<TIndex> & neighbors = this->_edges[index];
                        if (neighbors.size() > temp_neighbors->size())
                            temp_neighbors->resize(neighbors.size());

                        // get appropriate states of neighbors
                        const TState ** const temp_ptr = temp_neighbors->data();
                        for (int i = 0; i < neighbors.size(); ++i)
                        {
                            getLock(neighbors[i])->lockForWrite();
                            const AsyncState<TState, TIndex> & neighbor = this->_nodes[neighbors[i]];

                            if (neighbor.r == 0)
                                temp_ptr[i] = &neighbor.q0;
                            else if (neighbor.r == 1)
                                temp_ptr[i] = &neighbor.q1;
                        }

                        // update
                        state.q1 = state.q0;
                        try
                        {
                            fUpdate(this, index, state.q1, state.q0, neighbors, temp_ptr);
                        }
                        catch (...)
                        {
                            for (int i = 0; i < neighbors.size(); ++i)
                                getLock(neighbors[i])->unlock();
                            throw;
                        }

                        state.r = 1;

                        // unlock
                        for (int i = 0; i < neighbors.size(); ++i)
                            getLock(neighbors[i])->unlock();
                    }
                }
                else if (isReady(index, state.r))
                {
                    state.r = (state.r+1) % 3;
                }
            }
            catch (...)
            {
                getLock(index)->unlock();
                throw;
            }

            // unlock and return
            getLock(index)->unlock();

            return state;
        }
        //@}

        /// \return Whether or not a given cell is ready to be updated.
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
