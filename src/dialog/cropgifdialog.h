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

#ifndef CROPGIFDIALOG_H
#define CROPGIFDIALOG_H

#include "framelessdialogbase.h"

#include <QDialog>
#include <QLabel>
#include <QMainWindow>
#include <QSpinBox>

class CropGifDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    CropGifDialog(QWidget *parent = nullptr);

    void setMaxSize(QSize maxsize);
    void sigSelRectChanged();

public slots:
    void setSelRect(const QRectF &rect);

signals:
    void crop(qreal x, qreal y, qreal w, qreal h);
    void selRectChanged(qreal x, qreal y, qreal w, qreal h);
    void pressCancel();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QSpinBox *m_x, *m_y, *m_w, *m_h;
    QSize m_max;
};

#endif // CROPGIFDIALOG_H
