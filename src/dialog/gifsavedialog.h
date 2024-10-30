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

#ifndef GIFSAVEDIALOG_H
#define GIFSAVEDIALOG_H

#include "framelessdialogbase.h"

#include <QLineEdit>
#include <QSpinBox>

struct GifSaveResult {
    unsigned int loop = 0;
    QString comment;
};

class GifSaveDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    explicit GifSaveDialog(QWidget *parent = nullptr);
    virtual ~GifSaveDialog();

    GifSaveResult getResult() const;

private slots:
    void on_accept();
    void on_reject();

private:
    GifSaveResult _res;

    QSpinBox *_loop;
    QLineEdit *_comment;
};

#endif // GIFSAVEDIALOG_H
