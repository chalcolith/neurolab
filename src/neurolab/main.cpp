#include <QtGui/QApplication>
#include "mainwindow.h"

#include <QMessageBox>

int main(int argc, char *argv[])
{
    try
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
        return application.exec();
    }
    catch (NeuroLab::Exception & ne)
    {
        QMessageBox::critical(0, "Critical Error", ne.message());
    }
    catch (std::exception & se)
    {
        QMessageBox::critical(0, "Critical Error", se.what());
    }
    
    return -1;
}
