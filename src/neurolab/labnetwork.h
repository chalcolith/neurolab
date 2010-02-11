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
        
        bool _dirty, first_change;
        QString _fname;
        
    public:
        LabNetwork(QWidget *_parent = 0);
        virtual ~LabNetwork();
        
        bool dirty() { return _dirty; }
        
        LabScene *scene() { return _tree ? _tree->scene() : 0; }
        LabView *view() { return _tree ? _tree->view() : 0; }
        const QString & fname() const { return _fname; }
        
        NeuroLib::NeuroNet *neuronet() { return _neuronet; }
        
        NeuroItem *getSelectedItem();
        void deleteSelectedItem();
        void labelSelectedItem(const QString & s);
        
        static LabNetwork *open(QWidget *parent = 0, const QString & fname = QString());
        
    public slots:
        bool save(bool saveAs = false);
        bool close();

        void changed();
        void changed(const QList<QRectF> &);

        void newItem(const QString & typeName);

        void reset();
        void start();
        void stop();
        void step();
    };
    
} // namespace NeuroLab

#endif // LABNETWORK_H
