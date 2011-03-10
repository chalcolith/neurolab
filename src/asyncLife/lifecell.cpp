#include "lifecell.h"

void LifeCell::writeBinary(QDataStream &ds, const Automata::AutomataFileVersion &) const
{
    ds << this->alive;
}

void LifeCell::readBinary(QDataStream &ds, const Automata::AutomataFileVersion &)
{
    ds >> this->alive;
}
