#ifndef SCALEGIFDIALOG_H
#define SCALEGIFDIALOG_H

#include "framelessdialogbase.h"

#include <QDialog>
#include <QMainWindow>
#include <QObject>
#include <QSpinBox>

struct ScaleResult {
    int width;
    int height;
};

class ScaleGIFDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    ScaleGIFDialog(QSize size, QWidget *parent = nullptr);
    ScaleResult getResult();

private:
    void on_accept();
    void on_reject();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QSpinBox *sbwidth;
    QSpinBox *sbheight;
    bool _lockscale = true;
    int _oldwidth = 0, _oldheight = 0;
    ScaleResult res;
};

#endif // SCALEGIFDIALOG_H
