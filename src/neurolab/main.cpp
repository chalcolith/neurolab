#include <QtGui/QApplication>
#include <QMessageBox>

#include "mainwindow.h"
#include "../automata/exception.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("Balafon");
    QCoreApplication::setOrganizationDomain("balafon.net");
    QCoreApplication::setApplicationName("NeuroLab");
    QCoreApplication::setApplicationVersion(NeuroLab::NEUROLAB_VERSION);
    
    QApplication application(argc, argv);
    NeuroLab::MainWindow window;

    QObject::connect(&application, SIGNAL(lastWindowClosed()), &window, SLOT(on_action_Quit_triggered()));
    QObject::connect(&window, SIGNAL(quitting()), &application, SLOT(quit()));
    
    window.show();
    
    try
    {        
        return application.exec();
    }
    catch (Automata::Exception & ne)
    {
        QMessageBox::critical(0, "Critical Error", ne.message());
    }
    catch (std::exception & se)
    {
        QMessageBox::critical(0, "Critical Error", se.what());
    }
    
    return -1;
}
