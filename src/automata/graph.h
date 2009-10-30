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
        
        QVector<TNode> nodes;
        QVector< QVector<TIndex> > edges;

    public:
        Graph(const int initialCapacity = 0, bool directed = false)
            : directed(directed)
        {
            nodes.reserve(initialCapacity);
            edges.reserve(initialCapacity);
        }

        virtual ~Graph()
        {
        }

        TIndex addNode(const TNode & node)
        {
            TIndex index = nodes.size();

            nodes.append(node);
            nodes[index].setIndex(index);

            edges.append(QVector<TIndex>());
            
            return index;
        }
        
        void addEdge(const TIndex & from, const TIndex & to)
        {
            if (from < edges.size())
            {
                QVector<TIndex> & outgoing = edges[from];
                
                if (!outgoing.contains(to))
                    outgoing.append(to);
            }
            else
            {
                throw IndexOverflow();
            }
            
            if (!directed)
            {
                if (to < edges.size())
                {
                    QVector<TIndex> & incoming = edges[to];
                    
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
            if (from < edges.size())
            {
                QVector<TIndex> & outgoing = edges[from];
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
                if (to < edges.size())
                {
                    QVector<TIndex> & incoming = edges[to];
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
            if (index < nodes.size())
                return nodes[index];
            else
                throw IndexOverflow();
        }

        TNode & operator[] (const TIndex & index)
        {
            if (index < nodes.size())
                return nodes[index];
            else
                throw IndexOverflow();
        }

        const TIndex * const getNeighbors(const TIndex & index, int & num) const
        {
            if (index < nodes.size())
            {
                const QVector<TIndex> & neighbors = edges[index];
                num = neighbors.size();
                return neighbors.data();
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
            ds << this->nodes;
            ds << this->edges;
            
            return ds;
        }
        
        virtual QDataStream & readBinary(QDataStream & ds)
        {
            ds.setVersion(QDataStream::Qt_4_5);

            int version;            
            ds >> version;
            
            // data
            ds >> this->directed;
            ds >> this->nodes;
            ds >> this->edges;
            
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
