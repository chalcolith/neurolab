#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "labnetwork.h"

#include <QtGui/QMainWindow>
#include <QVBoxLayout>

namespace Ui
{
    class MainWindow;
}

namespace NeuroLab
{    
    
    extern const QString NEUROLAB_VERSION;
    
    class MainWindow
        : public QMainWindow
    {
        Q_OBJECT
        
        static MainWindow *_instance;
        
        Ui::MainWindow *_ui;
        QVBoxLayout *layout;
        
        LabNetwork *currentNetwork;
        
    public:
        MainWindow(QWidget *parent = 0, const QString & initialFname = QString());
        ~MainWindow();
        
        static MainWindow *instance();
        Ui::MainWindow *ui() { return _ui; }
        
    signals:
        void quitting();        
        
    private:
        void loadStateSettings();
        void saveStateSettings();
        void setupConnections();
        
        bool newNetwork();
        bool openNetwork();
        bool saveNetwork();
        bool closeNetwork();
        
        void setNetwork(LabNetwork *network);
                
    private slots:
        void on_actionStep_triggered();
        void on_actionStop_triggered();
        void on_actionStart_triggered();
        void on_actionDeactivate_triggered();
        void on_actionActivate_triggered();
        void on_actionLabel_triggered();
        void on_action_Sidebar_triggered();
        void on_action_New_triggered();
        void on_action_Open_triggered();
        void on_action_Close_triggered();
        void on_action_Save_triggered();
        void on_action_Quit_triggered();
        void on_action_Delete_triggered();

        void on_actionNew_Node_triggered();
        void on_actionNew_Excitory_Link_triggered();
        void on_actionNew_Inhibitory_Link_triggered();

        void enableItemMenu();
    };
        
    class LabException
        : public Automata::Exception
    {
    public:
        LabException(const QString & message) : Automata::Exception(message) {}
    };
    
} // namespace NeuroLab

#endif // MAINWINDOW_H
