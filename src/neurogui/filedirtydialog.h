#ifndef FILEDIRTYDIALOG_H
#define FILEDIRTYDIALOG_H

#include "neurogui_global.h"
#include <QDialog>

namespace Ui {
    class filedirtydialog;
}

namespace NeuroLab
{

    /// A dialog to ask the user whether or not to save a dirty file.
    class NEUROGUISHARED_EXPORT FileDirtyDialog
        : public QDialog
    {
        Q_OBJECT
    public:
        FileDirtyDialog(const QString & label = QString());
        ~FileDirtyDialog();

        enum Response
        {
            CANCEL = QDialog::Rejected,
            SAVE = QDialog::Accepted,
            DISCARD
        };

    protected:
        void changeEvent(QEvent *e);

    private:
        Ui::filedirtydialog *ui;

    private slots:
        void on_cancelButton_clicked();
        void on_discardButton_clicked();
        void on_saveButton_clicked();
    };

}

#endif // FILEDIRTYDIALOG_H
