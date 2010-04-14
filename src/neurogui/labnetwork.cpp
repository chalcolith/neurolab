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

        connect(&_future_watcher, SIGNAL(finished()), this, SLOT(futureFinished()), Qt::UniqueConnection);
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

    QList<QGraphicsItem *> LabNetwork::items() const
    {
        return _tree ? _tree->items() : QList<QGraphicsItem *>();
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

    void LabNetwork::changeItemLabel(NeuroItem *item, const QString & label)
    {
        emit itemLabelChanged(item, label);
    }

    /// Should be changed whenever anything in the the network file formate changes.
    static const QString LAB_SCENE_COOKIE("Neurolab SCENE 009");

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
            if (file.open(QIODevice::ReadOnly))
            {
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
            else
            {
                delete ln;
                throw LabException(tr("Unable to open network file."));
            }
        }

        // read scene data
        {
            QFile file(nln_fname);

            if (file.open(QIODevice::ReadOnly))
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
            else
            {
                delete ln;
                throw LabException(tr("Unable to open network scene file."));
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
            if (file.open(QIODevice::WriteOnly))
            {
                QDataStream data(&file);
                data << *_neuronet;
            }
            else
            {
                throw LabException(tr("Unable to write network file."));
            }
        }

        // write scene
        {
            QFile file(this->_fname);

            if (file.open(QIODevice::WriteOnly))
            {
                // items
                if (_tree)
                {
                    QDataStream data(&file);
                    data.setVersion(QDataStream::Qt_4_5);
                    data << LAB_SCENE_COOKIE;
                    data << *_tree;
                }
            }
            else
            {
                throw LabException(tr("Unable to write network scene file."));
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
                QGraphicsItem *gi = i.peekNext();
                if (!gi)
                    continue;

                NeuroItem *item = dynamic_cast<NeuroItem *>(gi);
                if (item)
                {
                    emit itemDeleted(item);

                    setChanged();

                    if (sc->itemUnderMouse() == item)
                        sc->setItemUnderMouse(0);
                }

                sc->removeItem(gi);
                delete gi;
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
        emit valuesChanged();

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

        if ((_current_step % 3) == 0)
            emit stepIncremented();

        // are we done?
        if (_current_step == _max_steps)
        {
            if (_max_steps > 3)
            {
                emit stepProgressValueChanged(_current_step);
            }

            emit actionsEnabled(true);
            emit statusChanged(tr("Done stepping %1 times.").arg(_max_steps / 3));

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
