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

#include "labexception.h"

#include "labscene.h"
#include "neuroitem.h"
#include "../automata/exception.h"

#include "filedirtydialog.h"
#include "aboutdialog.h"
#include "labview.h"
#include "labscene.h"
#include "labnetwork.h"
#include "labtree.h"
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

namespace NeuroGui
{

    const QString
#include "../version.txt"
    ;

    MainWindow *MainWindow::_instance = 0;

    QDir MainWindow::LAST_DIRECTORY = QDir::current();

    MainWindow::MainWindow(QWidget *parent, const QString & title, const QString & initialFname)
        : QMainWindow(parent),
          _title(title),
          _ui(new Ui::MainWindow()),
          _networkLayout(0),
          _zoomSpinBox(0),
          _zoomSpinBoxAction(0),
          _numStepsSpinBox(0),
          _numStepsSpinBoxAction(0),
          _stepProgressBar(0),
          _breadCrumbBar(0),
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

        // set up ui
        _ui->setupUi(this);
        this->setupUi();

        // load plugins
        loadPlugins();

        // set up connections
        setupConnections();

        // read state from settings
        loadStateSettings();

        // load initial network
        if (initialFname.isNull() || initialFname.isEmpty())
            newNetwork();
        else
            setNetwork(LabNetwork::open(initialFname));
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

    void MainWindow::setupUi()
    {
        setTitle();

        QVBoxLayout *sidebarLayout = new QVBoxLayout(_ui->sidebar_page_1);
        sidebarLayout->addWidget(_propertyEditor = new QtTreePropertyBrowser(this));
        _propertyEditor->setFactoryForManager(_propertyManager, _propertyFactory);

        _zoomSpinBox = new QSpinBox(this);
        _zoomSpinBox->setRange(10, 1000);
        _zoomSpinBox->setSuffix(QString("%"));
        _zoomSpinBox->setValue(100);
        _zoomSpinBox->setSingleStep(10);
        _zoomSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        _zoomSpinBoxAction = _ui->viewToolbar->insertWidget(_ui->action_Zoom_Out, _zoomSpinBox);

        _numStepsSpinBox = new QSpinBox(this);
        _numStepsSpinBox->setRange(0, 1000000);
        _numStepsSpinBox->setValue(1);
        _numStepsSpinBoxAction = _ui->simulationToolbar->insertWidget(_ui->action_Step, _numStepsSpinBox);

        _stepProgressBar = new QProgressBar(this);
        _stepProgressBar->setVisible(false);
        _stepProgressBar->reset();
        _ui->statusBar->addPermanentWidget(_stepProgressBar);

        _ui->tabWidget->setTabText(1, "");

        // network tab widget
        _networkLayout = new QVBoxLayout(_ui->tab_1);

        _breadCrumbBar = new QToolBar(tr("breadCrumbBar"));
        _networkLayout->addWidget(_breadCrumbBar);
        _breadCrumbBar->setVisible(false);
    }

    void MainWindow::setupConnections()
    {
        connect(_ui->menu_File, SIGNAL(aboutToShow()), this, SLOT(filterFileMenu()));
        connect(_ui->menu_Edit, SIGNAL(aboutToShow()), this, SLOT(filterEditMenu()));

        connect(_ui->sidebarDockWidget, SIGNAL(visibilityChanged(bool)), _ui->action_Sidebar, SLOT(setChecked(bool)), Qt::UniqueConnection);
        connect(_ui->mainToolBar->toggleViewAction(), SIGNAL(toggled(bool)), _ui->action_Main_Toolbar, SLOT(setChecked(bool)), Qt::UniqueConnection);
        connect(_ui->viewToolbar->toggleViewAction(), SIGNAL(toggled(bool)), _ui->action_View_Toolbar, SLOT(setChecked(bool)), Qt::UniqueConnection);
        connect(_ui->simulationToolbar->toggleViewAction(), SIGNAL(toggled(bool)), _ui->action_Simulation_Toolbar, SLOT(setChecked(bool)), Qt::UniqueConnection);

        connect(_zoomSpinBox, SIGNAL(valueChanged(int)), this, SLOT(zoomValueChanged(int)), Qt::UniqueConnection);

        connect(_propertyManager, SIGNAL(valueChanged(QtProperty*,QVariant)), this, SLOT(propertyValueChanged(QtProperty*,QVariant)));
    }

    void MainWindow::loadPlugins()
    {
#ifdef __APPLE__
        QString pluginPath = QCoreApplication::applicationDirPath() + "/../Frameworks";
        {
            QDir dir(pluginPath);
            QStringList entries = dir.entryList(QDir::Dirs);
            foreach (QString entry, entries)
            {
                if (!entry.startsWith("."))
                    loadPlugins(pluginPath + "/" + entry + "/Versions/Current");
            }
        }

        pluginPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
        loadPlugins(pluginPath);

        {
            QDir dir(pluginPath);
            QStringList entries = dir.entryList(QDir::Dirs);
            foreach (QString entry, entries)
            {
                if (!entry.startsWith("."))
                    loadPlugins(pluginPath + "/" + entry);
            }
        }
#else
        QString pluginPath = QCoreApplication::applicationDirPath() + "/plugins";
        loadPlugins(pluginPath);

        pluginPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
        loadPlugins(pluginPath);
#endif
    }

    void MainWindow::loadPlugins(const QString & dirPath)
    {
        QDir dir(dirPath);
        if (!dir.exists())
            return;

        QStringList entries = dir.entryList(QDir::Files);
        foreach (QString entry, entries)
        {
            QString fname = dirPath + "/" + entry;
#ifndef __APPLE__
            if (QLibrary::isLibrary(fname))
#endif
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
        LAST_DIRECTORY = QDir(settings.value("last_dir", QVariant(LAST_DIRECTORY.absolutePath())).toString());
        settings.endGroup();
    }

    void MainWindow::saveStateSettings()
    {
        QSettings settings;
        settings.beginGroup("MainWindow");
        settings.setValue("size", size());
        settings.setValue("pos", pos());
        settings.setValue("status", saveState(NEUROLAB_APP_VERSION()));
        settings.setValue("last_dir", QVariant(LAST_DIRECTORY.absolutePath()));
        settings.endGroup();
    }

    void MainWindow::setActionsEnabled(bool enabled)
    {
        foreach (QAction *i, this->actions())
            i->setEnabled(enabled);

        foreach (QAction *i, _ui->menuBar->actions())
            i->setEnabled(enabled);

        foreach (QAction *i, _ui->mainToolBar->actions())
            i->setEnabled(enabled);

        foreach (QAction *i, _ui->viewToolbar->actions())
            i->setEnabled(enabled);

        foreach (QAction *i, _ui->simulationToolbar->actions())
            i->setEnabled(enabled);

        if (_breadCrumbBar)
        {
            foreach (QAction *i, _breadCrumbBar->actions())
                i->setEnabled(enabled);
        }

        if (_zoomSpinBox)
            _zoomSpinBox->setEnabled(enabled);

        if (_zoomSpinBoxAction)
            _zoomSpinBoxAction->setEnabled(enabled);

        if (_numStepsSpinBox)
            _numStepsSpinBox->setEnabled(enabled);

        if (_numStepsSpinBoxAction)
            _numStepsSpinBoxAction->setEnabled(enabled);

        if (enabled)
            filterActions();
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

    bool MainWindow::openNetwork(const QString & fname)
    {
        if (_currentDataFile && _currentDataFile->changed())
        {
            FileDirtyDialog fdd(this, tr("You have unsaved data.  Do you wish to save it, discard it, or cancel closing the network?"));
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

        if (_currentNetwork && _currentNetwork->changed())
        {
            FileDirtyDialog fdd(this, tr("Your network file contains changes that have not been saved.  Do you wish to save it, discard it, or cancel closing the network?"));
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

        setStatus("");
        LabNetwork *newNetwork = 0;

        try
        {
            newNetwork = LabNetwork::open(fname);
        }
        catch (Automata::Exception & le)
        {
            QMessageBox::critical(this, tr("Unable to open network file %1").arg(fname), le.message());
            newNetwork = 0;
        }

        if (newNetwork)
        {
            try
            {
                if (_currentDataFile)
                {
                    _currentDataFile->setChanged(false);
                    closeDataFile();
                }

                if (_currentNetwork)
                {
                    _currentNetwork->setChanged(false);
                    closeNetwork();
                }
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
                FileDirtyDialog fdd(this, tr("Your network file contains changes that have not been saved.  Do you wish to save it, discard it, or cancel closing the network?"));
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

    bool MainWindow::reloadNetwork()
    {
        if (_currentNetwork && !_currentNetwork->fname().isEmpty())
        {
            bool dataChanged = _currentDataFile ? _currentDataFile->changed() : false;
            bool networkChanged = _currentNetwork->changed();

            if (_currentDataFile)
                _currentDataFile->setChanged(false);
            _currentNetwork->setChanged(false);

            QString fname = _currentNetwork->fullPath();
            if (openNetwork(fname))
            {
                return true;
            }
            else
            {
                if (_currentDataFile)
                    _currentDataFile->setChanged(dataChanged);
                if (_currentNetwork)
                    _currentNetwork->setChanged(networkChanged);
            }
        }

        return false;
    }

    bool MainWindow::newDataFile()
    {
        if (!_currentNetwork)
            return false;

        if (!closeDataFile())
            return false;

        _ui->tabWidget->setTabText(1, tr("Collecting Data"));
        _ui->dataTableWidget->setToolTip(tr("Nodes with labels will be recorded here."));
        _currentDataFile = new NeuroGui::LabDataFile(_currentNetwork, _ui->dataTableWidget, this);
        return true;
    }

    bool MainWindow::closeDataFile()
    {
        if (_currentDataFile && _currentDataFile->changed())
        {
            FileDirtyDialog fdd(this, tr("You have unsaved data.  Do you wish to save it, discard it, or cancel closing the network?"));
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

    void MainWindow::networkChanged(const QString &title)
    {
        setTitle(title);
        filterActions();
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
        qDebug() << status;
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

        _rememberedProperties.clear();

        if (_currentNetwork)
        {
            disconnect(_currentNetwork, SIGNAL(networkChanged(QString)), this, SLOT(networkChanged(QString)));
            disconnect(_currentNetwork, SIGNAL(statusChanged(QString)), this, SLOT(setStatus(QString)));
            disconnect(_currentNetwork, SIGNAL(propertyObjectChanged(QList<PropertyObject*>)),
                       this, SLOT(setPropertyObjects(QList<PropertyObject*>)));
            disconnect(_currentNetwork, SIGNAL(itemDeleted(NeuroItem*)), this, SLOT(deletedItem(NeuroItem*)));

            disconnect(_currentNetwork, SIGNAL(actionsEnabled(bool)), this, SLOT(setActionsEnabled(bool)));
            disconnect(_currentNetwork, SIGNAL(stepProgressRangeChanged(int,int)), this, SLOT(setProgressRange(int, int)));
            disconnect(_currentNetwork, SIGNAL(stepProgressValueChanged(int)), this, SLOT(setProgressValue(int)));

            _currentNetwork->removeWidgetsFrom(_networkLayout);

            delete _currentNetwork;
            _currentNetwork = 0;
        }

        if (network)
        {
            _currentNetwork = network;
            setSubNetwork(_currentNetwork->treeNode());

            connect(_currentNetwork, SIGNAL(networkChanged(QString)), this, SLOT(networkChanged(QString)), Qt::UniqueConnection);
            connect(_currentNetwork, SIGNAL(statusChanged(QString)), this, SLOT(setStatus(QString)), Qt::UniqueConnection);
            connect(_currentNetwork, SIGNAL(propertyObjectChanged(QList<PropertyObject*>)),
                    this, SLOT(setPropertyObjects(QList<PropertyObject*>)), Qt::UniqueConnection);
            connect(_currentNetwork, SIGNAL(itemDeleted(NeuroItem*)), this, SLOT(deletedItem(NeuroItem*)));

            connect(_currentNetwork, SIGNAL(actionsEnabled(bool)), this, SLOT(setActionsEnabled(bool)), Qt::UniqueConnection);
            connect(_currentNetwork, SIGNAL(stepProgressRangeChanged(int,int)), this, SLOT(setProgressRange(int, int)), Qt::UniqueConnection);
            connect(_currentNetwork, SIGNAL(stepProgressValueChanged(int)), this, SLOT(setProgressValue(int)), Qt::UniqueConnection);
        }
        else
        {
            _zoomSpinBox->setValue(100);
        }

        setTitle();
        setPropertyObject(_currentNetwork);
        update();

        filterActions();

        if (_breadCrumbBar)
        {
            _breadCrumbBar->setVisible(_currentNetwork != 0);
        }
    }

    void MainWindow::setSubNetwork(LabTreeNode *treeNode)
    {
        Q_ASSERT(_networkLayout);

        // sanity checks
        if (!_currentNetwork || !treeNode)
            return;
        if (treeNode->tree()->network() != _currentNetwork)
            throw LabException(tr("Internal error: trying to set a subnetwork that is not part of the current network."));

        // remove the current network's view
        if (_currentNetwork->scene())
        {
            _currentNetwork->scene()->setItemUnderMouse(0);
        }

        if (_currentNetwork->view())
        {
            _currentNetwork->view()->hide();
            _networkLayout->removeWidget(_currentNetwork->view());
        }

        // set the new node
        _currentNetwork->setTreeNode(treeNode);

        if (_currentNetwork->scene())
        {
            _currentNetwork->scene()->clearSelection();
            _currentNetwork->scene()->setItemUnderMouse(0);
        }

        if (_currentNetwork->view())
        {
            _networkLayout->addWidget(_currentNetwork->view());
            _currentNetwork->view()->show();
            _zoomSpinBox->setValue(_currentNetwork->view()->zoom());
        }

        treeNode->updateItemProperties();
        setPropertyObject(_currentNetwork);

        updateBreadcrumbs();
    }

    void MainWindow::treeNodeDeleted(LabTreeNode *treeNode)
    {
        _breadCrumbs.removeAll(treeNode);
        updateBreadcrumbs();
    }

    void MainWindow::updateBreadcrumbs()
    {
        _breadCrumbBar->clear();

        // create breadcrumbs
        if (_breadCrumbBar && _currentNetwork)
        {
            // get new breadcrumbs for this network
            QList<LabTreeNode *> newBreadCrumbs;
            LabTreeNode *newNode = _currentNetwork->treeNode();
            while (newNode)
            {
                newBreadCrumbs.insert(0, newNode);
                newNode = newNode->parent();
            }

            // see if we're on a common path from the old breadcrumbs,
            // and whether or not we can maintain the old ones going
            // deeper than us in the tree
            int num = _breadCrumbs.size();
            for (int i = 0; i < num; ++i)
            {
                if (i < newBreadCrumbs.size() && newBreadCrumbs[i] != _breadCrumbs[i])
                    break;
                if (i >= newBreadCrumbs.size())
                    newBreadCrumbs.append(_breadCrumbs[i]);
            }

            // set the new breadcrumbs
            _breadCrumbs = newBreadCrumbs;
            num = _breadCrumbs.size();
            for (int i = 0; i < num; ++i)
            {
                if (_breadCrumbs[i]->currentAction())
                {
                    _breadCrumbBar->addAction(_breadCrumbs[i]->currentAction());
                }
                else
                {
                    QAction *action = _breadCrumbBar->addAction(_breadCrumbs[i]->label());
                    action->setCheckable(true);
                    connect(_breadCrumbs[i], SIGNAL(nodeSelected(LabTreeNode*)), this, SLOT(setSubNetwork(LabTreeNode*)));

                    _breadCrumbs[i]->setCurrentAction(action);
                }

                _breadCrumbs[i]->currentAction()->setChecked(_breadCrumbs[i] == _currentNetwork->treeNode());
            }
        }
    }

    void MainWindow::setPropertyObject(PropertyObject *po)
    {
        QList<PropertyObject *> property_objects;
        if (po)
            property_objects.append(po);
        setPropertyObjects(property_objects);
    }

    void MainWindow::setPropertyObjects(const QList<PropertyObject *> & property_objects)
    {
        // remove existing properties
        QList<QtProperty *> currentProperties = _propertyEditor->properties();
        foreach (QtProperty *property, currentProperties)
        {
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
                foreach (const PropertyObject::PropertyBase *p, _rememberedProperties[typeName])
                {
                    if (!p->remember())
                        continue;

                    const QString name = p->name();
                    const QVariant value = p->value();
                    item->setPropertyValue(name, value);
                }
            }

            item->setSelected(true);
        }
    }

    void MainWindow::deletedItem(NeuroItem *item)
    {
        QString typeName(typeid(*item).name());
        _rememberedProperties.remove(typeName);
    }

    /// Remembers the properties for the last selected instance of a particular class.
    void MainWindow::propertyValueChanged(QtProperty *, const QVariant &)
    {
        if (_rememberProperties && _propertyObjects.size() > 0)
        {
            foreach (PropertyObject *po, _propertyObjects)
            {
                if (po)
                {
                    QString typeName(typeid(*po).name());
                    _rememberedProperties[typeName] = po->properties();
                }
            }
        }
    }

    void MainWindow::zoomValueChanged(int val)
    {
        if (_currentNetwork)
            _currentNetwork->setZoom(val);
    }

    void MainWindow::filterActions()
    {
        filterFileMenu();
        filterEditMenu();
    }

    void MainWindow::filterFileMenu()
    {
        bool showPrint = false;
        bool showExport = false;
        if (_ui->tabWidget->currentWidget() == _ui->tab_1 && _currentNetwork && _currentNetwork->items().size() > 0)
        {
            showPrint = true;
            showExport = true;
        }
//        else if (_ui->tabWidget->currentWidget() == _ui->tab_2 && _currentDataFile && _currentDataFile->table() && _currentDataFile->table()->rowCount() > 0)
//        {
//            showPrint = true;
//        }

        _ui->action_Reload_Network->setEnabled(showPrint && _currentNetwork && _currentNetwork->changed() && !_currentNetwork->fname().isEmpty());
        _ui->action_Save->setEnabled(_currentNetwork && _currentNetwork->changed());
        _ui->action_Close->setEnabled(_currentNetwork);

        _ui->action_Print->setEnabled(showPrint);

        _ui->menuExport_to->setEnabled(showExport);
        _ui->action_SVG->setEnabled(showExport);
        _ui->action_PNG->setEnabled(showExport);
        _ui->action_PS->setEnabled(showExport);
        _ui->action_PDF->setEnabled(showExport);

        bool showData = _currentDataFile;
        _ui->action_Save_Data_Set->setEnabled(showData);
        _ui->action_Close_Data_Set->setEnabled(showData);
    }

    void MainWindow::filterEditMenu()
    {
        bool showEditOps = _currentNetwork && _currentNetwork->scene() && _currentNetwork->scene()->selectedItems().size() > 0;

        _ui->action_Cut->setEnabled(showEditOps);
        _ui->action_Copy->setEnabled(showEditOps);
        _ui->action_Paste->setEnabled(_currentNetwork && _currentNetwork->canPaste());
        _ui->action_Delete->setEnabled(showEditOps);
    }

} // namespace NeuroGui


// action slots

void NeuroGui::MainWindow::on_action_New_triggered()
{
    try
    {
        newNetwork();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Open_triggered()
{
    try
    {
        if (!openNetwork() && !_currentNetwork)
            newNetwork();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Close_triggered()
{
    try
    {
        closeNetwork();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Reload_Network_triggered()
{
    try
    {
        reloadNetwork();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Save_triggered()
{
    try
    {
        saveNetwork();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Quit_triggered()
{
    try
    {
        close();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Sidebar_triggered()
{
    try
    {
        _ui->sidebarDockWidget->toggleViewAction()->trigger();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Main_Toolbar_triggered()
{
    try
    {
        _ui->mainToolBar->toggleViewAction()->trigger();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_View_Toolbar_triggered()
{
    try
    {
        _ui->viewToolbar->toggleViewAction()->trigger();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}


void NeuroGui::MainWindow::on_action_Simulation_Toolbar_triggered()
{
    try
    {
        _ui->simulationToolbar->toggleViewAction()->trigger();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Start_triggered()
{
}

void NeuroGui::MainWindow::on_action_Stop_triggered()
{
}

void NeuroGui::MainWindow::on_action_Step_triggered()
{
    try
    {
        if (_currentNetwork)
            _currentNetwork->step(_numStepsSpinBox->value());
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Reset_triggered()
{
    try
    {
        if (_currentDataFile && !closeDataFile())
            return;

        if (_currentNetwork)
            _currentNetwork->reset();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Delete_triggered()
{
    try
    {
        if (_currentNetwork)
            _currentNetwork->deleteSelected();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_New_Data_Set_triggered()
{
    try
    {
        newDataFile();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Save_Data_Set_triggered()
{
    try
    {
        if (_currentDataFile)
            _currentDataFile->saveAs();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Close_Data_Set_triggered()
{
    try
    {
        closeDataFile();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Manual_triggered()
{
    try
    {
        QDesktopServices::openUrl(QUrl("http://bitbucket.org/kulibali/neurocogling/wiki/Manual"));
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_About_NeuroLab_triggered()
{
    try
    {
        NeuroGui::AboutDialog about(this);
        about.setLabel(tr("Neurocognitive Linguistics Laboratory v%1").arg(NeuroGui::VERSION));
        about.exec();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Cut_triggered()
{
    try
    {
        if (_currentNetwork)
            _currentNetwork->cutSelected();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Copy_triggered()
{
    try
    {
        if (_currentNetwork)
            _currentNetwork->copySelected();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Paste_triggered()
{
    try
    {
        if (_currentNetwork)
            _currentNetwork->pasteItems();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Select_All_triggered()
{
    try
    {
        if (_currentNetwork)
            _currentNetwork->selectAll();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Print_triggered()
{
    try
    {
        // are we in the scene or the data file?
        QWidget *current = _ui->tabWidget->currentWidget();

        if (current == _ui->tab_1 && _currentNetwork)
        {
            _currentNetwork->exportPrint();
        }
        //    else if (current == _ui->tab_2 && _currentDataFile)
        //    {
        //    }
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_SVG_triggered()
{
    try
    {
        if (_currentNetwork)
            _currentNetwork->exportSVG();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_PNG_triggered()
{
    try
    {
        if (_currentNetwork)
            _currentNetwork->exportPNG();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_PS_triggered()
{
    try
    {
        if (_currentNetwork)
            _currentNetwork->exportPS();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_PDF_triggered()
{
    try
    {
        if (_currentNetwork)
            _currentNetwork->exportPDF();
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Zoom_In_triggered()
{
    try
    {
        if (_zoomSpinBox && _currentNetwork)
        {
            int prev = _zoomSpinBox->value();
            int step = _zoomSpinBox->singleStep();
            _zoomSpinBox->setValue(prev + step);
        }
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}

void NeuroGui::MainWindow::on_action_Zoom_Out_triggered()
{
    try
    {
        if (_zoomSpinBox && _currentNetwork)
        {
            int prev = _zoomSpinBox->value();
            int step = _zoomSpinBox->singleStep();
            _zoomSpinBox->setValue(prev - step);
        }
    }
    catch (Automata::Exception & e)
    {
        QMessageBox::critical(this, tr("Error"), e.message());
    }
}
