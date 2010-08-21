#ifndef LIFEWIDGET_H
#define LIFEWIDGET_H

#include <QWidget>

class LifeBoard;

class LifeWidget : public QWidget
{
    bool autoStep;
    LifeBoard *board;

public:
    LifeWidget(QWidget *parent = 0);
    virtual ~LifeWidget();

protected:
    virtual void paintEvent(QPaintEvent *);

public slots:
    void step();
    void start();
    void stop();
    void reset();
};

#endif // LIFEWIDGET_H
