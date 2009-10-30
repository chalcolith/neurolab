#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    
    MainWindow window;
    window.show();
    
    QObject::connect(&window, SIGNAL(done()), &application, SLOT(quit()));
    
    return application.exec();
}
