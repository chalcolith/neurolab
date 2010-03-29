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

namespace NeuroLab
{

    /// Constructor.
    /// \param parent The QObject that should own this network object.
    LabNetwork::LabNetwork(QWidget *parent)
        : QObject(parent), PropertyObject(),
        _tree(0), _neuronet(0), running(false), _dirty(false), first_change(true),
        filename_property(0), decay_property(0), learn_property(0), learn_time_property(0)
    {
        _neuronet = new NeuroLib::NeuroNet();
        _tree = new LabTree(parent, this);

        if (_tree->scene())
            connect(_tree->scene(), SIGNAL(changed(QList<QRectF>)), this, SLOT(changed(QList<QRectF>)));
    }

    LabNetwork::~LabNetwork()
    {
        delete _tree; _tree = 0;
        delete _neuronet; _neuronet = 0;
    }

    void LabNetwork::setDirty(bool dirty)
    {
        bool old = _dirty;
        _dirty = dirty;

        if (old != _dirty)
            MainWindow::instance()->setTitle();
    }

    LabScene *LabNetwork::scene()
    {
        return _tree ? _tree->scene() : 0;
    }

    LabView *LabNetwork::view()
    {
        return _tree ? _tree->view() : 0;
    }

    /// Adds the network's properties to the parent property.
    /// \param manager The property manager with which to create properties if necessary.
    /// \param parentItem The parent property to which to add new properties.
    void LabNetwork::buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem)
    {
        if (_properties.count() == 0)
        {
            manager->connect(manager, SIGNAL(valueChanged(QtProperty*,QVariant)), this, SLOT(propertyValueChanged(QtProperty*,QVariant)));

            filename_property = manager->addProperty(QVariant::String, tr("Filename"));
            filename_property->setEnabled(false);

            QString fname = _fname;
            int index = fname.lastIndexOf('/');
            if (index == -1)
                index = fname.lastIndexOf('\\');
            if (index != -1)
                fname = fname.mid(index+1);

            filename_property->setValue(QVariant(fname));
            _properties.append(filename_property);

            decay_property = manager->addProperty(QVariant::Double, tr("Decay Rate"));
            _properties.append(decay_property);

            learn_property = manager->addProperty(QVariant::Double, tr("Learn Rate"));
            _properties.append(learn_property);

            learn_time_property = manager->addProperty(QVariant::Double, tr("Learn Time"));
            _properties.append(learn_time_property);
        }

        parentItem->setPropertyName(tr("Network"));
        PropertyObject::buildProperties(manager, parentItem);

        //
        decay_property->setValue(QVariant(_neuronet->decay()));
        learn_property->setValue(QVariant(_neuronet->learn()));
        learn_time_property->setValue(QVariant(static_cast<int>(_neuronet->learnTime())));
    }

    /// Updates the properties from the values in memory.
    void LabNetwork::updateProperties()
    {
        _updating = true;

        if (!_neuronet)
            return;

        if (decay_property)
            decay_property->setValue(QVariant(_neuronet->decay()));
        if (learn_property)
            learn_property->setValue(QVariant(_neuronet->learn()));
        if (learn_time_property)
            learn_time_property->setValue(QVariant(_neuronet->learn()));

        _updating = false;
    }

    /// Changes the values in memory if the properties in the property grid change.
    void LabNetwork::propertyValueChanged(QtProperty *property, const QVariant & value)
    {
        if (_updating)
            return;

        QtVariantProperty *vprop = dynamic_cast<QtVariantProperty *>(property);
        float prev;
        bool changed = false;

        if (vprop == decay_property)
        {
            prev = _neuronet->decay();
            _neuronet->setDecay(value.toFloat());
            changed = prev != _neuronet->decay();
        }
        else if (vprop == learn_property)
        {
            prev = _neuronet->learn();
            _neuronet->setLearn(value.toFloat());
            changed = prev != _neuronet->learn();
        }
        else if (vprop == learn_time_property)
        {
            prev = _neuronet->learnTime();
            _neuronet->setLearnTime(value.toFloat());
            changed = prev != _neuronet->learnTime();
        }

        if (changed)
            setDirty(true);
    }

    /// Handles setting the main window's properties when the selected item changes.
    void LabNetwork::selectionChanged()
    {
        if (!scene())
            return;

        QList<QGraphicsItem *> items = scene()->selectedItems();
        PropertyObject *po = items.size() == 1 ? dynamic_cast<PropertyObject *>(items[0]) : 0;

        MainWindow::instance()->setPropertyObject(po ? po : this);
    }

    static const QString LAB_SCENE_COOKIE("Neurolab SCENE 007");

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
        bool prevRunning = this->running;
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

        setDirty(false);

        if (prevRunning)
            start();

        return true;
    }

    /// Saves the network if it has changed since the last change.
    /// \return True if the network was closed successfully.
    bool LabNetwork::close()
    {
        if (_dirty && !save())
            return false;
        return true;
    }

    /// Creates a new item with the given type name.
    void LabNetwork::newItem(const QString & typeName)
    {
        if (scene() && view())
        {
            scene()->newItem(typeName, scene()->lastMousePos());
            setDirty();
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
                setDirty();

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
        running = true;
    }

    /// Stops the network running (currently not implemented).
    void LabNetwork::stop()
    {
        running = false;
        _tree->update();
    }

    /// Advances the network by one timestep.
    void LabNetwork::step()
    {
        running = true;
        _neuronet->step();
        _neuronet->step();
        _neuronet->step();
        running = false;

        setDirty();
        _tree->update();
    }

    /// Resets the network: all nodes and links that are not frozen have their output values set to 0.
    void LabNetwork::reset()
    {
        setDirty();
        _tree->reset();
    }

} // namespace NeuroLab
