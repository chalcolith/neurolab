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

#include "labdatafile.h"
#include "labnetwork.h"
#include "neuronetworkitem.h"
#include "subnetwork/subnetworkitem.h"

#include <QTableWidget>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QVariant>

namespace NeuroGui
{

    LabDataFile::LabDataFile(LabNetwork *network, QTableWidget *table, QObject *parent)
        : QObject(parent), _changed(false), _network(network), _table(table)
    {
        if (_network)
        {
            reset();
            connect(_network, SIGNAL(itemLabelChanged(NeuroItem*,QString)), this, SLOT(changeItemLabel(NeuroItem*,QString)), Qt::UniqueConnection);
            connect(_network, SIGNAL(itemDeleted(NeuroItem*)), this, SLOT(deleteItem(NeuroItem*)), Qt::UniqueConnection);
            connect(_network, SIGNAL(valuesChanged()), this, SLOT(valuesChanged()));
            connect(_network, SIGNAL(stepIncremented()), this, SLOT(incrementStep()));
        }
    }

    LabDataFile::~LabDataFile()
    {
    }

    void LabDataFile::changeItemLabel(NeuroItem *item, const QString & label)
    {
        Q_ASSERT(item != 0);
        Q_ASSERT(_network != 0);
        Q_ASSERT(_table != 0);

        // only record network items
        if (dynamic_cast<NeuroNetworkItem *>(item) == 0)
            return;

        if (dynamic_cast<SubNetworkItem *>(item) != 0)
            return;

        // get column index
        int columnIndex = -1;

        if (_itemColumnIndices.contains(item))
        {
            columnIndex = _itemColumnIndices[item];

            // remove column if the label is now empty
            if (label.isNull() || label.isEmpty())
            {
                setChanged();
                _table->removeColumn(columnIndex);
                columnIndex = -1;
            }
        }
        else if (!label.isNull() && !label.isEmpty())
        {
            columnIndex = _table->columnCount();
            _itemColumnIndices[item] = columnIndex;
        }

        if (columnIndex == -1)
            return; // don't bother with empty labels

        // insert column if not present
        if (columnIndex >= _table->columnCount())
        {
            _table->setColumnCount(columnIndex + 1);
        }

        // create new header item if necessary
        QTableWidgetItem *header = _table->horizontalHeaderItem(columnIndex);

        if (!header)
        {
            header = new QTableWidgetItem();
            _table->setHorizontalHeaderItem(columnIndex, header);
        }

        // set label
        header->setText(label);
        valuesChanged();
    }

    void LabDataFile::deleteItem(NeuroItem *item)
    {
        Q_ASSERT(item != 0);
        Q_ASSERT(_table != 0);

        if (_itemColumnIndices.contains(item))
        {
            setChanged();
            _table->removeColumn(_itemColumnIndices[item]);
            _itemColumnIndices.remove(item);
        }
    }

    void LabDataFile::reset()
    {
        Q_ASSERT(_network);
        Q_ASSERT(_table);

        _table->clear();
        _table->setSortingEnabled(false);

        _itemColumnIndices.clear();

        foreach (QGraphicsItem *gi, _network->items())
        {
            NeuroItem *item = dynamic_cast<NeuroItem *>(gi);

            if (item && !item->label().isEmpty() && !item->label().isNull())
            {
                changeItemLabel(item, item->label());
            }
        }

        incrementStep();
        setChanged(false);
    }

    static void disableEdit(QTableWidgetItem *wi)
    {
        Qt::ItemFlags f = wi->flags();
        f &= !Qt::ItemIsEditable;
        wi->setFlags(f);
    }

    void LabDataFile::valuesChanged()
    {
        int row = _table->rowCount() - 1;
        if (row >= 0)
        {
            foreach (NeuroItem *item, _itemColumnIndices.keys())
            {
                int columnIndex = _itemColumnIndices[item];

                QTableWidgetItem *wi = new QTableWidgetItem(item->dataValue());
                disableEdit(wi);
                _table->setItem(row, columnIndex, wi);
            }

            setChanged();
        }
    }

    void LabDataFile::incrementStep()
    {
        if (_itemColumnIndices.size() == 0)
            return;

        int newRow = _table->rowCount();
        _table->setRowCount(newRow + 1);

        valuesChanged();

        if (_table->verticalHeaderItem(newRow))
            _table->verticalHeaderItem(newRow)->setText(QString::number(newRow));
    }

    static QString csv(const QString & s)
    {
        QString result = s;
        result.replace('\"', "\"\"");
        if (result.contains('\"') || result.contains(',') || result.isNull() || result.isEmpty())
            result = QString("\"%1\"").arg(result);
        return result;
    }

    void LabDataFile::saveAs()
    {
        // write data header
        QString fname = QFileDialog::getSaveFileName(0, tr("Save Data File"), ".", tr("Comma-Separated Value Files (*.csv);;All Files(*)"));
        if (!fname.isNull() || !fname.isEmpty())
        {
            QFile file(fname);

            if (file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream ts(&file);

                // write date & time
                QDateTime utc = QDateTime::currentDateTime().toUTC();
                ts << "# NeuroLab Network Simulation Results: " << utc.toString(Qt::ISODate) << " UTC" << '\n';
                ts << "# \n";

                // write network properties
                foreach (const PropertyObject::PropertyBase *p, _network->properties())
                {
                    ts << "# " << p->name() << ": " << p->value().toString() << '\n';
                }

                ts << '\n';

                // write column names
                for (int col = 0; col < _table->columnCount(); ++col)
                {
                    if (col > 0)
                        ts << ", ";
                    if (_table->horizontalHeaderItem(col))
                        ts << csv(_table->horizontalHeaderItem(col)->text());
                }
                if (_table->columnCount() > 0)
                    ts << '\n';

                // write data
                for (int row = 0; row < _table->rowCount(); ++row)
                {
                    for (int col = 0; col < _table->columnCount(); ++col)
                    {
                        if (col > 0)
                            ts << ", ";
                        if (_table->item(row, col))
                            ts << csv(_table->item(row, col)->text());
                    }
                    ts << '\n';
                }

                ts.flush();
                setChanged(false);
            }
            else
            {
                throw Common::IOError(tr("Unable to write data file."));
            }
        }
    }

} // namespace NeuroGui
