#ifndef LIFECELL_H
#define LIFECELL_H

#include "../automata/automaton.h"

#include <QtGlobal>
#include <QDataStream>

class LifeCell
{
public:
    enum
    {
        NUM_PER_LOCK = 32
    };

    typedef qint32 LifeIndex;
    typedef Automata::Automaton<LifeCell, LifeCell::LifeIndex, NUM_PER_LOCK> BOARD_TYPE;

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

    void update(BOARD_TYPE *, const LifeIndex &, LifeCell & next, const QVector<int> & neighbor_indices, const LifeCell * const * const neighbors) const;

    virtual void writeBinary(QDataStream & ds, const Automata::AutomataFileVersion & file_version) const;
    virtual void readBinary(QDataStream & ds, const Automata::AutomataFileVersion & file_version);
};

#endif // LIFECELL_H
