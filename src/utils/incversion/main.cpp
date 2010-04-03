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

#include <QtCore/qglobal.h>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QList>
#include <QRegExp>

static QTextStream cout(stdout);
static QTextStream cerr(stderr);

static int num_printed = 0;

static QString increment(const QString & line, bool bump)
{
    QString result = line;

    QRegExp re("(\\d+)\\.(\\d+)\\.(\\d+)");
    if (re.indexIn(line) >= 0)
    {
        QString cap = re.cap(3);
        int lastNum = cap.toInt();
        QString newNum = QString::number(lastNum + (bump ? 1 : 0));
        result.replace(re.pos(3), cap.length(), newNum);

        if (num_printed++)
            cout << '\n';
        cout << re.cap(1) << "." << re.cap(2) << "." << newNum;
        cout.flush();
    }

    return result;
}

int main(int argc, char *argv[])
{
    bool bump = true;

    for (int i = 1; i < argc; ++i)
    {
        QList<QString> lines;
        QString fname(argv[i]);

        if (fname == "-nobump")
        {
            bump = false;
            continue;
        }

        // read
        QFile file(fname);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream fs(&file);
            while (!fs.atEnd())
            {
                QString line = increment(fs.readLine(), bump);
                lines.append(line);
            }
        }
        else
        {
            cerr << "incversion: unable to open '" << fname << "' for input!" << endl;
            return 1;
        }
        file.close();

        if (!bump)
            continue;

        // write
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream fs(&file);
            for (int i = 0; i < lines.size(); ++i)
            {
                fs << lines[i];
                if (i+1 < lines.size())
                    fs << '\n';
            }
            fs.flush();
        }
        else
        {
            cerr << "incversion: unable to open '" << fname << "' for output!" << endl;
            return 1;
        }
        file.close();
    }

    return 0;
}
