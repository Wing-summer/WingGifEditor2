#include "newdialog.h"
#include "control/toast.h"
#include "utilities.h"

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QPushButton>
#include <QShortcut>
#include <QWidget>

NewDialog::NewDialog(NewType type, QWidget *parent)
    : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    auto nopic = QPixmap(":/com.wingsummer.winggif/images/nopic.jpeg");

    auto btnBox = new QWidget(this);
    auto buttonLayout = new QHBoxLayout(btnBox);
    buttonLayout->setSpacing(0);

    auto b = new QPushButton(tr("Add"), this);
    connect(b, &QPushButton::clicked, this, [=] {
        QFileDialog d;
        if (type == NewType::FromPics) {
            d.setNameFilter(tr("Images (*.jpg *.tiff *.png)"));
        } else {
            d.setNameFilter("GIF (*.gif)");
        }
        d.setAcceptMode(QFileDialog::AcceptOpen);
        d.setFileMode(QFileDialog::ExistingFiles);
        if (d.exec()) {
            auto files = d.selectedFiles();
            for (auto &item : files) {
                QImage img;
                if (img.load(item)) {
                    filenames << item;
                    auto p = new QListWidgetItem(QIcon(":/images/picture.png"),
                                                 item, imgslist);
                    p->setToolTip(item);
                }
            }
        }
    });
    buttonLayout->addWidget(b);

    b = new QPushButton(tr("Remove"), this);
    connect(b, &QPushButton::clicked, this, [=] {
        auto indices = imgslist->selectionModel()->selectedRows();
        QList<int> ins;
        for (auto i : indices) {
            ins.append(i.row());
        }
        std::sort(ins.begin(), ins.end(), std::greater<int>());
        for (auto i : ins) {
            auto p = imgslist->item(i);
            imgslist->removeItemWidget(p);
            delete p;
            filenames.removeAt(i);
        }
    });
    buttonLayout->addWidget(b);

    b = new QPushButton(tr("Clear"), this);
    connect(b, &QPushButton::clicked, this, [=] {
        imgslist->clear();
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
    hbox->addWidget(imgslist);
    auto vbox = new QVBoxLayout(w);
    hbox->addLayout(vbox);
    imgview = new QLabel(this);
    imgview->setFixedSize(200, 200);
    imgview->setScaledContents(true);
    imgview->setPixmap(nopic.scaled(imgview->size(), Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation));
    vbox->addWidget(imgview);
    vbox->addSpacing(5);
    vbox->addWidget(new QLabel(tr("Size"), this));
    imgsize = new QLabel(tr("Unknown"), this);
    vbox->addWidget(imgsize);

    layout->addWidget(w);
    layout->addSpacing(5);
    layout->addWidget(new QLabel(tr("FirstSizeJudge"), this));
    layout->addSpacing(20);
    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(dbbox, &QDialogButtonBox::accepted, this, &NewDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this, &NewDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &NewDialog::on_accept);
    layout->addWidget(dbbox);

    connect(imgslist, &QListWidget::itemSelectionChanged, this, [=] {
        auto cur = imgslist->currentItem();
        if (cur) {
            auto p = QPixmap(cur->text());
            imgview->setPixmap(p.scaled(imgview->size(), Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation));
            imgsize->setText(QString("%1 × %2")
                                 .arg(p.size().width())
                                 .arg(p.size().height()));
        } else {
            imgview->setPixmap(nopic);
            imgsize->setText(tr("Unknown"));
        }
    });

    buildUpContent(widget);

    setWindowTitle(tr("New"));
}

QStringList NewDialog::getResult() { return filenames; }

void NewDialog::on_accept() {
    if (filenames.count()) {
        done(1);
        return;
    }
    Toast::toast(this, NAMEICONRES("new.png"), tr("NoImageFile"));
}

void NewDialog::on_reject() { done(0); }

void NewDialog::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event)
    done(0);
}
