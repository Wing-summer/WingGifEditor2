#ifndef EDITORSETTINGDIALOG_H
#define EDITORSETTINGDIALOG_H

#include "dialog/framelessdialogbase.h"
#include <QWidget>

namespace Ui {
class EditorSettingDialog;
}

class EditorSettingDialog : public FramelessDialogBase {
    Q_OBJECT

public:
    explicit EditorSettingDialog(QWidget *parent = nullptr);
    ~EditorSettingDialog();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::EditorSettingDialog *ui;

    void reload();
};

#endif // EDITORSETTINGDIALOG_H
