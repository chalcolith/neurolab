#include "lifecell.h"

void LifeCell::update(BOARD_TYPE *, const LifeIndex &, LifeCell &next, const QVector<int> &neighbor_indices, const LifeCell *const *const neighbors) const
{
    int num_alive_neighbors = 0;
    for (int i = 0; i < neighbor_indices.size(); ++i)
        if (neighbors[i]->alive)
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


void LifeCell::writeBinary(QDataStream &ds, const Automata::AutomataFileVersion &) const
{
    ds << this->alive;
}

void LifeCell::readBinary(QDataStream &ds, const Automata::AutomataFileVersion &)
{
    ds >> this->alive;
}
