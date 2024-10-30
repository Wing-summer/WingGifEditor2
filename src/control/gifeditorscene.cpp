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

#include "gifeditorscene.h"
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>

#include <QAtomicInt>
#include <QColor>
#include <QHash>

size_t qHash(const QColor &color, size_t seed = 0) noexcept {
    return qHash(qMakePair(color.rgba(), color.spec()), seed);
}

GifEditorScene::GifEditorScene(const QImage &img, QObject *parent)
    : QGraphicsScene(parent) {
    sel = new ImageCropper();
    sel->setAttribute(Qt::WA_TranslucentBackground);
    sel->setBackgroundColor(Qt::transparent);
    sel->setCropperVisible(false);
    sel->setCroppingRectBorderColor(Qt::gray);
    setFrameImg(img);
    gw = addWidget(sel);
    connect(sel, &ImageCropper::sizeChanged, gw, [=] {
        // call the internal updateProxyGeometryFromWidget
        gw->resetTransform();
        gw->setWidget(nullptr);
        gw->setWidget(sel);
        // center the image
        setSceneRect(QRectF(QPointF(), gw->size()));
    });
    connect(sel, &ImageCropper::selRectChanged, this,
            &GifEditorScene::selRectChanged);
}

GifEditorScene::~GifEditorScene() { sel->deleteLater(); }

bool GifEditorScene::isCuttingMode() { return sel->cropperVisible(); }

void GifEditorScene::setCuttingMode(bool value) {
    sel->setCropperVisible(value);
}

void GifEditorScene::setFrameImg(const QImage &img) {
    sel->setImage(img);
    auto mainColor = getMainThemeColor(img);
    if (!img.isNull()) {
        sel->setCroppingRectBorderColor(getContrastingColor(mainColor));
    }
}

QRectF GifEditorScene::contentBounding() const { return gw->boundingRect(); }

QSize GifEditorScene::frameImageSize() const { return sel->imageSize(); }

QRectF GifEditorScene::selRect() const { return sel->selRect(); }

void GifEditorScene::setSelRect(int x, int y, int w, int h) {
    sel->setSelRect(x, y, w, h);
}

double GifEditorScene::getLuminance(const QColor &color) {
    return 0.299 * color.red() + 0.587 * color.green() + 0.114 * color.blue();
}

QColor GifEditorScene::getContrastingColor(const QColor &color) {
    double luminance = getLuminance(color);
    return (luminance > 128) ? QColor(Qt::black) : QColor(Qt::white);
}

QColor GifEditorScene::getMainThemeColor(const QImage &image) {
    QHash<QColor, QAtomicInt> colorCount;

    // Reduce the image size to speed up the process
    QImage scaledImage = image.scaled(50, 50);

    // Count each pixel's color
    for (int y = 0; y < scaledImage.height(); ++y) {
        for (int x = 0; x < scaledImage.width(); ++x) {
            QColor color = scaledImage.pixelColor(x, y);
            colorCount[color]++;
        }
    }

    // Find the most frequent color
    QColor dominantColor;
    int maxCount = 0;
    for (auto it = colorCount.constBegin(); it != colorCount.constEnd(); ++it) {
        if (it.value() > maxCount) {
            maxCount = it.value();
            dominantColor = it.key();
        }
    }

    return dominantColor;
}
