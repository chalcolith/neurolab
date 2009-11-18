#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "labscene.h"

#include <QSettings>

namespace NeuroLab
{
    
    MainWindow *MainWindow::_instance = 0;
    
    MainWindow::MainWindow(QWidget *parent, const QString & initialFname)
        : QMainWindow(parent), 
          ui(new Ui::MainWindow),
          layout(0),
          currentNetwork(0)
    {
        if (_instance)
            throw new Exception("You cannot create more than one main window.");

        _instance = this;
        
        // set up ui and other connections
        ui->setupUi(this);
        
        setupConnections();
        
        // central widget layout
        layout = new QVBoxLayout();
        ui->centralWidget->setLayout(layout);
        
        this->setWindowTitle(tr("NeuroLab"));
        
        // read state from settings
        loadStateSettings();
        
        // load initial network
        if (initialFname.isNull() || initialFname.isEmpty())
            setNetwork(new LabNetwork());
        else
            setNetwork(LabNetwork::open(this, initialFname));
    }
    
    MainWindow::~MainWindow()
    {
        setNetwork(0);
        delete ui;
    }
    
    MainWindow *MainWindow::instance()
    {
        return _instance;
    }

    void MainWindow::loadStateSettings()
    {
        QSettings settings;
        settings.beginGroup("MainWindow");
        resize(settings.value("size", QSize(800, 600)).toSize());
        move(settings.value("pos", QPoint(10, 10)).toPoint());
        restoreState(settings.value("status", QByteArray()).toByteArray(), NEUROLAB_APP_VERSION);
        settings.endGroup();
    }
    
    void MainWindow::saveStateSettings()
    {    
        QSettings settings;
        settings.beginGroup("MainWindow");
        settings.setValue("size", size());
        settings.setValue("pos", pos());
        settings.setValue("status", saveState(NEUROLAB_APP_VERSION));
        settings.endGroup();
    }
    
    void MainWindow::setupConnections()
    {
        connect(ui->sidebarDockWidget, SIGNAL(visibilityChanged(bool)), ui->action_Sidebar, SLOT(setChecked(bool)));
    }
    
    bool MainWindow::newNetwork()
    {
        if (!closeNetwork())
            return false;
                
        setNetwork(new LabNetwork());        
        return true;
    }
    
    bool MainWindow::openNetwork()
    {
        if (!closeNetwork())
            return false;
                
        setNetwork(LabNetwork::open(this, QString()));
        return true;
    }
    
    /// \return True if we closed the network successfully; false otherwise.
    bool MainWindow::closeNetwork()
    {
        if (currentNetwork)
        {
            if (!currentNetwork->close())
                return false;
            
            setNetwork(0);
        }
        
        return true;
    }
    
    void MainWindow::setNetwork(LabNetwork *network)
    {
        if (currentNetwork)
            layout->removeWidget(currentNetwork->getView());
        delete currentNetwork;
        
        ui->nodeButton->setChecked(true);
        
        if (network)
        {
            currentNetwork = network;
            layout->addWidget(currentNetwork->getView());
            currentNetwork->getView()->show();
            currentNetwork->setMode(LabScene::MODE_ADD_NODE);
            
            this->setWindowTitle(tr("NeuroLab: %1").arg(currentNetwork->getFName()));
        }
        else
        {
            currentNetwork = 0;
            ui->centralWidget = 0;
            this->setWindowTitle(tr("NeuroLab"));
        }

        this->update();
    }
    
} // namespace NeuroLab

//////////////////////////////////////////////////////////////////////
// action slots

void NeuroLab::MainWindow::on_action_New_triggered()
{
    newNetwork();
}

void NeuroLab::MainWindow::on_action_Open_triggered()
{
    openNetwork();
}

void NeuroLab::MainWindow::on_action_Close_triggered()
{
    closeNetwork();
}

void NeuroLab::MainWindow::on_action_Save_triggered()
{
    /// \todo Handle save as.
}

void NeuroLab::MainWindow::on_action_Quit_triggered()
{
    if (closeNetwork())
        quitting();
    saveStateSettings();
}

void NeuroLab::MainWindow::on_action_Sidebar_triggered()
{
    ui->sidebarDockWidget->toggleViewAction()->trigger();
}

void NeuroLab::MainWindow::on_nodeButton_toggled(bool checked)
{
    if (checked && currentNetwork)
        currentNetwork->setMode(NeuroLab::LabScene::MODE_ADD_NODE);
}

void NeuroLab::MainWindow::on_eLinkButton_toggled(bool checked)
{
    if (checked && currentNetwork)
        currentNetwork->setMode(NeuroLab::LabScene::MODE_ADD_E_LINK);    
}

void NeuroLab::MainWindow::on_iLinkButton_toggled(bool checked)
{
    if (checked && currentNetwork)
        currentNetwork->setMode(NeuroLab::LabScene::MODE_ADD_I_LINK);
}
