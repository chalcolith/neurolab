#ifndef LABNETWORK_H
#define LABNETWORK_H

#include "neurogui_global.h"
#include "propertyobj.h"
#include "../neurolib/neurocell.h"

#include <QObject>
#include <QVariant>

class QtVariantProperty;

namespace NeuroLab
{

    class LabScene;
    class LabView;
    class LabTree;
    class NeuroItem;

    /// Contains information for working with a NeuroLib::NeuroNet in the GUI.
    class NEUROGUISHARED_EXPORT LabNetwork
        : public PropertyObject
    {
        Q_OBJECT

        LabTree *_tree;
        NeuroLib::NeuroNet *_neuronet;

        bool running;

        bool _changed, first_change;
        QString _fname;

        Property<LabNetwork, QVariant::String, QString, QString> _filename_property;
        Property<LabNetwork, QVariant::Double, double, NeuroLib::NeuroCell::NeuroValue> _decay_property;
        Property<LabNetwork, QVariant::Double, double, NeuroLib::NeuroCell::NeuroValue> _learn_rate_property;
        Property<LabNetwork, QVariant::Double, double, NeuroLib::NeuroCell::NeuroValue> _learn_time_property;

    public:
        LabNetwork(QWidget *parent = 0);
        virtual ~LabNetwork();

        /// \return Whether or not the network has changed since the last save.
        bool changed() const { return _changed; }

        /// Sets the dirty state of the network.
        void setChanged(bool changed = true);

        /// \return The currently active graphics scene.
        LabScene *scene();

        /// \return The currently active graphics view.
        LabView *view();

        /// \return A pointer to the neural network automaton.
        const NeuroLib::NeuroNet *neuronet() const { return _neuronet; }

        /// \return A pointer to the neural network automaton.
        NeuroLib::NeuroNet *neuronet() { return _neuronet; }

        const QString & fullPath() const { return _fname; }

        QString fname() const;

        virtual QString uiName() const { return tr("Network"); }

        NeuroLib::NeuroCell::NeuroValue decay() const;
        void setDecay(const NeuroLib::NeuroCell::NeuroValue &);

        NeuroLib::NeuroCell::NeuroValue learnRate() const;
        void setLearnRate(const NeuroLib::NeuroCell::NeuroValue &);

        NeuroLib::NeuroCell::NeuroValue learnTime() const;
        void setLearnTime(const NeuroLib::NeuroCell::NeuroValue &);

        static LabNetwork *open(QWidget *parent = 0, const QString & fname = QString());

    public slots:
        bool save(bool saveAs = false);
        bool close();

        void newItem(const QString & typeName);
        void deleteSelected();

        void reset();
        void start();
        void stop();
        void step(int numSteps);

        void selectionChanged();
    };

} // namespace NeuroLab

#endif // LABNETWORK_H
