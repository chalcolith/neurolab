#ifndef LABNETWORK_H
#define LABNETWORK_H

#include "neurogui_global.h"
#include "propertyobj.h"

#include <QObject>

class QtVariantProperty;

namespace NeuroLib
{
    class NeuroNet;
}

namespace NeuroLab
{

    class LabScene;
    class LabView;
    class LabTree;
    class NeuroItem;

    /// Contains information for working with a NeuroLib::NeuroNet in the GUI.
    class NEUROGUISHARED_EXPORT LabNetwork
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
        LabNetwork(QWidget *parent = 0);
        virtual ~LabNetwork();

        /// \return Whether or not the network has changed since the last save.
        bool dirty() const { return _dirty; }

        /// Sets the dirty state of the network.
        void setDirty(bool dirty = true);

        /// \return The currently active graphics scene.
        LabScene *scene();

        /// \return The currently active graphics view.
        LabView *view();

        /// \return The name of the file from which the network was loaded.
        const QString & fname() const { return _fname; }

        /// \return A pointer to the neural network automaton.
        const NeuroLib::NeuroNet *neuronet() const { return _neuronet; }

        /// \return A pointer to the neural network automaton.
        NeuroLib::NeuroNet *neuronet() { return _neuronet; }

        virtual void buildProperties(QtVariantPropertyManager *manager, QtProperty *parentItem);
        void updateProperties();

        static LabNetwork *open(QWidget *parent = 0, const QString & fname = QString());

    public slots:
        bool save(bool saveAs = false);
        bool close();

        void newItem(const QString & typeName);
        void deleteSelected();

        void reset();
        void start();
        void stop();
        void step();

        void selectionChanged();
        void propertyValueChanged(QtProperty *property, const QVariant & value);
    };

} // namespace NeuroLab

#endif // LABNETWORK_H
