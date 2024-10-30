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

#include "framelessdialogbase.h"

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QObject>
#include <QVBoxLayout>

enum class NewType { FromPics, FromGifs };

class NewDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    NewDialog(NewType type, QWidget *parent = nullptr);
    QStringList getResult();

private:
    void on_accept();
    void on_reject();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QStringList filenames;

    QListWidget *imgslist;
    QLabel *imgview;
    QLabel *imgsize;
};

#endif // NEWDIALOG_H
