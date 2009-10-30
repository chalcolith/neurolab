#include <QFileDialog>
#include <QMessageBox>

#include "labnetwork.h"

namespace NeuroLab
{
    
    LabNetwork::LabNetwork(QWidget *parent)
            : sceneTree(0), network(0), running(false), dirty(false), first_change(true)
    {
        network = new NeuroLib::NeuroNet();
        sceneTree = new LabTree(parent);
                
        connect(sceneTree->getScene(), SIGNAL(changed(QList<QRectF>)), this, SLOT(changed(QList<QRectF>)));
    }
    
    LabNetwork::~LabNetwork()
    {
        delete sceneTree; sceneTree = 0;
        delete network; network = 0;
    }
    
    LabNetwork *LabNetwork::open(QWidget *parent, const QString & fname)
    {
        QString nln_fname = fname;
        
        if (nln_fname.isEmpty() || nln_fname.isNull())
        {
            nln_fname = QFileDialog::getOpenFileName(0, tr("Open Network"), ".", tr("NeuroLab Networks (*.nln)"));
            
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
        ln->fname = nln_fname;
        
        {
            QFile file(network_fname);
            file.open(QIODevice::ReadOnly);
            {
                QDataStream ds(&file);
                ds >> *ln->network;
            }
            file.close();
        }
        
        // initialize from scene data
        {
            QFile file(nln_fname);
            file.open(QIODevice::ReadOnly);
            {
                QDataStream ds(&file);
                /// \todo initialize from scene data
            }
            file.close();
        }
        
        return ln;
    }
    
    void LabNetwork::start()
    {
        running = true;
    }
    
    void LabNetwork::stop()
    {
        running = false;
    }
    
    bool LabNetwork::save(bool saveAs)
    {
        bool prevRunning = this->running;
        stop();
        
        if (!this->sceneTree || !this->network)
            return false;
        
        if (this->fname.isEmpty() || this->fname.isNull() || saveAs)
        {
            this->fname.clear();
            
            QString nln_fname = QFileDialog::getSaveFileName(0, tr("Save Network"), ".", tr("NeuroLab Networks (*.nln)"));
            
            if (nln_fname.isEmpty() || nln_fname.isNull())
                return false;
            
            this->fname = nln_fname;
        }
        
        QString base_name = this->fname.endsWith(".nln", Qt::CaseInsensitive) ? this->fname.left(this->fname.length() - 4) : this->fname;
        QString network_fname = base_name + ".nnn";
        
        // write network
        {
            QFile file(network_fname);
            file.open(QIODevice::WriteOnly);
            {
                QDataStream data(&file);
                data << *network;
            }
            file.close();
        }
        
        /// \todo go through the scene and write
        {
            QFile file(this->fname);
            file.open(QIODevice::WriteOnly);
            
            file.close();
        }
        
        this->dirty = false;
        
        if (prevRunning)
            start();
        
        return true;
    }
    
    bool LabNetwork::close()
    {
        if (dirty && !save())
            return false;
        return true;
    }
    
    static void setSceneMode(LabTreeNode & n, const LabScene::Mode & m)
    {
        n.getScene()->setMode(m);
        
        for (QMutableListIterator<LabTreeNode> i(n.getChildren()); i.hasNext(); )
            setSceneMode(i.next(), m);
    }
    
    void LabNetwork::setMode(const LabScene::Mode & m)
    {
        if (sceneTree)
            setSceneMode(sceneTree->getRoot(), m);
    }
    
    void LabNetwork::changed(const QList<QRectF> &)
    {
        if (!first_change)
            this->dirty = true;
        else
            first_change = false;
    }
    
} // namespace NeuroLab
