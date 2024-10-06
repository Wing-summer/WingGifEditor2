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
