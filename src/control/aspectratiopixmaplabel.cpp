/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include "aspectratiopixmaplabel.h"

AspectRatioPixmapLabel::AspectRatioPixmapLabel(QWidget *parent)
    : QLabel(parent) {
    this->setMinimumSize(1, 1);
    setScaledContents(false);
}

void AspectRatioPixmapLabel::setPixmap(const QPixmap &pm) {
    pixmapWidth = pm.width();
    pixmapHeight = pm.height();

    updateMargins();
    QLabel::setPixmap(pm);
}

void AspectRatioPixmapLabel::resizeEvent(QResizeEvent *event) {
    updateMargins();
    QLabel::resizeEvent(event);
}

void AspectRatioPixmapLabel::updateMargins() {
    if (pixmapWidth <= 0 || pixmapHeight <= 0)
        return;

    int w = this->width();
    int h = this->height();

    if (w <= 0 || h <= 0)
        return;

    if (w * pixmapHeight > h * pixmapWidth) {
        int m = (w - (pixmapWidth * h / pixmapHeight)) / 2;
        setContentsMargins(m, 0, m, 0);
    } else {
        int m = (h - (pixmapHeight * w / pixmapWidth)) / 2;
        setContentsMargins(0, m, 0, m);
    }
}
