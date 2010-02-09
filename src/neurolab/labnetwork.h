#ifndef LABNETWORK_H
#define LABNETWORK_H

#include <QFile>

#include "labtree.h"
#include "../neurolib/neuronet.h"

namespace NeuroLab
{
    
    class NeuroItem;
    
    /// Contains information for working with a NeuroLib::NeuroNet in the GUI.
    class LabNetwork
        : public QObject
    {
        Q_OBJECT
        
        LabTree *_tree;
        NeuroLib::NeuroNet *_neuronet;
        
        bool running;
        
        bool dirty, first_change;
        QString _fname;
        
    public:
        LabNetwork(QWidget *_parent = 0);
        virtual ~LabNetwork();
        
        LabScene *scene() { return _tree ? _tree->scene() : 0; }
        LabView *view() { return _tree ? _tree->view() : 0; }
        const QString & fname() const { return _fname; }
        
        NeuroLib::NeuroNet *neuronet() { return _neuronet; }
        
        NeuroItem *getSelectedItem();
        void deleteSelectedItem();
        void labelSelectedItem(const QString & s);
        
        static LabNetwork *open(QWidget *parent = 0, const QString & fname = QString());
        
    public slots:
        void start();
        void stop();
        void step();
        
        bool save(bool saveAs = false);
        bool close();

        void newNode();
        void newExcitoryLink();
        void newInhibitoryLink();

        void changed();
        void changed(const QList<QRectF> &);
    };
    
} // namespace NeuroLab

#endif // LABNETWORK_H
