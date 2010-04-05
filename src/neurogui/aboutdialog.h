#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
    class AboutDialog;
}

namespace NeuroLab
{

    class AboutDialog
        : public QDialog
    {
        Q_OBJECT
    public:
        AboutDialog(QWidget *parent = 0);
        ~AboutDialog();

        void setLabel(const QString &);

    protected:
        void changeEvent(QEvent *e);

    private:
        Ui::AboutDialog *ui;
    };

}

#endif // ABOUTDIALOG_H
