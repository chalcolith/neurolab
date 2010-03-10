#include "labeldialog.h"
#include "ui_labeldialog.h"

namespace NeuroLab
{
    
    LabelDialog::LabelDialog(QWidget *parent) :
            QDialog(parent),
            ui(new Ui::LabelDialog)
    {
        ui->setupUi(this);
        ui->lineEdit->setFocus();
    }
    
    LabelDialog::~LabelDialog()
    {
        delete ui;
    }
    
    const QString LabelDialog::label() const
    {
        return ui->lineEdit->text();
    }
    
    void LabelDialog::setLabel(const QString & s)
    {
        ui->lineEdit->setText(s);
    }
    
    void LabelDialog::changeEvent(QEvent *e)
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
