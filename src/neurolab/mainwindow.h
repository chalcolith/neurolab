#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "labnetwork.h"

#include <QtGui/QMainWindow>
#include <QVBoxLayout>

extern int NEUROLAB_APP_MAINVERSION;
extern int NEUROLAB_APP_SUBVERSION;

#define NEUROLAB_APP_VERSION (NEUROLAB_APP_MAINVERSION*1000 + NEUROLAB_APP_SUBVERSION)

namespace Ui
{
    class MainWindow;
}

namespace NeuroLab
{
    
    class MainWindow 
            : public QMainWindow
    {
        Q_OBJECT
        
        Ui::MainWindow *ui;
        QVBoxLayout *layout;
        
        LabNetwork *currentNetwork;
        
    public:
        MainWindow(QWidget *parent = 0, const QString & initialFname = QString());
        ~MainWindow();
        
    signals:
        void quitting();        
        
    private:
        void loadStateSettings();
        void saveStateSettings();
        void setupConnections();
        
        bool newNetwork();
        bool openNetwork();
        bool closeNetwork();
        
        void setNetwork(LabNetwork *network);
                
    private slots:
        void on_iLinkButton_toggled(bool checked);
        void on_eLinkButton_toggled(bool checked);
        void on_nodeButton_toggled(bool checked);
        void on_action_Sidebar_triggered();
        void on_action_New_triggered();
        void on_action_Open_triggered();
        void on_action_Close_triggered();
        void on_action_Save_triggered();
        void on_action_Quit_triggered();
};
    
} // namespace NeuroLab

#endif // MAINWINDOW_H
