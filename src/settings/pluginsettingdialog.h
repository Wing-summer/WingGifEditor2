#ifndef PLUGINSETTINGDIALOG_H
#define PLUGINSETTINGDIALOG_H

#include "dialog/framelessdialogbase.h"
#include <QWidget>

namespace Ui {
class PluginSettingDialog;
}

class PluginSettingDialog : public FramelessDialogBase {
    Q_OBJECT

public:
    explicit PluginSettingDialog(QWidget *parent = nullptr);
    ~PluginSettingDialog();

private:
    Ui::PluginSettingDialog *ui;

private slots:
    void on_plglist_itemSelectionChanged();
};

#endif // PLUGINSETTINGDIALOG_H
