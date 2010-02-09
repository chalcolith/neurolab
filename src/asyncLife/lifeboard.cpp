#include "lifeboard.h"
#include <QDateTime>

LifeBoard::LifeBoard(const int & width, const int & height)
    : BASE(width*height, true),
      width(width), height(height)
{
    // initialize with alternating cells
    qsrand(QDateTime::currentDateTime().toTime_t());

    LifeCell cell;

    for (int i = 0; i < width*height; ++i)
    {
        cell.alive = !(qrand() % 2);
        addNode(cell);
    }

    initNeighbors();
} // LifeBoard::LifeBoard()

void LifeBoard::reset()
{
    for (int i = 0; i < width*height; ++i)
    {
        this->_nodes[i].q0.alive = this->_nodes[i].q1.alive = !(qrand() % 2);
        this->_nodes[i].r = 0;
    }
}

void LifeBoard::initNeighbors()
{
    // we have set the graph to be directed, so we need to add all links
    for (LifeCell::LifeIndex row = 0; row < height; ++row)
    {
        for (LifeCell::LifeIndex col = 0; col < width; ++col)
        {
            LifeCell::LifeIndex index, rowAbove, rowBelow, colLeft, colRight;

            index = (row * width) + col;
            rowAbove = (row + (height-1)) % height;
            rowBelow = (row + 1) % height;
            colLeft = (col + (width-1)) % width;
            colRight = (col + 1) % width;

            addEdge(index, (rowAbove * width) + colLeft); // top left
            addEdge(index, (rowAbove * width) + col); // top
            addEdge(index, (rowAbove * width) + colRight); // top right
            addEdge(index, (row * width) + colLeft); // left
            addEdge(index, (row * width) + colRight); // right
            addEdge(index, (rowBelow * width) + colLeft); // bottom left
            addEdge(index, (rowBelow * width) + col); // bottom
            addEdge(index, (rowBelow * width) + colRight); // bottom right
        }
    }
} // LifeBoard::initNeighbors()
