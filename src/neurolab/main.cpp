#include <QtGui/QApplication>
#include "mainwindow.h"

int NEUROLAB_APP_MAINVERSION = 0;
int NEUROLAB_APP_SUBVERSION = 1;

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("Balafon");
    QCoreApplication::setOrganizationDomain("balafon.net");
    QCoreApplication::setApplicationName("NeuroLab");
    QCoreApplication::setApplicationVersion(QString("%1.%2").arg(NEUROLAB_APP_VERSION).arg(NEUROLAB_APP_SUBVERSION));
    
    QApplication application(argc, argv);
    NeuroLab::MainWindow window;

    QObject::connect(&application, SIGNAL(lastWindowClosed()), &window, SLOT(on_action_Quit_triggered()));
    QObject::connect(&window, SIGNAL(quitting()), &application, SLOT(quit()));
    
    window.show();
    return application.exec();
}
