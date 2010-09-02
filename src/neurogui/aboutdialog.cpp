#include "aboutdialog.h"
#include "ui_aboutdialog.h"

namespace NeuroGui
{

    AboutDialog::AboutDialog(QWidget *parent)
        : QDialog(parent),
        ui(new Ui::AboutDialog)
    {
        ui->setupUi(this);
    }

    AboutDialog::~AboutDialog()
    {
        delete ui;
    }

    void AboutDialog::setLabel(const QString & str)
    {
        ui->label->setText(str);
    }

    void AboutDialog::changeEvent(QEvent *e)
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
