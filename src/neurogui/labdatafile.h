#ifndef LABDATAFILE_H
#define LABDATAFILE_H

#include "neurogui_global.h"

#include <QObject>
#include <QMap>

class QTableWidget;

namespace NeuroLab
{

    class NeuroItem;
    class LabNetwork;

    class NEUROGUISHARED_EXPORT LabDataFile
        : public QObject
    {
        Q_OBJECT

        bool _changed;

        LabNetwork *_network;
        QTableWidget *_table;

        QMap<NeuroItem *, int> _itemColumnIndices;

    public:
        LabDataFile(LabNetwork *network, QTableWidget *table, QObject *parent = 0);
        virtual ~LabDataFile();

        bool changed() const { return _changed; }
        void setChanged(bool changed = true) { _changed = changed; }

    public slots:
        void changeItemLabel(NeuroItem *item, const QString & label);
        void deleteItem(NeuroItem *item);

        void reset();
        void valuesChanged();
        void incrementStep();

        void saveAs();
    };

}

#endif // LABDATAFILE_H
