#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "labscene.h"
#include "neuroitem.h"
#include "labeldialog.h"
#include "../automata/exception.h"

#include "filedirtydialog.h"
#include "labview.h"
#include "labscene.h"
#include "labnetwork.h"
#include "neurolinkitem.h"
#include "neuronodeitem.h"

#include <QDir>
#include <QLibrary>
#include <QSettings>
#include <QCloseEvent>
#include <QMessageBox>
#include <QSpinBox>
#include <QProgressBar>

#include <QtTreePropertyBrowser>
#include <QtVariantEditorFactory>
#include <QtVariantPropertyManager>

namespace NeuroLab
{

    const QString
#include "../neurolab_version.txt"
    ;

    MainWindow *MainWindow::_instance = 0;

    MainWindow::MainWindow(QWidget *parent, const QString & title, const QString & initialFname)
        : QMainWindow(parent),
          _title(title),
          _ui(new Ui::MainWindow()),
          _networkLayout(0),
          _numStepsSpinBox(0),
          _numStepsSpinBoxAction(0),
          _stepProgressBar(0),
          _currentNetwork(0),
          _propertyEditor(0),
          _propertyFactory(new QtVariantEditorFactory()),
          _propertyManager(new QtVariantPropertyManager()),
          _propertyObject(0)
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

        _numStepsSpinBox = new QSpinBox();
        _numStepsSpinBox->setRange(0, 1000000);
        _numStepsSpinBox->setValue(1);
        _numStepsSpinBoxAction = _ui->mainToolBar->insertWidget(_ui->action_Step, _numStepsSpinBox);

        _stepProgressBar = new QProgressBar();
        _stepProgressBar->setVisible(false);
        _stepProgressBar->reset();
        _ui->statusBar->addPermanentWidget(_stepProgressBar);

        setupConnections();

        // central widget layout
        _networkLayout = new QVBoxLayout();
        _ui->tab_1->setLayout(_networkLayout);

        this->setWindowTitle(tr("NeuroLab"));

        // read state from settings
        loadStateSettings();

        // load plugins
        loadPlugins();

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

    void MainWindow::loadPlugins()
    {
        QString pluginPath = QCoreApplication::applicationDirPath() + "/plugins";
        loadPlugins(pluginPath);
    }

    void MainWindow::loadPlugins(const QString & dirPath)
    {
        QDir dir(dirPath);
        if (!dir.exists())
            return;

        QStringList entries = dir.entryList(QDir::Files);
        for (QStringListIterator i(entries); i.hasNext(); i.next())
        {
            QString fname = dirPath + "/" + i.peekNext();
            if (QLibrary::isLibrary(fname))
            {
                QLibrary lib(fname);
                if (lib.load()) // the libraries should remain loaded even though the lib object goes out of scope
                    setStatus(tr("Loaded %1.").arg(i.peekNext()));
            }
        }
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

    void MainWindow::setActionsEnabled(bool enabled)
    {
        QList<QAction *> actions = _ui->mainToolBar->actions();
        for (QListIterator<QAction *> i(actions); i.hasNext(); )
            i.next()->setEnabled(enabled);
        _ui->action_Quit->setEnabled(enabled);
    }

    void MainWindow::closeEvent(QCloseEvent *event)
    {
        if (_currentNetwork && _currentNetwork->running())
        {
            event->ignore();
        }
        else
        {
            if (closeNetwork())
                saveStateSettings();
            else
                event->ignore();
        }
    }

    bool MainWindow::newNetwork()
    {
        setStatus("");

        if (_currentNetwork && !closeNetwork())
            return false;

        setNetwork(new LabNetwork());
        setStatus(tr("Created new network."));
        return true;
    }

    bool MainWindow::openNetwork()
    {
        setStatus("");
        LabNetwork *newNetwork = 0;

        if (_currentNetwork && !closeNetwork())
            return false;

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
            setNetwork(newNetwork);
            setStatus(tr("Opened %1").arg(newNetwork->fname()));
            return true;
        }

        return false;
    }

    bool MainWindow::saveNetwork()
    {
        setStatus("");

        try
        {
            if (_currentNetwork && _currentNetwork->save(false))
            {
                setStatus(tr("Saved %1").arg(_currentNetwork->fname()));
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
            if (_currentNetwork->changed())
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
        QString t = !(title.isEmpty() || title.isNull()) ? title : _title;

        if (_currentNetwork && !_currentNetwork->fname().isEmpty())
        {
            QString fname = _currentNetwork->fname();
            int index = fname.lastIndexOf('/');
            if (index == -1)
                index = fname.lastIndexOf('\\');
            if (index != -1)
                fname = fname.mid(index+1);

            this->setWindowTitle(QString("%1: %2%3").arg(t).arg(fname).arg(_currentNetwork->changed() ? "*" : ""));
        }
        else
        {
            this->setWindowTitle(QString("%1%2").arg(t).arg(_currentNetwork && _currentNetwork->changed() ? "*" : ""));
        }
    }

    void MainWindow::setStatus(const QString & status)
    {
        _ui->statusBar->showMessage(status);
    }

    void MainWindow::setProgressRange(int minimum, int maximum)
    {
        _stepProgressBar->setVisible(true);
        _stepProgressBar->setRange(minimum, maximum);
    }

    void MainWindow::setProgressValue(int value)
    {
        _stepProgressBar->setValue(value);
    }

    void MainWindow::setNetwork(LabNetwork *network)
    {
        if (_currentNetwork)
            _networkLayout->removeWidget(_currentNetwork->view());
        delete _currentNetwork;
        _currentNetwork = 0;

        if (network)
        {
            _currentNetwork = network;
            _networkLayout->addWidget(_currentNetwork->view());
            _currentNetwork->view()->show();

            connect(network, SIGNAL(titleChanged(QString)), this, SLOT(setTitle(QString)));
            connect(network, SIGNAL(statusChanged(QString)), this, SLOT(setStatus(QString)));
            connect(network, SIGNAL(propertyObjectChanged(PropertyObject*)), this, SLOT(setPropertyObject(PropertyObject*)));
            connect(network, SIGNAL(actionsEnabled(bool)), this, SLOT(setActionsEnabled(bool)));
            connect(network, SIGNAL(stepProgressRangeChanged(int,int)), this, SLOT(setProgressRange(int, int)));
            connect(network, SIGNAL(stepProgressValueChanged(int)), this, SLOT(setProgressValue(int)));
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
    if (!openNetwork() && !_currentNetwork)
        newNetwork();
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
        _currentNetwork->step(_numStepsSpinBox->value());
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

void NeuroLab::MainWindow::on_action_New_Data_Set_triggered()
{

}

void NeuroLab::MainWindow::on_action_Save_Data_Set_triggered()
{

}
