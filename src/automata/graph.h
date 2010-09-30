#ifndef GRAPH_H
#define GRAPH_H

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

#include <QVector>
#include <QDataStream>
#include <QReadWriteLock>

#include "exception.h"

namespace Automata
{

    /// Base class for graphs.
    /// \param TNode Type of node in the graph.
    /// \param TIndex Type to use as indices into the graph.
    template <typename TNode, typename TIndex = unsigned int>
    class Graph
    {
    protected:
        bool _directed;

        QVector<TNode> _nodes;
        QVector< QVector<TIndex> > _edges;
        QList<TIndex> _free_nodes;

        QReadWriteLock _nodes_lock;
        QReadWriteLock _edges_lock;

    public:
        /// Constructor.
        /// \param initialCapacity The number of nodes for which the graph object will initially reserve memory.
        /// \param directed Whether or not the graph is directed.  If it is NOT directed, Graph::addEdge() will
        /// add both incoming and outgoing edges.
        Graph(const int initialCapacity = 0, bool directed = false)
            : _directed(directed)
        {
            _nodes.reserve(initialCapacity);
            _edges.reserve(initialCapacity);
        }

        virtual ~Graph()
        {
        }

        /// Adds a node to the graph.
        /// \return The index of the newly-created node.
        TIndex addNode(const TNode & node)
        {
            QWriteLocker nwl(&_nodes_lock);
            QWriteLocker ewl(&_edges_lock);

            TIndex index;

            if (_free_nodes.size() > 0)
            {
                index = _free_nodes.last();
                _free_nodes.removeAt(_free_nodes.size() - 1);

                _nodes[index] = node;
            }
            else
            {
                index = _nodes.size();

                _nodes.append(node);
                _edges.append(QVector<TIndex>());
            }

            return index;
        }

        /// Removes a node from the graph.
        /// \note This can be quite expensive in a large graph.
        void removeNode(const TIndex & index)
        {
            QWriteLocker nwl(&_nodes_lock);
            QWriteLocker ewl(&_edges_lock);

            if (index < _edges.size())
            {
                _edges[index].clear();
                for (TIndex i = 0; i < _nodes.size(); ++i)
                {
                    int ii = _edges[i].indexOf(index);

                    if (ii != -1)
                        _edges[i].remove(ii);
                }
                _free_nodes.append(index);
            }
            else
            {
                throw IndexOverflow();
            }
        }

        /// Adds an edge to the graph.  If the graph is NOT directed, will also add a reciprocal edge.
        /// \param from The index of the source node.
        /// \param to The index of the destination node.
        /// \see Graph::removeEdge()
        void addEdge(const TIndex & from, const TIndex & to)
        {
            QWriteLocker ewl(&_edges_lock);

            if (from < _edges.size())
            {
                QVector<TIndex> & outgoing = _edges[from];

                if (!outgoing.contains(to))
                    outgoing.append(to);
            }
            else
            {
                throw IndexOverflow();
            }

            if (!_directed)
            {
                if (to < _edges.size())
                {
                    QVector<TIndex> & incoming = _edges[to];

                    if (!incoming.contains(from))
                        incoming.append(from);
                }
                else
                {
                    throw IndexOverflow();
                }
            }
        }

        /// Removes an edge from the graph.  If the graph is NOT directed, will also remove the reciprocal edge.
        /// \param from The index of the source node.
        /// \param to The index of the destination node.
        /// \see Graph::addEdge()
        void removeEdge(const TIndex & from, const TIndex & to)
        {
            QWriteLocker eql(&_edges_lock);

            if (from < _edges.size())
            {
                QVector<TIndex> & outgoing = _edges[from];
                int i = outgoing.indexOf(to);
                while (i != -1)
                {
                    outgoing.remove(i);
                    i = outgoing.indexOf(to);
                }
            }
            else
            {
                throw IndexOverflow();
            }

            if (!_directed)
            {
                if (to < _edges.size())
                {
                    QVector<TIndex> & incoming = _edges[to];
                    int i = incoming.indexOf(from);
                    while (i != -1)
                    {
                        incoming.remove(i);
                        i = incoming.indexOf(from);
                    }
                }
                else
                {
                    throw IndexOverflow();
                }
            }
        }

        /// \returns A const reference to a node in the graph.
        /// \param index The index of the node.
        const TNode & operator[] (const TIndex & index) const
        {
            if (index < _nodes.size())
                return _nodes[index];
            else
                throw IndexOverflow();
        }

        /// \returns A reference to a node in the graph.
        /// \param index The index of the node.
        TNode & operator[] (const TIndex & index)
        {
            if (index < _nodes.size())
                return _nodes[index];
            else
                throw IndexOverflow();
        }

        /// Returns a pointer to an array containing the indices of all the nodes to which there is an edge from
        /// the given node.
        /// \param index The index of the node.
        /// \param num Is set to the size of the array.
        TIndex * neighbors(const TIndex & index, int & num) const
        {
            if (index < _nodes.size())
            {
                const QList<TIndex> & nbrs = _edges[index];
                num = nbrs.size();
                return nbrs.data();
            }
            else
            {
                throw IndexOverflow();
            }
        }

        /// Writes the graph's data.  Should be called by derived classes' implementations.
        virtual void writeBinary(QDataStream & ds, const AutomataFileVersion & file_version) const
        {
            // directed flag
            ds << _directed;

            // nodes
            quint32 num = static_cast<quint32>(_nodes.size());
            ds << num;

            for (quint32 i = 0; i < num; ++i)
            {
                _nodes[i].writeBinary(ds, file_version);
            }

            // edges
            ds << _edges;

            // free nodes
            num = static_cast<quint32>(_free_nodes.size());
            ds << num;

            for (quint32 i = 0; i < num; ++i)
            {
                quint32 index = static_cast<quint32>(_free_nodes[i]);
                ds << index;
            }
        }

        /// Reads the graph's data.  Should be called by derived classes' implementations.
        virtual void readBinary(QDataStream & ds, const AutomataFileVersion & file_version)
        {
            if (file_version.automata_version >= Automata::AUTOMATA_FILE_VERSION_1)
            {
                ds >> _directed;

                // nodes
                quint32 num;
                ds >> num;

                _nodes.resize(num);
                for (quint32 i = 0; i < num; ++i)
                {
                    _nodes[i].readBinary(ds, file_version);
                }

                // edges
                ds >> _edges;

                if (file_version.automata_version >= Automata::AUTOMATA_FILE_VERSION_2)
                {
                    ds >> num;

                    _free_nodes.clear();
                    for (quint32 i = 0; i < num; ++i)
                    {
                        quint32 index;
                        ds >> index;

                        _free_nodes.append(static_cast<TIndex>(index));
                    }
                }
            }
            else // if (file_version.automata_version >= Automata::AUTOMATA_FILE_VERSION_OLD)
            {
                int version;
                ds >> version;

                // data
                ds >> this->_directed;
                ds >> this->_nodes;
                ds >> this->_edges;
            }
        }
    };

} // namespace Automata

#endif // GRAPH_H
