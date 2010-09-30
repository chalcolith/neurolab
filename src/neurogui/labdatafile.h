#ifndef LABDATAFILE_H
#define LABDATAFILE_H

/*
Neurocognitive Linguistics Lab
Copyright (c) 2010, Gordon Tisher
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

#include <QObject>
#include <QMap>

class QTableWidget;

namespace NeuroGui
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
        explicit LabDataFile(LabNetwork *network, QTableWidget *table, QObject *parent = 0);
        virtual ~LabDataFile();

        bool changed() const { return _changed; }
        void setChanged(bool changed = true) { _changed = changed; }

        QTableWidget *table() { return _table; }

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
