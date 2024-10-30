/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

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
