#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    lifeWidget = new LifeWidget();
    lifeWidget->setObjectName("lifeWidget");

    QVBoxLayout *hLayout = new QVBoxLayout(ui->centralWidget);
    hLayout->addWidget(lifeWidget);

    lifeWidget->show();

    connect(this, SIGNAL(setStatus(QString)), ui->statusBar, SLOT(showMessage(QString)));
    setStatus("The Game of Life");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_Quit_triggered()
{
    done();
}

void MainWindow::on_actionStart_triggered()
{
    lifeWidget->start();
}

void MainWindow::on_actionStop_triggered()
{
    lifeWidget->stop();
}

void MainWindow::on_actionStep_triggered()
{
    lifeWidget->step();
}

void MainWindow::on_actionReset_triggered()
{
    lifeWidget->reset();
}
