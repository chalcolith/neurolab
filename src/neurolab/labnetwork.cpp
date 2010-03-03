#include "mainwindow.h"
#include "labnetwork.h"
#include "../automata/exception.h"

#include <QFileDialog>
#include <QMessageBox>

#include <QtVariantPropertyManager>
#include <QtVariantProperty>

namespace NeuroLab
{
    
    LabNetwork::LabNetwork(QWidget *_parent)
        : _tree(0), _neuronet(0), running(false), _dirty(false), first_change(true), filename_property(0)
    {
        _neuronet = new NeuroLib::NeuroNet();
        _tree = new LabTree(_parent, this);

        if (_tree->scene())
            connect(_tree->scene(), SIGNAL(changed(QList<QRectF>)), this, SLOT(changed(QList<QRectF>)));
    }
    
    LabNetwork::~LabNetwork()
    {
        delete _tree; _tree = 0;
        delete _neuronet; _neuronet = 0;
    }
    
    void LabNetwork::buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem)
    {
        if (_properties.count() == 0)
        {
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
        }
        
        parentItem->setPropertyName(tr("Network"));
        PropertyObject::buildProperties(manager, parentItem);
    }
    
    NeuroItem *LabNetwork::getSelectedItem()
    {
        return (_tree && _tree->scene()) ? _tree->scene()->selectedItem() : 0;
    }
    
    void LabNetwork::deleteSelectedItem()
    {
        if (_tree && _tree->scene())
            _tree->scene()->deleteSelectedItem();
        changed();
    }
    
    void LabNetwork::labelSelectedItem(const QString & s)
    {
        if (_tree && _tree->scene())
            _tree->scene()->labelSelectedItem(s);
        changed();
    }    
    
    static const QString LAB_SCENE_COOKIE("Neurolab SCENE 002");
    
    LabNetwork *LabNetwork::open(QWidget *parent, const QString & fname)
    {
        QString nln_fname = fname;
        
        if (nln_fname.isEmpty() || nln_fname.isNull())
        {
            nln_fname = QFileDialog::getOpenFileName(parent, tr("Open Network"), ".", tr("NeuroLab Networks (*.nln);;All Files (*.*)"));
            
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
                QDataStream ds(&file);
                ds >> *ln->_neuronet;
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
                QDataStream ds(&file);
                QString cookie;
                ds >> cookie;
                if (cookie != LAB_SCENE_COOKIE)
                {
                    delete ln;
                    throw LabException(tr("Scene file %1 is not compatible with this version of NeuroLab.").arg(nln_fname));
                }
                
                ds >> *ln->_tree;
            }
        }
        
        //
        return ln;
    }
    
    bool LabNetwork::save(bool saveAs)
    {
        bool prevRunning = this->running;
        stop();
        
        if (!this->_tree || !this->_neuronet)
            return false;
        
        if (this->_fname.isEmpty() || this->_fname.isNull() || saveAs)
        {
            this->_fname.clear();
            
            QString nln_fname = QFileDialog::getSaveFileName(0, tr("Save Network"), ".", tr("NeuroLab Networks (*.nln);;All Files (*.*)"));
            
            if (nln_fname.isEmpty() || nln_fname.isNull())
                return false;
            
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
                data << LAB_SCENE_COOKIE;
                data << *_tree;
            }
        }
        
        this->_dirty = false;
        
        if (prevRunning)
            start();
        
        return true;
    }
    
    bool LabNetwork::close()
    {
        if (_dirty && !save())
            return false;
        return true;
    }

    void LabNetwork::changed(const QList<QRectF> &)
    {
        changed();
    }

    void LabNetwork::changed()
    {
        if (!first_change)
            this->_dirty = true;
        else
            first_change = false;
    }
    
    void LabNetwork::newItem(const QString & typeName)
    {
        if (this->scene())
            this->scene()->newItem(typeName);
        changed();
    }
    
    void LabNetwork::start()
    {
        running = true;
        changed();
    }
    
    void LabNetwork::stop()
    {
        running = false;
        _tree->update();
        changed();
    }
    
    void LabNetwork::step()
    {
        running = true;
        
        // three times to fully run through the asynchronous algorithm
        _neuronet->step();
        _neuronet->step();
        _neuronet->step();
        running = false;
        
        _tree->update();
        changed();
    }
    
    void LabNetwork::reset()
    {
        _tree->reset();
        changed();
    }
    
} // namespace NeuroLab
