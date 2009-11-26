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
        
        Ui::MainWindow *ui;
        QVBoxLayout *layout;
        
        LabNetwork *currentNetwork;
        
    public:
        MainWindow(QWidget *parent = 0, const QString & initialFname = QString());
        ~MainWindow();
        
        static MainWindow *instance();
        
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
        void on_actionDelete_triggered();
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
    
    
    class Exception
    {
        QString _message;
        
    public:
        Exception(const QString & _message) : _message(_message) {}
        const QString & message() const { return _message; }
    };
    
} // namespace NeuroLab

#endif // MAINWINDOW_H
