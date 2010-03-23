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
#include <QCloseEvent>
#include <QMessageBox>
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
          _propertyEditor(0), _propertyFactory(new QtVariantEditorFactory()), _propertyManager(new QtVariantPropertyManager())
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
    }

    void MainWindow::closeEvent(QCloseEvent *event)
    {
        if (closeNetwork())
            saveStateSettings();
        else
            event->ignore();
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
        LabNetwork *newNetwork = 0;

        try
        {
            newNetwork = LabNetwork::open(this, QString());
        }
        catch (LabException & le)
        {
            QMessageBox::critical(this, tr("Unable to open network."), le.message());
            newNetwork = 0;
        }

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
        try
        {
            if (_currentNetwork && _currentNetwork->save(false))
            {
                setTitle();
                return true;
            }
        }
        catch (LabException & le)
        {
            QMessageBox::critical(this, tr("Unable to save network."), le.message());
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
                    if (!saveNetwork())
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
        }

        return true;
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
        if (po == _propertyObject)
            return;

        // remove existing properties
        QList<QtProperty *> currentProperties = _propertyEditor->properties();
        for (QListIterator<QtProperty *> i(currentProperties); i.hasNext(); i.next())
        {
            QtProperty *property = i.peekNext();
            _propertyEditor->removeProperty(property);
            delete property;
        }

        // get new properties
        _propertyObject = po;

        if (_propertyObject)
        {
            // new top item
            QtProperty *topItem = _propertyManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Properties"));

            _propertyObject->buildProperties(_propertyManager, topItem);

            _propertyEditor->addProperty(topItem);
            _propertyEditor->setRootIsDecorated(false);
        }
    }

} // namespace NeuroLab


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
    close();
}

void NeuroLab::MainWindow::on_action_Sidebar_triggered()
{
    _ui->sidebarDockWidget->toggleViewAction()->trigger();
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
        _currentNetwork->step();
}

void NeuroLab::MainWindow::on_action_Reset_triggered()
{
    if (_currentNetwork)
        _currentNetwork->reset();
}

void NeuroLab::MainWindow::on_action_Delete_triggered()
{
    if (_currentNetwork)
        _currentNetwork->deleteSelected();
}
