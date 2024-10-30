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

#include "gifsavedialog.h"

#include <QFormLayout>
#include <QVBoxLayout>

#include <QDialogButtonBox>

GifSaveDialog::GifSaveDialog(QWidget *parent) : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    auto clayout = new QFormLayout(widget);
    clayout->setSpacing(5);

    _loop = new QSpinBox(this);
    _loop->setRange(0, std::numeric_limits<quint16>::max());
    _loop->setValue(0);
    clayout->addRow(tr("Loop"), _loop);

    _comment = new QLineEdit(this);
    _comment->setMinimumWidth(200);
    _comment->setText(QStringLiteral("Made by WingGifEditor2"));
    clayout->addRow(tr("Comment"), _comment);

    layout->addLayout(clayout);
    layout->addSpacing(10);

    layout->addWidget(new QLabel(tr("ZeroIsLoop"), this));
    layout->addSpacing(20);

    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(dbbox, &QDialogButtonBox::accepted, this,
            &GifSaveDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this,
            &GifSaveDialog::on_reject);

    layout->addWidget(dbbox);

    buildUpContent(widget);
    setWindowTitle(tr("GifSave"));
}

GifSaveDialog::~GifSaveDialog() {}

GifSaveResult GifSaveDialog::getResult() const { return _res; }

void GifSaveDialog::on_accept() {
    _res.loop = _loop->value();
    _res.comment = _comment->text();
    done(1);
}

void GifSaveDialog::on_reject() { done(0); }
