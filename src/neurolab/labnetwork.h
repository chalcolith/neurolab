#ifndef LABNETWORK_H
#define LABNETWORK_H

#include "labtree.h"
#include "../neurolib/neuronet.h"
#include "propertyobj.h"

#include <QFile>

class QtVariantProperty;

namespace NeuroLab
{

    class NeuroItem;

    /// Contains information for working with a NeuroLib::NeuroNet in the GUI.
    class LabNetwork
        : public QObject, public PropertyObject
    {
        Q_OBJECT

        LabTree *_tree;
        NeuroLib::NeuroNet *_neuronet;

        bool running;

        bool _dirty, first_change;
        QString _fname;

        QtVariantProperty *filename_property;
        QtVariantProperty *decay_property;
        QtVariantProperty *learn_property;
        QtVariantProperty *learn_time_property;

    public:
        LabNetwork(QWidget *_parent = 0);
        virtual ~LabNetwork();

        bool dirty() const { return _dirty; }
        void setDirty(bool dirty = true);

        LabScene *scene() { return _tree ? _tree->scene() : 0; }
        LabView *view() { return _tree ? _tree->view() : 0; }
        const QString & fname() const { return _fname; }

        NeuroLib::NeuroNet *neuronet() { return _neuronet; }

        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);
        void updateProperties();

        static LabNetwork *open(QWidget *parent = 0, const QString & fname = QString());

    public slots:
        bool save(bool saveAs = false);
        bool close();

        void newItem(const QString & typeName);
        void deleteSelected();
        void toggleActivated();
        void toggleFrozen();

        void reset();
        void start();
        void stop();
        void step();

        void selectionChanged();
        void propertyValueChanged(QtProperty *property, const QVariant & value);
    };

} // namespace NeuroLab

#endif // LABNETWORK_H
