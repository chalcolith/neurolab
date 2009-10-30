#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include "lifewidget.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    LifeWidget *lifeWidget;

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

signals:
    void done();
    void setStatus(const QString & status);

private slots:
    void on_actionReset_triggered();
    void on_actionStep_triggered();
    void on_actionStop_triggered();
    void on_actionStart_triggered();
    void on_action_Quit_triggered();
};

#endif // MAINWINDOW_H
