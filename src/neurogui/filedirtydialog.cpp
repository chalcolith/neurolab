#include "filedirtydialog.h"
#include "ui_filedirtydialog.h"

namespace NeuroLab
{
    
    FileDirtyDialog::FileDirtyDialog(QWidget *parent) :
            QDialog(parent),
            ui(new Ui::filedirtydialog),
            _response(CANCEL)
    {
        ui->setupUi(this);
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
    _response = SAVE;
    this->close();
}

void NeuroLab::FileDirtyDialog::on_discardButton_clicked()
{
    _response = DISCARD;
    this->close();
}

void NeuroLab::FileDirtyDialog::on_cancelButton_clicked()
{
    _response = CANCEL;
    this->close();
}
