#include "filedirtydialog.h"
#include "ui_filedirtydialog.h"

namespace NeuroLab
{

    FileDirtyDialog::FileDirtyDialog(const QString & label)
        : QDialog(0),
        ui(new Ui::filedirtydialog)
    {
        ui->setupUi(this);

        setWindowTitle(tr("Save?"));
        setResult(CANCEL);

        if (!label.isNull() && !label.isEmpty())
            ui->label->setText(label);
    }

    FileDirtyDialog::~FileDirtyDialog()
    {
        delete ui;
    }

    void FileDirtyDialog::changeEvent(QEvent *e)
    {
        QDialog::changeEvent(e);
        switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
        }
    }

}

void NeuroLab::FileDirtyDialog::on_saveButton_clicked()
{
    done(SAVE);
}

void NeuroLab::FileDirtyDialog::on_discardButton_clicked()
{
    done(DISCARD);
}

void NeuroLab::FileDirtyDialog::on_cancelButton_clicked()
{
    done(CANCEL);
}
