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

#ifndef SCALEGIFDIALOG_H
#define SCALEGIFDIALOG_H

#include "framelessdialogbase.h"

#include <QDialog>
#include <QMainWindow>
#include <QObject>
#include <QSpinBox>

struct ScaleResult {
    int width;
    int height;
};

class ScaleGIFDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    ScaleGIFDialog(QSize size, QWidget *parent = nullptr);
    ScaleResult getResult();

private:
    void on_accept();
    void on_reject();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QSpinBox *sbwidth;
    QSpinBox *sbheight;
    bool _lockscale = true;
    int _oldwidth = 0, _oldheight = 0;
    ScaleResult res;
};

#endif // SCALEGIFDIALOG_H
