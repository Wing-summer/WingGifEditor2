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

#ifndef GIFEDITORSCENE_H
#define GIFEDITORSCENE_H

#include "imagecropper/imagecropper.h"
#include <QGraphicsScene>

class GifEditorScene : public QGraphicsScene {
    Q_OBJECT
public:
    GifEditorScene(const QImage &img, QObject *parent = nullptr);
    virtual ~GifEditorScene();

    bool isCuttingMode();
    void setCuttingMode(bool value);

    void setFrameImg(const QImage &img);

    QRectF contentBounding() const;

    QSize frameImageSize() const;

    QRectF selRect() const;

    QImage renderedImage() const;

public slots:
    void setSelRect(int x, int y, int w, int h);

signals:
    void selRectChanged(const QRectF &rect);

private:
    // Function to calculate luminance of a color
    double getLuminance(const QColor &color);

    // Function to get the contrasting color (white or black)
    QColor getContrastingColor(const QColor &color);

    // Function to get the main theme color from the image
    QColor getMainThemeColor(const QImage &image);

private:
    ImageCropper *sel;
    QPointF oldpos;
    QGraphicsProxyWidget *gw;
};

#endif // GIFEDITORSCENE_H
