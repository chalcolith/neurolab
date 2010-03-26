#ifndef LABELDIALOG_H
#define LABELDIALOG_H

#include "neurogui_global.h"
#include <QDialog>

namespace Ui {
    class LabelDialog;
}

namespace NeuroLab
{

    /// A dialog to edit an item's label (not used since the property editor was implemented).
    class NEUROGUISHARED_EXPORT LabelDialog
        : public QDialog
    {
        Q_OBJECT
    public:
        LabelDialog(QWidget *parent = 0);
        ~LabelDialog();

        const QString label() const;
        void setLabel(const QString & s);

    protected:
        void changeEvent(QEvent *e);

    private:
        Ui::LabelDialog *ui;
    };

}

#endif // LABELDIALOG_H
