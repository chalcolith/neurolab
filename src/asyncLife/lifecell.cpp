#include "lifecell.h"

QDataStream & operator<< (QDataStream & ds, const LifeCell & lc)
{
    return ds << lc.alive;
}

QDataStream & operator>> (QDataStream & ds, LifeCell & lc)
{
    return ds >> lc.alive;
}
