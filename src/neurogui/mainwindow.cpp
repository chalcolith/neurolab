/*
Neurocognitive Linguistics Lab
Copyright (c) 2010, Gordon Tisher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

 - Neither the name of the Neurocognitive Linguistics Lab nor the
   names of its contributors may be used to endorse or promote
   products derived from this software without specific prior
   written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "labscene.h"
#include "neuroitem.h"
#include "labeldialog.h"
#include "../automata/exception.h"

#include "filedirtydialog.h"
#include "aboutdialog.h"
#include "labview.h"
#include "labscene.h"
#include "labnetwork.h"
#include "propertyobj.h"
#include "labdatafile.h"

#include <QDir>
#include <QLibrary>
#include <QSettings>
#include <QCloseEvent>
#include <QMessageBox>
#include <QSpinBox>
#include <QProgressBar>
#include <QDesktopServices>
#include <QUrl>

#include <QtTreePropertyBrowser>
#include <QtVariantEditorFactory>
#include <QtVariantPropertyManager>

namespace NeuroLab
{

    const QString
#include "../version.txt"
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
          _currentDataFile(0),
          _propertyEditor(0),
          _propertyFactory(new QtVariantEditorFactory()),
          _propertyManager(new QtVariantPropertyManager()),
          _noncePropertyObject(0),
          _rememberProperties(true)
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

        _ui->tabWidget->setTabText(1, "");

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
            newNetwork();
        else
            setNetwork(LabNetwork::open(this, initialFname));
    }

    MainWindow::~MainWindow()
    {
        setPropertyObject(0);
        setNetwork(0);

        delete _noncePropertyObject;
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
        for (QStringListIterator i(entries); i.hasNext(); )
        {
            QString entry = i.next();
            QString fname = dirPath + "/" + entry;
            if (QLibrary::isLibrary(fname))
            {
                QLibrary lib(fname);
                if (lib.load()) // the libraries should remain loaded even though the lib object goes out of scope
                    setStatus(tr("Loaded %1.").arg(entry));
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
        connect(_ui->sidebarDockWidget, SIGNAL(visibilityChanged(bool)), _ui->action_Sidebar, SLOT(setChecked(bool)), Qt::UniqueConnection);
        connect(_propertyManager, SIGNAL(valueChanged(QtProperty*,QVariant)), this, SLOT(propertyValueChanged(QtProperty*,QVariant)));

        connect(_ui->menu_Edit, SIGNAL(aboutToShow()), this, SLOT(filterEditMenu()));
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

        return true;
    }

    bool MainWindow::openNetwork()
    {
        setStatus("");
        LabNetwork *newNetwork = 0;

        try
        {
            newNetwork = LabNetwork::open(this, QString());
        }
        catch (Automata::Exception & le)
        {
            QMessageBox::critical(this, tr("Unable to open network."), le.message());
            newNetwork = 0;
        }

        if (newNetwork)
        {
            try
            {
                if (_currentDataFile)
                    closeDataFile();
                if (_currentNetwork)
                    closeNetwork();
            }
            catch (Automata::Exception & le)
            {
                QMessageBox::critical(this, tr("Problem closing network."), le.message());
            }

            try
            {
                setNetwork(newNetwork);
                setStatus(tr("Opened %1").arg(newNetwork->fname()));
                return true;
            }
            catch (Automata::Exception & le)
            {
                QMessageBox::critical(this, tr("Unable to open network."), le.message());
            }
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
        if (!closeDataFile())
            return false;

        if (_currentNetwork)
        {
            if (_currentNetwork->changed())
            {
                FileDirtyDialog fdd(tr("Your network file contains changes that have not been saved.  Do you wish to save it, discard it, or cancel closing the network?"));
                fdd.exec();

                switch (fdd.result())
                {
                case FileDirtyDialog::SAVE:
                    if (!saveNetwork())
                        return false;
                    break;
                case FileDirtyDialog::DISCARD:
                    break;
                case FileDirtyDialog::CANCEL:
                    return false;
                }
            }

            setNetwork(0);
        }

        return true;
    }

    bool MainWindow::newDataFile()
    {
        if (!_currentNetwork)
            return false;

        if (!closeDataFile())
            return false;

        _ui->tabWidget->setTabText(1, tr("Collecting Data"));
        _currentDataFile = new NeuroLab::LabDataFile(_currentNetwork, _ui->dataTableWidget, this);
        return true;
    }

    bool MainWindow::closeDataFile()
    {
        if (_currentDataFile && _currentDataFile->changed())
        {
            FileDirtyDialog fdd(tr("You have unsaved data.  Do you wish to save it, discard it, or cancel closing the network?"));
            fdd.exec();

            switch (fdd.result())
            {
            case FileDirtyDialog::SAVE:
                _currentDataFile->saveAs();
                break;
            case FileDirtyDialog::DISCARD:
                break;
            case FileDirtyDialog::CANCEL:
                return false;
            }
        }

        if (_currentDataFile)
        {
            delete _currentDataFile;
            _currentDataFile = 0;

            _ui->dataTableWidget->clearContents();
            _ui->tabWidget->setTabText(1, "");
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
        if (network == _currentNetwork)
            return;

        if (_currentNetwork)
        {
            _networkLayout->removeWidget(_currentNetwork->view());

            delete _currentNetwork;
            _currentNetwork = 0;
        }

        if (network)
        {
            _currentNetwork = network;
            _networkLayout->addWidget(_currentNetwork->view());
            _currentNetwork->view()->show();

            connect(_currentNetwork, SIGNAL(titleChanged(QString)), this, SLOT(setTitle(QString)), Qt::UniqueConnection);
            connect(_currentNetwork, SIGNAL(statusChanged(QString)), this, SLOT(setStatus(QString)), Qt::UniqueConnection);
            connect(_currentNetwork, SIGNAL(propertyObjectChanged(QList<PropertyObject*>)), this, SLOT(setPropertyObjects(QList<PropertyObject*>)), Qt::UniqueConnection);
            connect(_currentNetwork, SIGNAL(actionsEnabled(bool)), this, SLOT(setActionsEnabled(bool)), Qt::UniqueConnection);
            connect(_currentNetwork, SIGNAL(stepProgressRangeChanged(int,int)), this, SLOT(setProgressRange(int, int)), Qt::UniqueConnection);
            connect(_currentNetwork, SIGNAL(stepProgressValueChanged(int)), this, SLOT(setProgressValue(int)), Qt::UniqueConnection);
        }

        setTitle();
        setPropertyObject(_currentNetwork);
        update();
    }

    void MainWindow::setPropertyObject(PropertyObject *po)
    {
        QList<PropertyObject *> property_objects;
        property_objects.append(po);
        setPropertyObjects(property_objects);
    }

    void MainWindow::setPropertyObjects(const QList<PropertyObject *> & property_objects)
    {
        // remove existing properties
        QList<QtProperty *> currentProperties = _propertyEditor->properties();
        for (QListIterator<QtProperty *> i(currentProperties); i.hasNext(); )
        {
            QtProperty *property = i.next();
            _propertyEditor->removeProperty(property);
            delete property;
        }

        // if there is more than one object, make a temporary object to hold the common properties
        _propertyObjects = property_objects;

        delete _noncePropertyObject;
        _noncePropertyObject = 0;
        PropertyObject *cur_obj = 0;

        if (property_objects.size() > 1)
        {
            _noncePropertyObject = new CommonPropertyObject(0, property_objects);
            cur_obj = _noncePropertyObject;
        }
        else if (property_objects.size() == 1)
        {
            // don't assign nonce here, since we don't want to delete it above!
            cur_obj = property_objects[0];
        }

        // get new properties
        if (cur_obj)
        {
            // new top item
            QtProperty *topItem = _propertyManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Properties"));

            cur_obj->buildProperties(_propertyManager, topItem);

            _propertyEditor->addProperty(topItem);
            _propertyEditor->setRootIsDecorated(false);
            _propertyEditor->setPropertiesWithoutValueMarked(false);
        }
    }

    void MainWindow::createdItem(NeuroItem *item)
    {
        if (item)
        {
            _rememberProperties = false;
            setPropertyObject(item);
            _rememberProperties = true;

            QString typeName(typeid(*item).name());

            if (_rememberedProperties.contains(typeName))
            {
                for (QListIterator<PropertyObject::PropertyBase *> i(_rememberedProperties[typeName]); i.hasNext(); )
                {
                    const PropertyObject::PropertyBase *p = i.next();

                    const QString name = p->name();
                    const QVariant value = p->value();
                    item->setPropertyValue(name, value);
                }
            }

            item->setSelected(true);
        }
    }

    /// Remembers the properties for the last selected instance of a particular class.
    void MainWindow::propertyValueChanged(QtProperty *, const QVariant &)
    {
        if (_rememberProperties && _propertyObjects.size() > 0)
        {
            for (QListIterator<PropertyObject *> i(_propertyObjects); i.hasNext(); )
            {
                PropertyObject *po = i.next();

                if (po)
                {
                    QString typeName(typeid(*po).name());
                    _rememberedProperties[typeName] = po->properties();
                }
            }
        }
    }

    void MainWindow::filterEditMenu()
    {
        bool showEditOps = _currentNetwork && _currentNetwork->scene() && _currentNetwork->scene()->selectedItems().size() > 0;

        _ui->action_Cut->setEnabled(showEditOps);
        _ui->action_Copy->setEnabled(showEditOps);
        _ui->action_Paste->setEnabled(_currentNetwork && _currentNetwork->canPaste());
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
    if (_currentDataFile && !closeDataFile())
        return;

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
    newDataFile();
}

void NeuroLab::MainWindow::on_action_Save_Data_Set_triggered()
{
    if (_currentDataFile)
        _currentDataFile->saveAs();
}

void NeuroLab::MainWindow::on_action_Close_Data_Set_triggered()
{
    closeDataFile();
}

void NeuroLab::MainWindow::on_action_Manual_triggered()
{
    QDesktopServices::openUrl(QUrl("http://bitbucket.org/kulibali/neurocogling/wiki/Manual"));
}

void NeuroLab::MainWindow::on_action_About_NeuroLab_triggered()
{
    NeuroLab::AboutDialog about(this);
    about.setLabel(tr("Neurocognitive Linguistics Laboratory v%1").arg(NeuroLab::VERSION));
    about.exec();
}

void NeuroLab::MainWindow::on_action_Cut_triggered()
{
    if (_currentNetwork)
        _currentNetwork->cutSelected();
}

void NeuroLab::MainWindow::on_action_Copy_triggered()
{
    if (_currentNetwork)
        _currentNetwork->copySelected();
}

void NeuroLab::MainWindow::on_action_Paste_triggered()
{
    if (_currentNetwork)
        _currentNetwork->pasteItems();
}
