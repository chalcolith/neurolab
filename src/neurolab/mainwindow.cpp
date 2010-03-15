#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "labscene.h"
#include "neuroitem.h"
#include "labeldialog.h"
#include "../automata/exception.h"

#include "filedirtydialog.h"
#include "neurolinkitem.h"
#include "neuronodeitem.h"

#include <QSettings>
#include <QtTreePropertyBrowser>
#include <QtVariantEditorFactory>
#include <QtVariantPropertyManager>

namespace NeuroLab
{
        
    extern const QString 
#include "../neurolab_version.txt"
    ;
        
    MainWindow *MainWindow::_instance = 0;
    
    MainWindow::MainWindow(QWidget *parent, const QString & initialFname)
        : QMainWindow(parent), 
          _ui(new Ui::MainWindow()),
          _layout(0), _currentNetwork(0),
          _propertyEditor(0), _propertyFactory(new QtVariantEditorFactory()), _propertyManager(new QtVariantPropertyManager()),
          _lastWindowClosed(false)
    {
        if (_instance)
            throw LabException("You cannot create more than one main window.");

        _instance = this;
        
        // set up ui and other connections
        _ui->setupUi(this);
        
        QVBoxLayout *sidebarLayout = new QVBoxLayout();
        _ui->sidebar_page_1->setLayout(sidebarLayout);
        sidebarLayout->addWidget(_propertyEditor = new QtTreePropertyBrowser());
        _propertyEditor->setFactoryForManager(_propertyManager, _propertyFactory);
        
        setupConnections();
        
        // central widget layout
        _layout = new QVBoxLayout();
        _ui->centralWidget->setLayout(_layout);
        
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
        setPropertyObject(0);
        setNetwork(0);

        delete _propertyManager;
        delete _propertyFactory;
        
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
        if (_currentNetwork && !closeNetwork())
            return false;
                
        setNetwork(new LabNetwork());
        return true;
    }
    
    bool MainWindow::openNetwork()
    {
        LabNetwork *newNetwork = LabNetwork::open(this, QString());
        
        if (newNetwork)
        {
            if (_currentNetwork && !closeNetwork())
            {
                setNetwork(0);
                return false;
            }
            
            setNetwork(newNetwork);
            return true;
        }

        return false;
    }
    
    bool MainWindow::saveNetwork()
    {
        if (_currentNetwork && _currentNetwork->save(false))
        {
            setTitle();
            return true;
        }
        
        return false;
    }
    
    /// \return True if we closed the network successfully; false otherwise.
    bool MainWindow::closeNetwork()
    {
        if (_currentNetwork)
        {
            bool discard = false;
            if (_currentNetwork->dirty())
            {
                FileDirtyDialog fdd(this);
                fdd.exec();
                
                switch (fdd.response())
                {
                case FileDirtyDialog::SAVE:
                    if (!_currentNetwork->save(false))
                        return false;
                    break;
                case FileDirtyDialog::DISCARD:
                    discard = true;
                    delete _currentNetwork;
                    _currentNetwork = 0;
                    break;
                case FileDirtyDialog::CANCEL:
                    return false;
                }
            }
                        
            setNetwork(0);
            return true;
        }
        
        return false;
    }
    
    void MainWindow::setTitle(const QString & title)
    {
        if (_currentNetwork && !_currentNetwork->fname().isEmpty())
        {
            QString fname = _currentNetwork->fname();
            int index = fname.lastIndexOf('/');
            if (index == -1)
                index = fname.lastIndexOf('\\');
            if (index != -1)
                fname = fname.mid(index+1);
            
            this->setWindowTitle(QString("%1: %2%3").arg(title).arg(fname).arg(_currentNetwork->dirty() ? "*" : ""));            
        }
        else
        {
            this->setWindowTitle(QString("%1%2").arg(title).arg(_currentNetwork && _currentNetwork->dirty() ? "*" : ""));
        }
    }
    
    void MainWindow::setNetwork(LabNetwork *network)
    {
        if (_currentNetwork)
            _layout->removeWidget(_currentNetwork->view());
        delete _currentNetwork;
        
        if (network)
        {
            _currentNetwork = network;
            _layout->addWidget(_currentNetwork->view());
            _currentNetwork->view()->show();
        }
        else
        {
            _currentNetwork = 0;
            _ui->centralWidget = 0;
        }

        setTitle();
        setPropertyObject(_currentNetwork);
        update();
    }
    
    void MainWindow::setPropertyObject(PropertyObject *po)
    {
        // remove existing properties
        QList<QtProperty *> currentProperties = _propertyEditor->properties();
        for (QListIterator<QtProperty *> i(currentProperties); i.hasNext(); i.next())
        {
            QtProperty *property = i.peekNext();
            _propertyEditor->removeProperty(property);
            delete property;
        }
                
        // get new properties
        _currentPropertyObject = po;
        
        if (_currentPropertyObject)
        {
            // new top item
            QtProperty *topItem = _propertyManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Properties"));
            
            _currentPropertyObject->buildProperties(_propertyManager, topItem);
            
            _propertyEditor->addProperty(topItem);
            _propertyEditor->setRootIsDecorated(false);
        }
    }

    void MainWindow::enableItemMenu()
    {
        NeuroItem *item = (_currentNetwork && _currentNetwork->scene()) ? _currentNetwork->scene()->itemUnderMouse() : 0;
        
        bool onItem = item != 0;
        bool onNode = dynamic_cast<NeuroNodeItem *>(item) != 0;
        bool onLink = dynamic_cast<NeuroLinkItem *>(item) != 0;

        _ui->action_New_Node->setEnabled(!onNode);
        _ui->action_New_Excitory_Link->setEnabled(!onLink);
        _ui->action_New_Inhibitory_Link->setEnabled(!onLink);
        
        _ui->action_Delete->setEnabled(onItem);
        
        _ui->action_Activate->setEnabled(onNode);
        _ui->action_ToggleFrozen->setEnabled(onNode);
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

void NeuroLab::MainWindow::lastWindowClosed()
{
    if (!_lastWindowClosed)
    {
        _lastWindowClosed = true;
        on_action_Quit_triggered();
    }
}

void NeuroLab::MainWindow::on_action_Quit_triggered()
{
    if (closeNetwork())
    {
        quitting();
        saveStateSettings();
    }
    else
    {
        this->show();
    }
}

void NeuroLab::MainWindow::on_action_Sidebar_triggered()
{
    _ui->sidebarDockWidget->toggleViewAction()->trigger();
}

void NeuroLab::MainWindow::on_action_New_Node_triggered()
{
    if (_currentNetwork)
    {
        _currentNetwork->newItem(typeid(NeuroLab::NeuroNodeItem).name());
        update();
    }
}

void NeuroLab::MainWindow::on_action_New_Excitory_Link_triggered()
{
    if (_currentNetwork)
    {
        _currentNetwork->newItem(typeid(NeuroLab::NeuroExcitoryLinkItem).name());
        update();
    }
}

void NeuroLab::MainWindow::on_action_New_Inhibitory_Link_triggered()
{
    if (_currentNetwork)
    {
        _currentNetwork->newItem(typeid(NeuroLab::NeuroInhibitoryLinkItem).name());
        update();
    }
}

void NeuroLab::MainWindow::on_action_Delete_triggered()
{
    if (_currentNetwork && _currentNetwork->scene())
    {
        for (QListIterator<QGraphicsItem *> i(_currentNetwork->scene()->selectedItems()); i.hasNext(); i.next())
        {
            QGraphicsItem *item = i.peekNext();
            _currentNetwork->scene()->removeItem(item);
            delete item;
        }
    }
}

void NeuroLab::MainWindow::on_action_Activate_triggered()
{
    if (_currentNetwork && _currentNetwork->scene())
    {
        for (QListIterator<QGraphicsItem *> i(_currentNetwork->scene()->selectedItems()); i.hasNext(); i.next())
        {
            NeuroItem *item = dynamic_cast<NeuroItem *>(i.peekNext());
            if (item)
                item->toggleActivated();
        }

        update();
    }
}

void NeuroLab::MainWindow::on_action_ToggleFrozen_triggered()
{
    if (_currentNetwork && _currentNetwork->scene())
    {
        for (QListIterator<QGraphicsItem *> i(_currentNetwork->scene()->selectedItems()); i.hasNext(); i.next())
        {
            NeuroItem *item = dynamic_cast<NeuroItem *>(i.peekNext());
            if (item)
                item->toggleFrozen();
        }

        update();
    }
}

void NeuroLab::MainWindow::on_action_Start_triggered()
{
}

void NeuroLab::MainWindow::on_action_Stop_triggered()
{
}

void NeuroLab::MainWindow::on_action_Step_triggered()
{
    if (_currentNetwork)
    {
        _currentNetwork->step();        
        update();
    }
}

void NeuroLab::MainWindow::on_action_Reset_triggered()
{
    if (_currentNetwork)
    {
        _currentNetwork->reset();
        update();
    }
}
