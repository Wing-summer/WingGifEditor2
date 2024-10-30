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

#include "scalegifdialog.h"

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>

ScaleGIFDialog::ScaleGIFDialog(QSize size, QWidget *parent)
    : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);
    layout->addSpacing(3);

    auto group = new QButtonGroup(this);
    group->setExclusive(true);

    auto btnBox = new QWidget(this);
    auto buttonLayout = new QHBoxLayout(btnBox);
    buttonLayout->setSpacing(0);

    int id = 0;

    auto b = new QPushButton(QStringLiteral("Unlock"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [=](bool b) {
        if (b) {
            _lockscale = false;
        }
    });
    group->addButton(b, id++);
    buttonLayout->addWidget(b);

    b = new QPushButton(QStringLiteral("Lock"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [=](bool b) {
        if (b) {
            _lockscale = true;
        }
    });
    group->addButton(b, id++);
    buttonLayout->addWidget(b);
    b->setChecked(true);
    layout->addWidget(btnBox);

    layout->addSpacing(10);
    layout->addWidget(new QLabel(tr("Width"), this));
    layout->addSpacing(3);

    _oldwidth = size.width();

    sbwidth = new QSpinBox(this);
    sbwidth->setMinimum(1);
    sbwidth->setMaximum(INT_MAX);
    sbwidth->setValue(_oldwidth);
    sbwidth->setSuffix(" px");

    connect(sbwidth, QOverload<int>::of(&QSpinBox::valueChanged), this, [=] {
        if (_lockscale)
            sbheight->setValue(sbwidth->value() * _oldheight / _oldwidth);
    });

    layout->addWidget(sbwidth);
    layout->addSpacing(5);
    layout->addWidget(new QLabel(tr("Height"), this));
    layout->addSpacing(3);

    _oldheight = size.height();

    sbheight = new QSpinBox(this);
    sbheight->setMinimum(1);
    sbheight->setMaximum(INT_MAX);
    sbheight->setValue(_oldheight);
    sbheight->setSuffix(" px");

    connect(sbheight, QOverload<int>::of(&QSpinBox::valueChanged), this, [=] {
        if (_lockscale)
            sbwidth->setValue(sbheight->value() * _oldwidth / _oldheight);
    });

    layout->addWidget(sbheight);
    layout->addSpacing(20);
    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(dbbox, &QDialogButtonBox::accepted, this,
            &ScaleGIFDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this,
            &ScaleGIFDialog::on_reject);

    // dont use it
    // auto key = QKeySequence(Qt::Key_Return);
    // auto s = new QShortcut(key, this);
    // connect(s, &QShortcut::activated, this, &ScaleGIFDialog::on_accept);

    layout->addWidget(dbbox);
    buildUpContent(widget);

    setWindowTitle(tr("ScaleGIFDialog"));
}

ScaleResult ScaleGIFDialog::getResult() { return res; }

void ScaleGIFDialog::on_accept() {
    res.width = sbwidth->value();
    res.height = sbheight->value();
    done(1);
}

void ScaleGIFDialog::on_reject() { done(0); }

void ScaleGIFDialog::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event);
    done(0);
}
