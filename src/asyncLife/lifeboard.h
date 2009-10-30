#ifndef LIFEBOARD_H
#define LIFEBOARD_H

#include "../automata/automaton.h"
#include "lifecell.h"

class LifeBoard : public Automata::Automaton<LifeCell, LifeCell::Update, LifeCell::LifeIndex>
{
    const int width, height;

public:
    LifeBoard(const int & width = 100, const int & height = 100);

    const int & getWidth() const { return width; }
    const int & getHeight() const { return height; }

    void reset();

private:
    void initNeighbors();
};

#endif // LIFEBOARD_H
