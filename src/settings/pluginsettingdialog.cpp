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

#include "pluginsettingdialog.h"
#include "plugin/pluginsystem.h"
#include "ui_pluginsettingdialog.h"
#include "utilities.h"

PluginSettingDialog::PluginSettingDialog(QWidget *parent)
    : FramelessDialogBase(parent), ui(new Ui::PluginSettingDialog) {
    auto widget = new QWidget(this);
    widget->setMinimumSize(400, 400);
    ui->setupUi(widget);
    buildUpContent(widget);

    setWindowTitle(tr("Plugins"));

    auto &plgsys = PluginSystem::instance();
    auto pico = ICONRES("plugin");
    ui->plglist->clear();
    for (auto &p : plgsys.plugins()) {
        ui->plglist->addItem(new QListWidgetItem(pico, p->pluginName()));
    }

    ui->txtc->clear();
}

PluginSettingDialog::~PluginSettingDialog() { delete ui; }

void PluginSettingDialog::on_plglist_itemSelectionChanged() {
    auto &plgsys = PluginSystem::instance();

    auto plg = plgsys.plugin(ui->plglist->currentRow());

    ui->txtc->append(tr("pluginName") + " : " + plg->pluginName());
    ui->txtc->append(tr("pluginAuthor") + " : " + plg->pluginAuthor());
    ui->txtc->append(tr("pluginVersion") + " : " +
                     QString::number(plg->pluginVersion()));
    ui->txtc->append(tr("pluginComment") + " : " + plg->pluginComment());
}
