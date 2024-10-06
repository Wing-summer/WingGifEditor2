#ifndef IMAGECROPPER_P_H
#define IMAGECROPPER_P_H

#include "imagecropper_e.h"

#include <QtCore/QRect>
#include <QtGui/QColor>
#include <QtGui/QPixmap>

namespace {
const QRect INIT_CROPPING_RECT = QRect();
const QSizeF INIT_PROPORTION = QSizeF(1.0, 1.0);
} // namespace

class ImageCropperPrivate {
public:
    ImageCropperPrivate()
        : imageForCropping(QImage()), croppingRect(INIT_CROPPING_RECT),
          lastStaticCroppingRect(QRectF()),
          cursorPosition(CursorPositionUndefined), isMousePressed(false),
          isProportionFixed(false), cropVisible(true), startMousePos(QPoint()),
          proportion(INIT_PROPORTION), deltas(INIT_PROPORTION),
          backgroundColor(Qt::black), croppingRectBorderColor(Qt::white) {}

public:
    QImage imageForCropping;
    QRectF croppingRect;
    QRectF lastStaticCroppingRect;
    CursorPosition cursorPosition;
    bool isMousePressed;
    bool isProportionFixed;
    bool cropVisible;
    QPointF startMousePos;
    QSizeF proportion;
    QSizeF deltas;
    QColor backgroundColor;
    QColor croppingRectBorderColor;
};

#endif // IMAGECROPPER_P_H
