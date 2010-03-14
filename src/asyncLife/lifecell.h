#ifndef LIFECELL_H
#define LIFECELL_H

#include "../automata/automaton.h"

#include <QtGlobal>
#include <QDataStream>

class LifeCell
{
public:
    typedef qint32 LifeIndex;
    
    bool alive;

    LifeCell(bool alive = false) : alive(alive)
    {
    }
    
    LifeCell(const LifeCell & cell) : alive(cell.alive)
    {
    }
    
    LifeCell & operator= (const LifeCell & cell)
    {
        alive = cell.alive;
        return *this;
    }
        
    struct Update
    {
        void operator() (Automata::Automaton<LifeCell, LifeCell::Update, LifeCell::LifeIndex> *, 
                         const LifeIndex &, const LifeCell & prev, LifeCell & next, 
                         const QVector<int> & neighbor_indices, const LifeCell * const * const neighbors) const
        {
            int num_alive_neighbors = 0;
            for (int i = 0; i < neighbor_indices.size(); ++i)
                if (neighbors[i]->alive)
                    ++num_alive_neighbors;
            
            if (prev.alive)
            {
                next.alive = num_alive_neighbors == 2 || num_alive_neighbors == 3;
            }
            else
            {
                next.alive = num_alive_neighbors == 3;
            }
        }
    };
};

QDataStream & operator<< (QDataStream & ds, const LifeCell & lc);
QDataStream & operator>> (QDataStream & ds, LifeCell & lc);

#endif // LIFECELL_H
