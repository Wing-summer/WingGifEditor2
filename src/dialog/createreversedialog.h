#ifndef CREATEREVERSEDIALOG_H
#define CREATEREVERSEDIALOG_H

#include "framelessdialogbase.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QMainWindow>
#include <QSpinBox>

struct ReverseResult {
    int start = -1;
    int end = -1;
};

class CreateReverseDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    CreateReverseDialog(int max, QWidget *parent = nullptr);
    ReverseResult getResult();

private:
    void on_accept();
    void on_reject();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QSpinBox *sbfrom, *sbto;
    ReverseResult res;
};

#endif // CREATEREVERSEDIALOG_H
