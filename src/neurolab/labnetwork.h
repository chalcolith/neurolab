#ifndef LABNETWORK_H
#define LABNETWORK_H

#include <QFile>

#include "labtree.h"
#include "../neurolib/neuronet.h"

namespace NeuroLab
{
    
    /// Contains information for working with a NeuroLib::NeuroNet in the GUI.
    class LabNetwork
            : public QObject
    {
        Q_OBJECT
        
        LabTree *sceneTree;
        NeuroLib::NeuroNet *network;
        
        bool running;
        
        bool dirty, first_change;
        QString fname;
        
    public:
        LabNetwork(QWidget *parent = 0);
        virtual ~LabNetwork();
        
        LabScene *getScene() { return sceneTree ? sceneTree->getScene() : 0; }
        LabView *getView() { return sceneTree ? sceneTree->getView() : 0; }
        const QString & getFName() const { return fname; }
        
        static LabNetwork *open(QWidget *parent = 0, const QString & fname = QString());
        
    public slots:
        void start();
        void stop();
        
        bool save(bool saveAs = false);
        bool close();

        void setMode(const LabScene::Mode &);
        
    private slots:
        void changed(const QList<QRectF> &);
    };
    
} // namespace NeuroLab

#endif // LABNETWORK_H
