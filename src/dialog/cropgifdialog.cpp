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

#include "cropgifdialog.h"
#include "utilities.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QPushButton>

CropGifDialog::CropGifDialog(QWidget *parent) : FramelessDialogBase(parent) {
    setAttribute(Qt::WA_ShowWithoutActivating);

    auto widget = new QWidget(this);
    auto layout = new QFormLayout(widget);
    layout->setHorizontalSpacing(8);
    layout->setVerticalSpacing(10);

    m_x = new QSpinBox(this);
    layout->addRow(QStringLiteral("X"), m_x);

    m_y = new QSpinBox(this);
    layout->addRow(QStringLiteral("Y"), m_y);

    m_w = new QSpinBox(this);
    layout->addRow(QStringLiteral("W"), m_w);

    m_h = new QSpinBox(this);
    layout->addRow(QStringLiteral("H"), m_h);

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(0);

    int id = 0;

    auto b = new QPushButton(tr("Cut"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [this](bool b) {
        m_rect.setRect(m_x->value(), m_y->value(), m_w->value(), m_h->value());
        this->accept();
    });
    buttonLayout->addWidget(b);

    b = new QPushButton(tr("Cancel"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, &CropGifDialog::reject);
    buttonLayout->addWidget(b);

    layout->addRow(buttonLayout);
    setFocusPolicy(Qt::StrongFocus);
    widget->setMinimumWidth(280);
    buildUpContent(widget);
    setWindowIcon(ICONRES(QStringLiteral("cutpic")));
    setWindowTitle(tr("CropGif"));

    this->layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void CropGifDialog::setMaxSize(QSize maxsize) {
    m_max = maxsize;
    m_x->setMaximum(maxsize.width());
    m_y->setMaximum(maxsize.height());
    m_w->setMaximum(maxsize.width());
    m_h->setMaximum(maxsize.height());
}

void CropGifDialog::setSelRect(const QRectF &rect) {
    auto x = qMin(qMax(int(rect.x()), 0), m_max.width());
    auto y = qMin(qMax(int(rect.y()), 0), m_max.height());
    auto w = qMin(qMax(int(rect.width()), 0), m_max.width() - x);
    auto h = qMin(qMax(int(rect.height()), 0), m_max.height() - y);
    m_x->setValue(x);
    m_y->setValue(y);
    m_w->setMaximum(m_max.width() - x);
    m_h->setMaximum(m_max.height() - y);
    m_w->setValue(w);
    m_h->setValue(h);
}

void CropGifDialog::closeEvent(QCloseEvent *event) {
    m_rect.setRect(0, 0, 0, 0);
    done(0);
}

QRect CropGifDialog::rect() const { return m_rect; }
