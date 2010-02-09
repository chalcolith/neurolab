#ifndef LABELDIALOG_H
#define LABELDIALOG_H

#include <QDialog>

namespace Ui {
    class LabelDialog;
}

class LabelDialog : public QDialog {
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

#endif // LABELDIALOG_H
