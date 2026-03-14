/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include "exportdialog.h"

#include "class/wingfiledialog.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QShortcut>
#include <QStandardPaths>
#include <QVBoxLayout>

ExportDialog::ExportDialog(bool isGlobal, QWidget *parent)
    : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);
    layout->addWidget(new QLabel(tr("ChooseFolder"), this));
    layout->addSpacing(3);

    auto hlayout = new QHBoxLayout;
    folder = new QLineEdit(this);
    folder->setMinimumWidth(200);
    folder->setReadOnly(true);
    QDir dir(
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    auto subpath = QLatin1String("WingGifEditor");
    dir.mkdir(subpath);
    connect(folder, &QLineEdit::textChanged, folder, &QLineEdit::setToolTip);
    folder->setText(dir.absoluteFilePath(subpath));
    hlayout->addWidget(folder);

    auto btn = new QPushButton(this);
    btn->setText(QLatin1String("..."));
    btn->setMinimumWidth(25);
    btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    connect(btn, &QPushButton::clicked, this, [this]() {
        auto path = WingFileDialog::getExistingDirectory();
        if (!path.isEmpty()) {
            folder->setText(path);
        }
    });
    hlayout->addWidget(btn);
    layout->addLayout(hlayout);

    auto group = new QButtonGroup(this);
    group->setExclusive(true);

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(0);

    int id = 0;

    auto b = new QPushButton(QStringLiteral("PNG"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [=](bool b) {
        if (b) {
            res.type = ExportImageType::PNG;
        }
    });
    group->addButton(b, id++);
    buttonLayout->addWidget(b);

    b = new QPushButton(QStringLiteral("JPG"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [=](bool b) {
        if (b) {
            res.type = ExportImageType::JPG;
        }
    });
    group->addButton(b, id++);
    buttonLayout->addWidget(b);

    b = new QPushButton(QStringLiteral("TIFF"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [=](bool b) {
        if (b) {
            res.type = ExportImageType::TIFF;
        }
    });
    group->addButton(b, id++);
    buttonLayout->addWidget(b);

    b = new QPushButton(QStringLiteral("WEBP"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [=](bool b) {
        if (b) {
            res.type = ExportImageType::WEBP;
        }
    });
    group->addButton(b, id++);
    buttonLayout->addWidget(b);

    group->button(0)->setChecked(true);
    layout->addLayout(buttonLayout);
    layout->addSpacing(10);

    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(dbbox, &QDialogButtonBox::accepted, this, &ExportDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this, &ExportDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &ExportDialog::on_accept);
    layout->addWidget(dbbox);

    buildUpContent(widget);
    setWindowTitle(tr("Export") +
                   (isGlobal ? tr("(Global)") : tr("(Selected)")));
}

ExportResult ExportDialog::getResult() { return res; }

void ExportDialog::on_accept() {
    res.path = folder->text();
    done(1);
}

void ExportDialog::on_reject() { done(0); }

void ExportDialog::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event);
    done(0);
}
