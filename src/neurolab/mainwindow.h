#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "labnetwork.h"
#include "propertyobj.h"

#include <QtGui/QMainWindow>
#include <QVBoxLayout>

class QtTreePropertyBrowser;
class QtVariantEditorFactory;
class QtVariantPropertyManager;

namespace Ui
{
    class MainWindow;
}

namespace NeuroLab
{    
    
    extern const QString VERSION;
    
    class MainWindow
        : public QMainWindow
    {
        Q_OBJECT
        
        static MainWindow *_instance;
        
        Ui::MainWindow *_ui;
        QVBoxLayout *_layout;
        
        LabNetwork *_currentNetwork;
        
        QtTreePropertyBrowser *_propertyEditor;
        
        QtVariantEditorFactory *_propertyFactory;
        QtVariantPropertyManager *_propertyManager;
        
        PropertyObject *_propertyObject;
                
    public:
        MainWindow(QWidget *parent = 0, const QString & initialFname = QString());
        ~MainWindow();
        
        static MainWindow *instance();
        Ui::MainWindow *ui() { return _ui; }
        
        QtTreePropertyBrowser *propertyEditor() { return _propertyEditor; }
        PropertyObject *propertyObject() { return _propertyObject; }
        
    protected:
        virtual void closeEvent(QCloseEvent *);

    public slots:
        void setTitle(const QString & title = QString("NeuroLab"));
        void setPropertyObject(PropertyObject *);
        
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
        void on_action_Reset_triggered();
        void on_action_Step_triggered();
        void on_action_Stop_triggered();
        void on_action_Start_triggered();
        void on_action_Activate_triggered();
        void on_action_ToggleFrozen_triggered();
        void on_action_Sidebar_triggered();
        void on_action_New_triggered();
        void on_action_Open_triggered();
        void on_action_Close_triggered();
        void on_action_Save_triggered();
        void on_action_Quit_triggered();
        void on_action_Delete_triggered();
        void on_action_New_Node_triggered();
        void on_action_New_Excitory_Link_triggered();
        void on_action_New_Inhibitory_Link_triggered();

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
