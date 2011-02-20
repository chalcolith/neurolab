#ifndef LABNETWORK_H
#define LABNETWORK_H

/*
Neurocognitive Linguistics Lab
Copyright (c) 2010,2011 Gordon Tisher
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

#include "neurogui_global.h"
#include "neuroitem.h"
#include "../neurolib/neuronet.h"

#include <QObject>
#include <QVariant>
#include <QFutureWatcher>
#include <QTime>
#include <QList>
#include <QMap>

class QGraphicsItem;
class QtVariantProperty;

namespace NeuroGui
{

    class LabScene;
    class LabView;
    class LabTree;
    class LabTreeNode;

    /// Contains information for working with a NeuroLib::NeuroNet in the GUI.
    class NEUROGUISHARED_EXPORT LabNetwork
        : public PropertyObject
    {
        Q_OBJECT

        LabTree *_tree;
        NeuroLib::NeuroNet *_neuronet;
        QMap<NeuroItem::IdType, NeuroItem *> _idMap; ///< Maps Ids to pointers.

        bool _running;

        bool _changed, first_change;
        QString _fname;

        Property<LabNetwork, QVariant::String, QString, QString> _filename_property;
        Property<LabNetwork, QVariant::Double, double, NeuroLib::NeuroCell::Value> _decay_property;
        Property<LabNetwork, QVariant::Double, double, NeuroLib::NeuroCell::Value> _link_learn_property;
        Property<LabNetwork, QVariant::Double, double, NeuroLib::NeuroCell::Value> _node_learn_property;
        Property<LabNetwork, QVariant::Double, double, NeuroLib::NeuroCell::Value> _node_forget_property;
        Property<LabNetwork, QVariant::Double, double, NeuroLib::NeuroCell::Value> _learn_time_property;

        quint32 _current_step, _max_steps;
        QFutureWatcher<void> _future_watcher;
        QTime _step_time;

        bool _cancel_step;

    public:
        explicit LabNetwork(QWidget *parent = 0);
        virtual ~LabNetwork();

        /// \return Whether or not the network is currently running.
        bool running() const { return _running; }

        /// \return Whether or not the network has changed since the last save.
        bool changed() const { return _changed; }

        /// Sets the dirty state of the network.
        void setChanged(bool changed = true);

        /// \return The current subnetwork node.
        LabTreeNode *treeNode();

        /// Sets the current subnetwork node.
        void setTreeNode(LabTreeNode *);

        /// \return The currently active graphics scene.
        LabScene *scene() const;

        /// \return The currently active graphics view.
        LabView *view() const;

        /// \return Items in the network, including all sub-networks.
        QList<QGraphicsItem *> items() const;

        /// \return Map of item ids to pointers.
        QMap<NeuroItem::IdType, NeuroItem *> & idMap() { return _idMap; }

        /// \return A pointer to the neural network automaton.
        const NeuroLib::NeuroNet *neuronet() const { return _neuronet; }

        /// \return A pointer to the neural network automaton.
        NeuroLib::NeuroNet *neuronet() { return _neuronet; }

        const QString & fullPath() const { return _fname; }

        QString fname() const;

        virtual QString uiName() const { return tr("Network"); }

        QString subNetworkLabel() const;
        void setSubNetworkLabel(const QString & label);

        NeuroLib::NeuroCell::Value decay() const;
        void setDecay(const NeuroLib::NeuroCell::Value &);

        NeuroLib::NeuroCell::Value linkLearnRate() const;
        void setLinkLearnRate(const NeuroLib::NeuroCell::Value &);

        NeuroLib::NeuroCell::Value nodeLearnRate() const;
        void setNodeLearnRate(const NeuroLib::NeuroCell::Value &);

        NeuroLib::NeuroCell::Value nodeForgetRate() const;
        void setNodeForgetRate(const NeuroLib::NeuroCell::Value &);

        NeuroLib::NeuroCell::Value learnTime() const;
        void setLearnTime(const NeuroLib::NeuroCell::Value &);

        static LabNetwork *open(const QString & fname = QString());

        bool canPaste() const;

        LabTreeNode *findSubNetwork(const quint32 & id);
        LabTreeNode *newSubNetwork();

        void removeWidgetsFrom(QLayout *w);

    public slots:
        bool save(bool saveAs = false);
        bool close();

        void newItem(const QString & typeName);
        void deleteSelected();

        void copySelected();
        void cutSelected();
        void pasteItems();
        void selectAll();

        void reset();
        void start();
        void stop();
        void step(int numSteps);
        void cancel();

        void selectionChanged();
        void changeItemLabel(NeuroItem *, const QString & label);

        void setZoom(int new_zoom);

        void futureFinished();

        void exportPrint();
        void exportSVG();
        void exportPNG();
        void exportPS();
        void exportPDF();

    signals:
        void networkChanged(const QString & title);
        void propertyObjectChanged(QList<PropertyObject *> property_objects);
        void itemLabelChanged(NeuroItem *, const QString & label);
        void itemDeleted(NeuroItem *);
        void actionsEnabled(bool enabled);
        void statusChanged(const QString & status);

        void preStep();
        void postStep();

        void stepIncremented();
        void stepProgressRangeChanged(int minimum, int maximum);
        void stepProgressValueChanged(int value);
    };

} // namespace NeuroGui

#endif // LABNETWORK_H
