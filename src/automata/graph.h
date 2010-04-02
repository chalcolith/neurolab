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
#include <QHash>
#include <QDataStream>

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
        bool directed;

        QVector<TNode> _nodes;
        QVector< QVector<TIndex> > _edges;

    public:
        /// Constructor.
        /// \param initialCapacity The number of nodes for which the graph object will initially reserve memory.
        /// \param directed Whether or not the graph is directed.  If it is NOT directed, Graph::addEdge() will
        /// add both incoming and outgoing edges.
        Graph(const int initialCapacity = 0, bool directed = false)
            : directed(directed)
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
            TIndex index = _nodes.size();

            _nodes.append(node);
            _edges.append(QVector<TIndex>());

            return index;
        }

        /// Adds an edge to the graph.  If the graph is NOT directed, will also add a reciprocal edge.
        /// \param from The index of the source node.
        /// \param to The index of the destination node.
        /// \see Graph::removeEdge()
        void addEdge(const TIndex & from, const TIndex & to)
        {
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

            if (!directed)
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

            if (!directed)
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
                const QVector<TIndex> & nbrs = _edges[index];
                num = nbrs.size();
                return nbrs.data();
            }
            else
            {
                throw IndexOverflow();
            }
        }


        /// Implemented by derived classes to write to a data stream.
        virtual QDataStream & writeBinary(QDataStream & ds) const
        {
            ds.setVersion(QDataStream::Qt_4_5);

            int version = 1;
            ds << version;

            // data
            ds << this->directed;
            ds << this->_nodes;
            ds << this->_edges;

            return ds;
        }

        /// Implemented by derived classes to read from a data stream.
        virtual QDataStream & readBinary(QDataStream & ds)
        {
            ds.setVersion(QDataStream::Qt_4_5);

            int version;
            ds >> version;

            // data
            ds >> this->directed;
            ds >> this->_nodes;
            ds >> this->_edges;

            return ds;
        }

        template <typename TN, typename TI>
        friend QDataStream & operator<< (QDataStream &, const Graph<TN, TI> &);

        template <typename TN, typename TI>
        friend QDataStream & operator>> (QDataStream &, Graph<TN, TI> &);
    };


    /// Writes to a data stream.  Derived classes should not re-implement this, but implement Graph::writeBinary() instead.
    template <typename TNode, typename TIndex>
    QDataStream & operator<< (QDataStream & s, const Graph<TNode, TIndex> & g)
    {
        return g.writeBinary(s);
    }

    /// Reads from a data stream.  Derived classes should not re-implement this, but implement Graph::readBinary() instead.
    template <typename TNode, typename TIndex>
    QDataStream & operator>> (QDataStream & s, Graph<TNode, TIndex> & g)
    {
        return g.readBinary(s);
    }

} // namespace Automata

#endif // GRAPH_H
