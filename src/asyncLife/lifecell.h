#ifndef LIFECELL_H
#define LIFECELL_H

#include "../automata/automaton.h"

#include <QtGlobal>
#include <QDataStream>

class LifeCell
{
public:
    typedef qint32 LifeIndex;
    typedef Automata::Automaton<LifeCell, LifeCell::LifeIndex> BOARD_TYPE;

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

    void update(BOARD_TYPE *, const LifeIndex &, LifeCell & next, const QVector<int> & neighbor_indices, const LifeCell *neighbors) const
    {
        int num_alive_neighbors = 0;
        for (int i = 0; i < neighbor_indices.size(); ++i)
            if (neighbors[i].alive)
                ++num_alive_neighbors;

        if (this->alive)
        {
            next.alive = num_alive_neighbors == 2 || num_alive_neighbors == 3;
        }
        else
        {
            next.alive = num_alive_neighbors == 3;
        }
    }

    virtual void writeBinary(QDataStream & ds, const Automata::AutomataFileVersion & file_version) const;
    virtual void readBinary(QDataStream & ds, const Automata::AutomataFileVersion & file_version);
};

#endif // LIFECELL_H
