#ifndef LIFECELL_H
#define LIFECELL_H

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
        void operator() (const LifeCell & prev, LifeCell & next, const int & numNeighbors, const LifeCell * const * const neighbors) const
        {
            int num_alive_neighbors = 0;
            for (int i = 0; i < numNeighbors; ++i)
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
