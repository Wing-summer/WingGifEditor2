#ifndef REDUCEFRAMEDIALOG_H
#define REDUCEFRAMEDIALOG_H

#include "framelessdialogbase.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QMainWindow>
#include <QSpinBox>

struct ReduceResult {
    int start = -1;
    int end = -1;
    int stepcount = 1;
};

class ReduceFrameDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    ReduceFrameDialog(int max, QWidget *parent = nullptr);
    ReduceResult getResult();

private:
    void on_accept();
    void on_reject();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QSpinBox *sbcount;
    QSpinBox *sbfrom, *sbto;

    ReduceResult res;
};

#endif // REDUCEFRAMEDIALOG_H
