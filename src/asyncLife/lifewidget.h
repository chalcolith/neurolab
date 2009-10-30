#ifndef LIFEWIDGET_H
#define LIFEWIDGET_H

#include <QWidget>
#include "lifeboard.h"

class LifeWidget : public QWidget
{
    bool autoStep;
    LifeBoard board;

public:
    LifeWidget(QWidget *parent = 0);

protected:
    virtual void paintEvent(QPaintEvent *);

public slots:
    void step();
    void start();
    void stop();
    void reset();
};

#endif // LIFEWIDGET_H
