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

#include "automata_global.h"
#include "graph.h"
#include "asyncstate.h"
#include "pool.h"

#include <QtGlobal>
#include <QtConcurrentMap>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

namespace Automata
{

    /// An automaton over graphs, with asynchronous update capability.
    /// \param TState A cell's state in the automaton.
    /// \param TIndex The type used to index cells in the automaton.
    /// \param NUM_PER_LOCK The number of cells per write lock.
    template <typename TState, typename TIndex = quint32>
    class Automaton
        : public Graph<AsyncState<TState, TIndex>, TIndex>
    {
        Pool< QVector<TState> > _temp_neighbor_pool;
        mutable QReadWriteLock _network_lock;

        /// \internal Used in the call to <tt>QtConcurrent::mapped()</tt>.
        struct MapFunctor
        {
            Automaton<TState, TIndex> & automaton;

        public:
            typedef AsyncState<TState, TIndex> & result_type;

            MapFunctor(Automaton<TState, TIndex> & automaton)
                : automaton(automaton) {}

            inline result_type operator() (result_type state)
            {
                automaton.update(state);
                return state;
            }
        };

        MapFunctor _functor;

    public:
        /// The node type for the graph.
        typedef AsyncState<TState, TIndex> ASYNC_STATE;

        /// Constructor.
        /// \param initialCapacity The number of cells for which the automaton will initially reserve memory.
        /// \param directed Whether or not the automaton's graph is directed.
        Automaton(const int initialCapacity = 0, bool directed = true)
            : Graph<ASYNC_STATE, TIndex>(initialCapacity, directed), _functor(*this)
        {
        }

        /// Destructor.
        virtual ~Automaton()
        {
        }

        /// Causes the asynchronous automaton to be advanced by one-third of a timestep.
        inline void step()
        {
            stepAsync().waitForFinished();
        }

        /// Causes the asynchronous automaton to be advanced by one-third of a timestep.
        /// Calling code must wait for the future to be finished.
        inline QFuture<void> stepAsync()
        {
            MapFunctor functor(*this);
            return QtConcurrent::map(this->_nodes, functor);
        }

        /// Adds a cell to the automaton.
        /// \return The index of the newly-created cell.
        TIndex addNode(const TState & node)
        {
            TIndex result = Graph<ASYNC_STATE, TIndex>::addNode(ASYNC_STATE(node, node));
            return result;
        }

        /// The ready state of a cell in the automaton.  Used to track asynchronous updates.
        /// \note This is not thread-safe; use only for imprecise visualization.
        /// \return The asynchronous ready state of a cell in the automaton.
        int readyState(const TIndex & index) const
        {
            if (index < this->_nodes.size())
                return this->_nodes[index].r;
            else
                throw IndexOverflow();
        }

    protected:
        typedef Automaton<TState, TIndex> BASE;

        /// Implements the asynchronous update operation on a cell in the automaton.
        inline void update(ASYNC_STATE & state)
        {
            const TIndex & index = &state - this->_nodes.data();
            const QVector<TIndex> & neighbors = this->_edges[index];

            // update
            if (state.r == 0)
            {
                // temporary neighbor array
                typename Pool< QVector<TState> >::Item tni(_temp_neighbor_pool);
                QVector<TState> *temp_neighbors = tni.data;
                TState *temp_ptr = 0;

                bool do_update = true;
                {
                    QReadLocker read_lock(&_network_lock);

                    if (isReady(index, 0))
                    {
                        do_update = true;

                        if (neighbors.size() > temp_neighbors->size())
                            temp_neighbors->resize(neighbors.size());
                        temp_ptr = temp_neighbors->data();

                        // get appropriate states of neighbors
                        for (int i = 0; i < neighbors.size(); ++i)
                        {
                            const ASYNC_STATE & neighbor = this->_nodes[neighbors[i]];

                            if (neighbor.r == 0)
                                temp_ptr[i] = neighbor.q0;
                            else if (neighbor.r == 1)
                                temp_ptr[i] = neighbor.q1;
                        }
                    }
                }

                    // update
                if (do_update)
                {
                    TState q0 = state.q0;
                    TState q1 = q0;

                    q1.update(this, index, q0, neighbors, temp_ptr);

                    QWriteLocker write_lock(&_network_lock);
                    state.q0 = q0;
                    state.q1 = q1;
                    state.r = 1;
                }
            }
            else
            {
                bool do_update;
                {
                    QReadLocker read_lock(&_network_lock);
                    do_update = isReady(index, state.r);
                }

                if (do_update)
                {
                    QWriteLocker write_lock(&_network_lock);
                    state.r = (state.r+1) % 3;
                }
            }
        }

    private:
        /// Used for asynchronous updates.
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
