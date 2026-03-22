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

#include "newdialog.h"
#include "class/wingfiledialog.h"
#include "control/toast.h"
#include "define.h"
#include "utilities.h"

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QShortcut>
#include <QWidget>

#include <QImageReader>

NewDialog::NewDialog(QWidget *parent) : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    static auto nopic =
        QPixmap(QLatin1String(":/com.wingsummer.winggif/images/nopic.jpeg"));

    _watcher = new QFileSystemWatcher(this);
    connect(_watcher, &QFileSystemWatcher::fileChanged, this,
            [this](const QString &filename) {
                auto idx = filenames.indexOf(filename);
                if (idx >= 0) {
                    auto item = imgslist->takeItem(idx);
                    if (item) {
                        delete item;
                    }
                    filenames.remove(idx);
                    _watcher->removePath(filename);
                }
            });

    auto btnBox = new QWidget(this);
    auto buttonLayout = new QHBoxLayout(btnBox);
    buttonLayout->setSpacing(0);

    auto b = new QPushButton(tr("Add"), this);
    connect(b, &QPushButton::clicked, this, [=] {
        auto files = WingFileDialog::getOpenFileNames(
            this, qAppName(), {},
            tr("Images (*.png *.jpg *.jpeg *.bmp *.tiff *.webp)"));
        for (auto &item : files) {
            QImage img;
            if (img.load(item)) {
                filenames << item;
                _watcher->addPath(item);
                QFileInfo finfo(item);
                auto p = new QListWidgetItem(ICONRES("picture"),
                                             finfo.fileName(), imgslist);
                p->setToolTip(item);
                p->setData(Qt::UserRole, item);
            }
        }
        if (imgslist->currentRow() < 0) {
            imgslist->setCurrentRow(0);
        }
    });
    buttonLayout->addWidget(b);

    b = new QPushButton(tr("Remove"), this);
    connect(b, &QPushButton::clicked, this, [=] {
        auto indices = imgslist->selectionModel()->selectedRows();
        QList<int> ins;
        for (auto i : std::as_const(indices)) {
            ins.append(i.row());
        }
        std::sort(ins.begin(), ins.end(), std::greater<int>());
        for (auto i : ins) {
            auto p = imgslist->item(i);
            imgslist->removeItemWidget(p);
            _watcher->removePath(filenames.at(i));
            delete p;
            filenames.removeAt(i);
        }
    });
    buttonLayout->addWidget(b);

    b = new QPushButton(tr("Clear"), this);
    connect(b, &QPushButton::clicked, this, [=] {
        imgslist->clear();
        _watcher->removePaths(filenames);
        filenames.clear();
    });
    buttonLayout->addWidget(b);

    b = new QPushButton(tr("MoveUp"), this);
    connect(b, &QPushButton::clicked, this, [=] {
        auto si = imgslist->selectionModel()->selectedRows();
        auto len = si.count();
        QVector<int> indices(len);
        auto i = 0;
        for (auto &item : si) {
            indices[i] = item.row();
        }
        std::sort(indices.begin(), indices.end());
        if (indices[0] == 0) {
            for (i = 1; i < len; i++) {
                if (indices[i] != i)
                    break;
            }
        }

        for (; i < len; i++) {
            auto p = imgslist->takeItem(i);
            imgslist->insertItem(i - 1, p);
            filenames.move(i, i - 1);
        }
    });
    buttonLayout->addWidget(b);

    b = new QPushButton(tr("MoveDown"), this);
    connect(b, &QPushButton::clicked, this, [=] {
        auto si = imgslist->selectionModel()->selectedRows();
        auto len = si.count();
        QVector<int> indices(len);
        auto i = 0;
        for (auto &item : si) {
            indices[i] = item.row();
        }
        std::sort(indices.begin(), indices.end(), std::greater<int>());
        if (indices[0] == len - 1) {
            for (i = 1; i < len; i++) {
                if (indices[i] != len - 1 - i)
                    break;
            }
        }
        for (; i < len; i++) {
            auto p = imgslist->takeItem(i);
            imgslist->insertItem(i + 1, p);
            filenames.move(i, i + 1);
        }
    });
    buttonLayout->addWidget(b);

    layout->addWidget(btnBox);
    layout->addSpacing(10);

    auto w = new QWidget(this);
    auto hbox = new QHBoxLayout(w);
    imgslist = new QListWidget(this);
    imgslist->setMinimumWidth(280);
    hbox->addWidget(imgslist);
    auto vbox = new QVBoxLayout(w);
    hbox->addLayout(vbox);
    imgview = new AspectRatioPixmapLabel(this);
    imgview->setFixedSize(220, 220);
    imgview->setScaledContents(true);
    imgview->setPixmap(nopic);
    vbox->addWidget(imgview, 0, Qt::AlignCenter);
    vbox->addSpacing(5);

    auto slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel(tr("Size"), this));
    sbimgWidth = new QSpinBox(this);
    sbimgWidth->setRange(10, FRAME_MAX_SIZE);
    sbimgWidth->setValue(100);
    sbimgWidth->setEnabled(false);
    slayout->addWidget(sbimgWidth, 1);
    slayout->addWidget(new QLabel(QLatin1String("x"), this));
    sbimgHeight = new QSpinBox(this);
    sbimgHeight->setRange(10, FRAME_MAX_SIZE);
    sbimgHeight->setValue(100);
    sbimgHeight->setEnabled(false);
    slayout->addWidget(sbimgHeight, 1);
    slayout->addWidget(new QLabel(QLatin1String("px"), this));
    vbox->addLayout(slayout);
    auto btnReset = new QPushButton(this);
    btnReset->setEnabled(false);
    btnReset->setText(tr("DefaultSize"));
    connect(btnReset, &QPushButton::clicked, this, [this]() {
        if (filenames.isEmpty()) {
            return;
        }
        QImageReader reader(filenames.first());
        auto size = reader.size();
        sbimgWidth->setValue(size.width());
        sbimgHeight->setValue(size.height());
    });
    // size widgets

    layout->addWidget(w);
    layout->addSpacing(20);
    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(dbbox, &QDialogButtonBox::accepted, this, &NewDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this, &NewDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &NewDialog::on_accept);
    layout->addWidget(dbbox);

    connect(imgslist, &QListWidget::currentItemChanged, this,
            [=](QListWidgetItem *current, QListWidgetItem *previous) {
                if (current) {
                    auto p = QPixmap(current->data(Qt::UserRole).toString());
                    imgview->setPixmap(p);
                } else {
                    imgview->setPixmap(nopic);
                }
                if (previous == nullptr && current) {
                    QImageReader reader(filenames.first());
                    auto size = reader.size();
                    sbimgWidth->setValue(size.width());
                    sbimgHeight->setValue(size.height());
                    sbimgWidth->setEnabled(true);
                    sbimgHeight->setEnabled(true);
                    btnReset->setEnabled(true);
                } else if (previous && current == nullptr) {
                    sbimgWidth->setValue(100);
                    sbimgWidth->setEnabled(false);
                    sbimgHeight->setValue(100);
                    sbimgHeight->setEnabled(false);
                    btnReset->setEnabled(false);
                }
            });

    buildUpContent(widget);

    setWindowTitle(tr("New"));
    setWindowIcon(ICONRES("new"));
}

QStringList NewDialog::getFilenames() { return filenames; }

QSize NewDialog::getGifFrameSize() {
    if (filenames.isEmpty()) {
        return {};
    }
    return QSize(sbimgWidth->value(), sbimgHeight->value());
}

void NewDialog::on_accept() {
    if (filenames.count()) {
        done(1);
        return;
    }
    Toast::toast(this, NAMEICONRES("new"), tr("NoImageFile"));
}

void NewDialog::on_reject() { done(0); }

void NewDialog::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event)
    done(0);
}
