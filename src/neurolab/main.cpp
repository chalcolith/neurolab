#include <QtGui/QApplication>
#include <QMessageBox>

#include "../automata/exception.h"
#include "../neurogui/mainwindow.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("Balafon");
    QCoreApplication::setOrganizationDomain("balafon.net");
    QCoreApplication::setApplicationName(QObject::tr("NeuroLab"));
    QCoreApplication::setApplicationVersion(NeuroLab::VERSION);

    QApplication application(argc, argv);
    NeuroLab::MainWindow window(0, QCoreApplication::applicationName());

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
