#ifndef GRAPH_H
#define GRAPH_H

#include <QVector>
#include <QHash>
#include <QDataStream>

#include "exception.h"

namespace Automata
{

    //////////////////////////////////////////////////////////////////
    /// Base class for graphs.
    /// \param TNode Type of node in the graph.  Must implement \c void \c setIndex(const TIndex &).
    template <typename TNode, typename TIndex = unsigned int>
    class Graph
    {
    protected:
        bool directed;
        
        QVector<TNode> _nodes;
        QVector< QVector<TIndex> > _edges;

    public:
        Graph(const int initialCapacity = 0, bool directed = false)
            : directed(directed)
        {
            _nodes.reserve(initialCapacity);
            _edges.reserve(initialCapacity);
        }

        virtual ~Graph()
        {
        }

        TIndex addNode(const TNode & node)
        {
            TIndex index = _nodes.size();

            _nodes.append(node);
            _nodes[index].setIndex(index);

            _edges.append(QVector<TIndex>());
            
            return index;
        }
        
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

        const TNode & operator[] (const TIndex & index) const
        {
            if (index < _nodes.size())
                return _nodes[index];
            else
                throw IndexOverflow();
        }

        TNode & operator[] (const TIndex & index)
        {
            if (index < _nodes.size())
                return _nodes[index];
            else
                throw IndexOverflow();
        }

        const TIndex * const neighbors(const TIndex & index, int & num) const
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


        // io functions
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


    // IO functions
    template <typename TNode, typename TIndex>
    QDataStream & operator<< (QDataStream & s, const Graph<TNode, TIndex> & g)
    {
        return g.writeBinary(s);
    }
    
    template <typename TNode, typename TIndex>
    QDataStream & operator>> (QDataStream & s, Graph<TNode, TIndex> & g)
    {
        return g.readBinary(s);
    }    
    
} // namespace Automata

#endif // GRAPH_H
