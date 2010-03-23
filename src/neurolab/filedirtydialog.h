#ifndef FILEDIRTYDIALOG_H
#define FILEDIRTYDIALOG_H

#include <QDialog>

namespace Ui {
    class filedirtydialog;
}

namespace NeuroLab
{

    /// A dialog to ask the user whether or not to save a dirty file.
    class FileDirtyDialog : public QDialog {
        Q_OBJECT
    public:
        FileDirtyDialog(QWidget *parent = 0);
        ~FileDirtyDialog();

        enum Response
        {
            SAVE,
            DISCARD,
            CANCEL
        };

        Response response() const { return _response; }

    protected:
        void changeEvent(QEvent *e);

    private:
        Ui::filedirtydialog *ui;

        Response _response;

    private slots:
        void on_cancelButton_clicked();
        void on_discardButton_clicked();
        void on_saveButton_clicked();
    };

}

#endif // FILEDIRTYDIALOG_H
