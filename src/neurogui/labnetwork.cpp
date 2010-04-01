#include "mainwindow.h"
#include "labnetwork.h"
#include "labtree.h"
#include "labview.h"
#include "labscene.h"
#include "neuroitem.h"
#include "propertyobj.h"
#include "../neurolib/neuronet.h"

#include <QFileDialog>
#include <QMessageBox>

#include <QtVariantPropertyManager>
#include <QtVariantProperty>

using namespace NeuroLib;

namespace NeuroLab
{

    /// Constructor.
    /// \param parent The QObject that should own this network object.
    LabNetwork::LabNetwork(QWidget *parent)
        : PropertyObject(parent),
        _tree(0), _neuronet(0), _running(false), _changed(false), first_change(true),
        _filename_property(this, &LabNetwork::fname, 0, tr("Filename"), false),
        _decay_property(this, &LabNetwork::decay, &LabNetwork::setDecay, tr("Decay Rate")),
        _learn_rate_property(this, &LabNetwork::learnRate, &LabNetwork::setLearnRate, tr("Learn Rate")),
        _learn_time_property(this, &LabNetwork::learnTime, &LabNetwork::setLearnTime, tr("Learn Window")),
        _current_step(0), _max_steps(0)
    {
        _neuronet = new NeuroLib::NeuroNet();
        _tree = new LabTree(parent, this);

        if (_tree->scene())
            connect(_tree->scene(), SIGNAL(changed(QList<QRectF>)), this, SLOT(changed(QList<QRectF>)));

        connect(&_future_watcher, SIGNAL(finished()), this, SLOT(futureFinished()));
    }

    LabNetwork::~LabNetwork()
    {
        delete _tree; _tree = 0;
        delete _neuronet; _neuronet = 0;
    }

    void LabNetwork::setChanged(bool changed)
    {
        bool old = _changed;
        _changed = changed;

        if (old != _changed)
        {
            emit titleChanged(QString());
        }
    }

    LabScene *LabNetwork::scene()
    {
        return _tree ? _tree->scene() : 0;
    }

    LabView *LabNetwork::view()
    {
        return _tree ? _tree->view() : 0;
    }

    QString LabNetwork::fname() const
    {
        int index = _fname.lastIndexOf("/");
        if (index != -1)
            return _fname.mid(index+1);
        else
            return _fname;
    }

    NeuroCell::NeuroValue LabNetwork::decay() const
    {
        Q_ASSERT(_neuronet != 0);
        return _neuronet->decay();
    }

    void LabNetwork::setDecay(const NeuroLib::NeuroCell::NeuroValue & decay)
    {
        Q_ASSERT(_neuronet != 0);
        _neuronet->setDecay(decay);
    }

    NeuroCell::NeuroValue LabNetwork::learnRate() const
    {
        Q_ASSERT(_neuronet != 0);
        return _neuronet->learnRate();
    }

    void LabNetwork::setLearnRate(const NeuroLib::NeuroCell::NeuroValue & learnRate)
    {
        Q_ASSERT(_neuronet != 0);
        _neuronet->setLearnRate(learnRate);
    }

    NeuroCell::NeuroValue LabNetwork::learnTime() const
    {
        Q_ASSERT(_neuronet != 0);
        return _neuronet->learnTime();
    }

    void LabNetwork::setLearnTime(const NeuroLib::NeuroCell::NeuroValue & learnTime)
    {
        Q_ASSERT(_neuronet != 0);
        _neuronet->setLearnTime(learnTime);
    }

    /// Handles setting the main window's properties when the selected item changes.
    void LabNetwork::selectionChanged()
    {
        if (!scene())
            return;

        QList<QGraphicsItem *> items = scene()->selectedItems();
        PropertyObject *po = items.size() == 1 ? dynamic_cast<PropertyObject *>(items[0]) : 0;

        emit propertyObjectChanged(po ? po : this);
    }

    static const QString LAB_SCENE_COOKIE("Neurolab SCENE 008");

    /// Loads a LabNetwork object and its corresponding NeuroNet from a file.
    /// LabNetwork files have the extension .nln; their corresponding NeuroNet files have the extension .nnn.
    /// \param parent The parent widget to use if the file dialog is needed.
    /// \param fname The name of the file from which to load the network.  If this is empty, then the standard file dialog is used to request the name of a file.
    LabNetwork *LabNetwork::open(QWidget *parent, const QString & fname)
    {
        QString nln_fname = fname;

        if (nln_fname.isEmpty() || nln_fname.isNull())
        {
            nln_fname = QFileDialog::getOpenFileName(parent, tr("Open Network"), ".", tr("NeuroLab Networks (*.nln);;All Files (*)"));

            if (nln_fname.isEmpty() || nln_fname.isNull())
                return 0;
        }

        QString base_fname = nln_fname.endsWith(".nln", Qt::CaseInsensitive) ? nln_fname.left(nln_fname.length() - 4) : nln_fname;
        QString network_fname = base_fname + ".nnn";

        if (!(QFile::exists(nln_fname) && QFile::exists(network_fname)))
        {
            QMessageBox::critical(0, tr("Missing Network File"), tr("The corresponding network data file (.NNN) is missing."));
            return 0;
        }

        // read network
        LabNetwork *ln = new LabNetwork(parent);
        ln->_fname = nln_fname;

        {
            QFile file(network_fname);
            file.open(QIODevice::ReadOnly);

            try
            {
                QDataStream data(&file);
                data >> *ln->_neuronet;

                ln->updateProperties();
            }
            catch (...)
            {
                delete ln;
                throw LabException(tr("Network file %1 is not compatible with this version of NeuroLab.").arg(network_fname));
            }
        }

        // read scene data
        {
            QFile file(nln_fname);
            file.open(QIODevice::ReadOnly);

            {
                QDataStream data(&file);
                data.setVersion(QDataStream::Qt_4_5);
                QString cookie;
                data >> cookie;

                if (cookie != LAB_SCENE_COOKIE)
                {
                    delete ln;
                    throw LabException(tr("Network scene file %1 is not compatible with this version of NeuroLab.").arg(nln_fname));
                }

                data >> *ln->_tree;
            }
        }

        //
        return ln;
    }

    /// Saves the network and its NeuroNet.
    /// \param saveAs Forces the file dialog to be used to choose a filename.
    bool LabNetwork::save(bool saveAs)
    {
        bool prevRunning = this->_running;
        stop();

        if (!this->_tree || !this->_neuronet)
            return false;

        if (this->_fname.isEmpty() || this->_fname.isNull() || saveAs)
        {
            this->_fname.clear();

            QString nln_fname = QFileDialog::getSaveFileName(0, tr("Save Network"), ".", tr("NeuroLab Networks (*.nln);;All Files (*)"));

            if (nln_fname.isEmpty() || nln_fname.isNull())
                return false;

            if (!nln_fname.endsWith(".nln", Qt::CaseInsensitive))
                nln_fname = nln_fname + ".nln";

            this->_fname = nln_fname;
        }

        QString base_name = this->_fname.endsWith(".nln", Qt::CaseInsensitive) ? this->_fname.left(this->_fname.length() - 4) : this->_fname;
        QString network_fname = base_name + ".nnn";

        // write network
        {
            QFile file(network_fname);
            file.open(QIODevice::WriteOnly);
            {
                QDataStream data(&file);
                data << *_neuronet;
            }
        }

        // write scene
        {
            QFile file(this->_fname);
            file.open(QIODevice::WriteOnly);

            // items
            if (_tree)
            {
                QDataStream data(&file);
                data.setVersion(QDataStream::Qt_4_5);
                data << LAB_SCENE_COOKIE;
                data << *_tree;
            }
        }

        setChanged(false);

        if (prevRunning)
            start();

        return true;
    }

    /// Saves the network if it has changed since the last change.
    /// \return True if the network was closed successfully.
    bool LabNetwork::close()
    {
        if (_changed && !save())
            return false;
        return true;
    }

    /// Creates a new item with the given type name.
    void LabNetwork::newItem(const QString & typeName)
    {
        if (scene() && view())
        {
            scene()->newItem(typeName, scene()->lastMousePos());
            setChanged();
        }
    }

    /// Deletes the selected items.
    void LabNetwork::deleteSelected()
    {
        LabScene *sc = scene();

        if (sc)
        {
            for (QListIterator<QGraphicsItem *> i(sc->selectedItems()); i.hasNext(); i.next())
            {
                QGraphicsItem *item = i.peekNext();
                setChanged();

                if (sc->itemUnderMouse() == dynamic_cast<NeuroItem *>(item))
                    sc->setItemUnderMouse(0);

                sc->removeItem(item);
                delete item;
            }
        }
    }

    /// Starts the network running (currently not implemented).
    void LabNetwork::start()
    {
        _running = true;
    }

    /// Stops the network running (currently not implemented).
    void LabNetwork::stop()
    {
        _running = false;
        _tree->update();
    }

    /// Advances the network by one timestep.
    void LabNetwork::step(int numSteps)
    {
        if (_running)
            return; // can't happen

        if (numSteps <= 0)
            return;

        setChanged();

        _running = true;
        _current_step = 0;
        _max_steps = numSteps * 3; // takes 3 steps of the automaton to fully process
        _step_time.start();

        emit actionsEnabled(false);

        if (numSteps > 1)
        {
            emit statusChanged(tr("Stepping %1 times...").arg(numSteps));
            emit stepProgressRangeChanged(0, _max_steps);
            emit stepProgressValueChanged(0);
        }

        _future_watcher.setFuture(_neuronet->stepAsync());
    }

    void LabNetwork::futureFinished()
    {
        ++_current_step;
        _future_watcher.waitForFinished();

        // are we done?
        if (_current_step == _max_steps)
        {
            if (_max_steps > 3)
            {
                emit stepProgressValueChanged(_current_step);
            }

            emit actionsEnabled(true);
            emit statusChanged(tr("Done."));

            _running = false;
            setChanged();
            _tree->update();
        }
        else
        {
            _future_watcher.setFuture(_neuronet->stepAsync());

            // only change the display 10 times a second
            if (_step_time.elapsed() >= 100)
            {
                _step_time.start();

                if (_max_steps > 3)
                {
                    emit stepProgressValueChanged(_current_step);
                }

                _tree->update();
            }
        }
    }

    /// Resets the network: all nodes and links that are not frozen have their output values set to 0.
    void LabNetwork::reset()
    {
        emit statusChanged("");
        setChanged();
        _tree->reset();
    }

} // namespace NeuroLab
