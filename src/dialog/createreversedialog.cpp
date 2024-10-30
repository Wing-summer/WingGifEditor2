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

#include "createreversedialog.h"
#include <QShortcut>
#include <QVBoxLayout>

CreateReverseDialog::CreateReverseDialog(int max, QWidget *parent)
    : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);
    layout->addWidget(new QLabel(tr("Start"), this));
    layout->addSpacing(3);
    sbfrom = new QSpinBox(this);
    sbfrom->setMinimum(1);
    sbfrom->setValue(0);
    sbfrom->setMaximum(max - 2);
    layout->addWidget(sbfrom);
    layout->addSpacing(5);
    layout->addWidget(new QLabel(tr("End"), this));
    sbto = new QSpinBox(this);
    sbto->setMinimum(2);
    sbto->setValue(max);
    sbto->setMaximum(max);
    layout->addWidget(sbto);
    layout->addSpacing(20);
    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(dbbox, &QDialogButtonBox::accepted, this,
            &CreateReverseDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this,
            &CreateReverseDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &CreateReverseDialog::on_accept);
    layout->addWidget(dbbox);

    buildUpContent(widget);
    setWindowTitle(tr("CreateReverseDialog"));
}

ReverseResult CreateReverseDialog::getResult() { return res; }

void CreateReverseDialog::on_accept() {
    res.start = sbfrom->value() - 1;
    res.end = sbto->value() - 1;
    done(1);
}

void CreateReverseDialog::on_reject() { done(0); }

void CreateReverseDialog::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event);
    done(0);
}
