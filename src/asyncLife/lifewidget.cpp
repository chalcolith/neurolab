#include "lifewidget.h"
#include "lifeboard.h"

#include <QPainter>

LifeWidget::LifeWidget(QWidget *parent) : QWidget(parent), autoStep(false), board(new LifeBoard())
{
}

LifeWidget::~LifeWidget()
{
    step_c.waitForFinished();
    step_b.waitForFinished();
    step_a.waitForFinished();

    delete board;
}

void LifeWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QRect viewport = painter.viewport();

    int x_step = viewport.width() / this->board->getWidth();
    int y_step = viewport.height() / this->board->getHeight();

    for (int y = 0; y < this->board->getHeight(); ++y)
    {
        for (int x = 0; x < this->board->getWidth(); ++x)
        {
            int index = x + (y * this->board->getWidth());

            QColor cellColor;

            switch (this->board->readyState(index))
            {
            case 2:
                cellColor = Qt::darkGray;
                break;
            case 1:
                cellColor = Qt::lightGray;
                break;
            case 0:
            default:
                cellColor = Qt::white;
            }

            //cellColor = Qt::white;

            painter.fillRect(x*x_step, y*y_step, x_step, y_step, cellColor);

            if ((*this->board)[index].current().alive)
                painter.fillRect(x*x_step + 1, y*y_step + 1, x_step - 2, y_step - 2, Qt::black);
        }
    }

    // trigger repaint
    if (this->autoStep)
    {
        step_c.waitForFinished();
        step_b.waitForFinished();
        step_a.waitForFinished();

        step_a = this->board->stepAsync();
//        step_b = this->board->stepAsync();
//        step_c = this->board->stepAsync();
        update();

    }
} // LifeWidget::paintEvent()

void LifeWidget::step()
{
    this->board->step();
    update();
}

void LifeWidget::start()
{
    this->autoStep = true;
    step_a = this->board->stepAsync();
//    step_b = this->board->stepAsync();
//    step_c = this->board->stepAsync();
    update();
}

void LifeWidget::stop()
{
    this->autoStep = false;
    update();
}

void LifeWidget::reset()
{
    this->board->reset();
}
