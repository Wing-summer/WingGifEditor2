#include "cropgifdialog.h"
#include <QHBoxLayout>
#include <QPushButton>

CropGifDialog::CropGifDialog(QWidget *parent) : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    layout->addWidget(new QLabel(QStringLiteral("X : "), this));
    layout->addSpacing(6);
    m_x = new QSpinBox(this);
    connect(m_x, &QSpinBox::editingFinished, this, [=] {
        m_w->setMaximum(m_max.width() - m_x->value());
        sigSelRectChanged();
    });
    layout->addWidget(m_x);
    layout->addSpacing(10);
    layout->addWidget(new QLabel(QStringLiteral("Y : "), this));
    layout->addSpacing(6);
    m_y = new QSpinBox(this);
    connect(m_y, &QSpinBox::editingFinished, this, [=] {
        m_h->setMaximum(m_max.height() - m_y->value());
        sigSelRectChanged();
    });
    layout->addWidget(m_y);
    layout->addSpacing(10);
    layout->addWidget(new QLabel(QStringLiteral("W : "), this));
    layout->addSpacing(6);
    m_w = new QSpinBox(this);
    connect(m_w, &QSpinBox::editingFinished, this,
            [=] { sigSelRectChanged(); });
    layout->addWidget(m_w);
    layout->addSpacing(10);
    layout->addWidget(new QLabel(QStringLiteral("H : "), this));
    layout->addSpacing(6);
    m_h = new QSpinBox(this);
    connect(m_h, &QSpinBox::editingFinished, this,
            [=] { sigSelRectChanged(); });
    layout->addWidget(m_h);
    layout->addSpacing(20);

    auto btnBox = new QWidget(this);
    auto buttonLayout = new QHBoxLayout(btnBox);
    buttonLayout->setSpacing(0);

    int id = 0;

    auto b = new QPushButton(tr("Cut"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [=](bool b) {
        emit this->crop(m_x->value(), m_y->value(), m_w->value(), m_h->value());
        this->close();
    });
    buttonLayout->addWidget(b);

    b = new QPushButton(tr("Cancel"), this);
    b->setCheckable(true);
    connect(b, &QPushButton::toggled, this, [=](bool b) {
        emit this->pressCancel();
        this->close();
    });
    buttonLayout->addWidget(b);

    layout->addWidget(btnBox);
    setFocusPolicy(Qt::StrongFocus);

    buildUpContent(widget);

    setWindowTitle(tr("CropGifDialog"));
}

void CropGifDialog::setMaxSize(QSize maxsize) {
    m_max = maxsize;
    m_x->setMaximum(maxsize.width());
    m_y->setMaximum(maxsize.height());
    m_w->setMaximum(maxsize.width());
    m_h->setMaximum(maxsize.height());
}

void CropGifDialog::sigSelRectChanged() {
    auto x = m_x->value();
    auto y = m_y->value();
    auto w = m_w->value();
    auto h = m_h->value();
    emit selRectChanged(x, y, w, h);
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
    emit selRectChanged(x, y, w, h);
}

void CropGifDialog::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event);
    emit crop(-1, -1, -1, -1);
}
