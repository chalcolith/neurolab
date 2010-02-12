#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "labscene.h"
#include "neuroitem.h"
#include "labeldialog.h"
#include "../automata/exception.h"

#include "neurolinkitem.h"
#include "neuronodeitem.h"

#include <QSettings>
#include <QtTreePropertyBrowser>

namespace NeuroLab
{
        
    extern const QString 
#include "../neurolab_version.txt"
    ;
        
    MainWindow *MainWindow::_instance = 0;
    
    MainWindow::MainWindow(QWidget *parent, const QString & initialFname)
        : QMainWindow(parent), 
          _ui(new Ui::MainWindow()),
          layout(0), _properties(0),
          currentNetwork(0)
    {
        if (_instance)
            throw LabException("You cannot create more than one main window.");

        _instance = this;
        
        // set up ui and other connections
        _ui->setupUi(this);
        
        QVBoxLayout *sidebarLayout = new QVBoxLayout();
        _ui->sidebar_page_1->setLayout(sidebarLayout);
        sidebarLayout->addWidget(_properties = new QtTreePropertyBrowser());
        
        setupConnections();
        
        // central widget layout
        layout = new QVBoxLayout();
        _ui->centralWidget->setLayout(layout);
        
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
        delete _ui;
        _instance = 0;
    }
    
    MainWindow *MainWindow::instance()
    {
        return _instance;
    }

    static int NEUROLAB_APP_VERSION()
    {
        QStringList nums = VERSION.split(".");
        int result = 0;
        for (int i = 0; i < nums.length(); ++i)
        {
            result = (result * 1000) + nums[i].toInt();
        }
        return result;
    }
    
    void MainWindow::loadStateSettings()
    {
        QSettings settings;
        settings.beginGroup("MainWindow");
        resize(settings.value("size", QSize(800, 600)).toSize());
        move(settings.value("pos", QPoint(10, 10)).toPoint());
        restoreState(settings.value("status", QByteArray()).toByteArray(), NEUROLAB_APP_VERSION());
        settings.endGroup();
    }
    
    void MainWindow::saveStateSettings()
    {    
        QSettings settings;
        settings.beginGroup("MainWindow");
        settings.setValue("size", size());
        settings.setValue("pos", pos());
        settings.setValue("status", saveState(NEUROLAB_APP_VERSION()));
        settings.endGroup();
    }
    
    void MainWindow::setupConnections()
    {
        connect(_ui->sidebarDockWidget, SIGNAL(visibilityChanged(bool)), _ui->action_Sidebar, SLOT(setChecked(bool)));
        connect(_ui->menuItem, SIGNAL(aboutToShow()), this, SLOT(enableItemMenu()));
    }
    
    bool MainWindow::newNetwork()
    {
        if (currentNetwork && !closeNetwork())
            return false;
                
        setNetwork(new LabNetwork());
        return true;
    }
    
    bool MainWindow::openNetwork()
    {
        LabNetwork *newNetwork = LabNetwork::open(this, QString());
        
        if (newNetwork)
        {
            closeNetwork();
            setNetwork(newNetwork);
            return true;
        }

        setTitle();        
        return false;
    }
    
    bool MainWindow::saveNetwork()
    {
        if (currentNetwork && currentNetwork->save(false))
        {
            setTitle();
            return true;
        }
        
        setTitle();
        return false;
    }
    
    /// \return True if we closed the network successfully; false otherwise.
    bool MainWindow::closeNetwork()
    {
        if (currentNetwork)
        {
            if (!currentNetwork->close())
                return false;
            
            setNetwork(0);
            return true;
        }
        
        return false;
    }
    
    void MainWindow::setTitle(const QString & title)
    {
        if (currentNetwork && !currentNetwork->fname().isEmpty())
        {
            QString fname = currentNetwork->fname();
            int index = fname.lastIndexOf('/');
            if (index == -1)
                index = fname.lastIndexOf('\\');
            if (index != -1)
                fname = fname.mid(index+1);
            
            this->setWindowTitle(QString("%1%2%3").arg(title).arg(fname).arg(currentNetwork->dirty() ? "*" : ""));            
        }
        else
        {
            this->setWindowTitle(QString("%1%2").arg(title).arg(currentNetwork && currentNetwork->dirty() ? "*" : ""));
        }
    }
    
    void MainWindow::setNetwork(LabNetwork *network)
    {
        if (currentNetwork)
            layout->removeWidget(currentNetwork->view());
        delete currentNetwork;
        
        if (network)
        {
            currentNetwork = network;
            layout->addWidget(currentNetwork->view());
            currentNetwork->view()->show();
        }
        else
        {
            currentNetwork = 0;
            _ui->centralWidget = 0;
        }

        setTitle();
        this->update();
    }

    void MainWindow::enableItemMenu()
    {
        NeuroItem *item = (currentNetwork && currentNetwork->scene()) ? currentNetwork->scene()->selectedItem() : 0;
        
        bool onItem = item != 0;
        bool onNode = dynamic_cast<NeuroNodeItem *>(item) != 0;
        bool onLink = dynamic_cast<NeuroLinkItem *>(item) != 0;

        _ui->actionNew_Node->setEnabled(!onNode);
        _ui->actionNew_Excitory_Link->setEnabled(!onLink);
        _ui->actionNew_Inhibitory_Link->setEnabled(!onLink);
        
        _ui->action_Delete->setEnabled(onItem);
        _ui->actionLabel->setEnabled(onItem);
        
        _ui->actionActivate->setEnabled(onNode);
        _ui->actionDeactivate->setEnabled(onNode);
        _ui->actionToggleFrozen->setEnabled(onNode);
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
    saveNetwork();
}

void NeuroLab::MainWindow::on_action_Quit_triggered()
{
    if (closeNetwork())
        quitting();
    saveStateSettings();
}

void NeuroLab::MainWindow::on_action_Sidebar_triggered()
{
    _ui->sidebarDockWidget->toggleViewAction()->trigger();
}

void NeuroLab::MainWindow::on_actionNew_Node_triggered()
{
    if (currentNetwork)
        currentNetwork->newItem(typeid(NeuroLab::NeuroNodeItem).name());
    setTitle();
}

void NeuroLab::MainWindow::on_actionNew_Excitory_Link_triggered()
{
    if (currentNetwork)
        currentNetwork->newItem(typeid(NeuroLab::NeuroExcitoryLinkItem).name());
    setTitle();
}

void NeuroLab::MainWindow::on_actionNew_Inhibitory_Link_triggered()
{
    if (currentNetwork)
        currentNetwork->newItem(typeid(NeuroLab::NeuroInhibitoryLinkItem).name());
    setTitle();
}

void NeuroLab::MainWindow::on_action_Delete_triggered()
{
    if (currentNetwork)
        currentNetwork->deleteSelectedItem();    
    setTitle();
}

void NeuroLab::MainWindow::on_actionLabel_triggered()
{
    if (currentNetwork)
    {
        LabelDialog ld(this);

        NeuroItem *item = currentNetwork->getSelectedItem();
        if (item)
            ld.setLabel(item->label());
        
        if (ld.exec() == QDialog::Accepted && !ld.label().isNull() && !ld.label().isEmpty())
        {
            currentNetwork->labelSelectedItem(ld.label());
        }
    }
    
    setTitle();
}

void NeuroLab::MainWindow::on_actionActivate_triggered()
{
    if (currentNetwork)
    {
        NeuroItem *item = currentNetwork->getSelectedItem();
        if (item)
            item->activate();
    }
    
    setTitle();
}

void NeuroLab::MainWindow::on_actionDeactivate_triggered()
{
    if (currentNetwork)
    {
        NeuroItem *item = currentNetwork->getSelectedItem();
        if (item)
            item->deactivate();
    }
    
    setTitle();
}

void NeuroLab::MainWindow::on_actionToggleFrozen_triggered()
{
    if (currentNetwork)
    {
        NeuroItem *item = currentNetwork->getSelectedItem();
        if (item)
            item->toggleFrozen();
    }
    
    setTitle();
}

void NeuroLab::MainWindow::on_actionStart_triggered()
{
    setTitle();
}

void NeuroLab::MainWindow::on_actionStop_triggered()
{
    setTitle();
}

void NeuroLab::MainWindow::on_actionStep_triggered()
{
    if (currentNetwork)
    {
        currentNetwork->step();
    }
    
    setTitle();
}

void NeuroLab::MainWindow::on_actionReset_triggered()
{
    if (currentNetwork)
    {
        currentNetwork->reset();
    }
    
    setTitle();
}
