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

#include "aboutsoftwaredialog.h"
#include "ui_aboutsoftwaredialog.h"

#include "utilities.h"

AboutSoftwareDialog::AboutSoftwareDialog(QWidget *parent)
    : QWidget(parent), ui(new Ui::AboutSoftwareDialog) {
    ui->setupUi(this);

    _dialog = new FramelessDialogBase(parent);
    _dialog->buildUpContent(this);
    _dialog->setWindowTitle(this->windowTitle());

    ui->lblVersion->setText(WINGGIF_VERSION);

    _dialog->setWindowTitle(tr("About"));
    _dialog->setWindowIcon(ICONRES(QStringLiteral("soft")));

    Utilities::moveToCenter(this);
}

AboutSoftwareDialog::~AboutSoftwareDialog() {
    delete ui;
    _dialog->deleteLater();
}

int AboutSoftwareDialog::exec() { return _dialog->exec(); }
