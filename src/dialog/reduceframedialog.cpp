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

#include "reduceframedialog.h"
#include <QShortcut>

#include <QVBoxLayout>

ReduceFrameDialog::ReduceFrameDialog(int max, QWidget *parent)
    : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);
    layout->addWidget(new QLabel(tr("ReduceCount"), this));
    layout->addSpacing(3);

    sbcount = new QSpinBox(this);
    sbcount->setMinimum(1);
    sbcount->setMaximum(max - 2);
    sbcount->setValue(1);
    layout->addWidget(sbcount);
    layout->addSpacing(5);
    layout->addWidget(new QLabel(tr("ReduceStart"), this));
    layout->addSpacing(3);

    sbfrom = new QSpinBox(this);
    sbfrom->setMinimum(1);
    sbfrom->setValue(1);
    sbfrom->setMaximum(max);
    layout->addWidget(sbfrom);
    layout->addSpacing(5);
    layout->addWidget(new QLabel(tr("ReduceEnd"), this));
    sbto = new QSpinBox(this);
    sbto->setMinimum(2);
    sbto->setMaximum(max);
    sbto->setValue(max);
    layout->addWidget(sbto);
    layout->addSpacing(20);
    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(dbbox, &QDialogButtonBox::accepted, this,
            &ReduceFrameDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this,
            &ReduceFrameDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &ReduceFrameDialog::on_accept);
    layout->addWidget(dbbox);

    buildUpContent(widget);
    setWindowTitle(tr("ReduceFrameDialog"));
}

ReduceResult ReduceFrameDialog::getResult() { return res; }

void ReduceFrameDialog::on_accept() {
    res.start = sbfrom->value() - 1;
    res.end = sbto->value() - 1;
    res.stepcount = sbcount->value();
    done(1);
}

void ReduceFrameDialog::on_reject() { done(0); }

void ReduceFrameDialog::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event);
    done(0);
}
