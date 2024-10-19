#ifndef IMAGECROPPER_H
#define IMAGECROPPER_H

#include "imagecropper_e.h"
#include "imagecropper_p.h"

#include <QWidget>

class ImageCropper : public QWidget {
    Q_OBJECT

public:
    ImageCropper(QWidget *parent = nullptr);
    ~ImageCropper();

    bool cropperVisible() const;

    QSizeF proportion() const;
    bool proportionFixed() const;

    QRectF selRect() const;

    QSize imageSize() const;

    void setSelRect(int x, int y, int w, int h);

public slots:
    void setImage(const QImage &_image);
    void setBackgroundColor(const QColor &_backgroundColor);
    void setCroppingRectBorderColor(const QColor &_borderColor);
    void setProportion(const QSizeF &_proportion);
    void setProportionFixed(const bool _isFixed);

    void setCropperVisible(bool b);

signals:
    void selRectChanged(const QRectF &rect);
    void sizeChanged();

protected:
    virtual void paintEvent(QPaintEvent *_event) override;
    virtual void mousePressEvent(QMouseEvent *_event) override;
    virtual void mouseMoveEvent(QMouseEvent *_event) override;
    virtual void mouseReleaseEvent(QMouseEvent *_event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    CursorPosition cursorPosition(const QRectF &_cropRect,
                                  const QPointF &_mousePosition);
    void updateCursorIcon(const QPointF &_mousePosition);

    const QRectF calculateGeometry(const QRectF &_sourceGeometry,
                                   const CursorPosition _cursorPosition,
                                   const QPointF &_mouseDelta);

    const QRectF
    calculateGeometryWithCustomProportions(const QRectF &_sourceGeometry,
                                           const CursorPosition _cursorPosition,
                                           const QPointF &_mouseDelta);

    const QRectF calculateGeometryWithFixedProportions(
        const QRectF &_sourceGeometry, const CursorPosition _cursorPosition,
        const QPointF &_mouseDelta, const QSizeF &_deltas);

    QRectF getImageRect(QSize wsize);

private:
    // Private data implementation
    ImageCropperPrivate *pimpl;
};

#endif // IMAGECROPPER_H
