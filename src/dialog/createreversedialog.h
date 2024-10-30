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

#ifndef CREATEREVERSEDIALOG_H
#define CREATEREVERSEDIALOG_H

#include "framelessdialogbase.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QMainWindow>
#include <QSpinBox>

struct ReverseResult {
    int start = -1;
    int end = -1;
};

class CreateReverseDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    CreateReverseDialog(int max, QWidget *parent = nullptr);
    ReverseResult getResult();

private:
    void on_accept();
    void on_reject();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QSpinBox *sbfrom, *sbto;
    ReverseResult res;
};

#endif // CREATEREVERSEDIALOG_H
