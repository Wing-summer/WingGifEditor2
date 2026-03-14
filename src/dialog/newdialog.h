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

#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include "control/aspectratiopixmaplabel.h"
#include "framelessdialogbase.h"

#include <QFileSystemWatcher>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMainWindow>
#include <QSpinBox>
#include <QVBoxLayout>

class NewDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    NewDialog(QWidget *parent = nullptr);

    QStringList getFilenames();

    QSize getGifFrameSize();

private:
    void on_accept();
    void on_reject();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QFileSystemWatcher *_watcher;
    QStringList filenames;

    QListWidget *imgslist;
    AspectRatioPixmapLabel *imgview;

    QSpinBox *sbimgWidth;
    QSpinBox *sbimgHeight;
};

#endif // NEWDIALOG_H
