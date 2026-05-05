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

#ifndef ASGIFIMAGE_H
#define ASGIFIMAGE_H

#include <QFont>
#include <QImage>
#include <QPen>

class ASGifImage {
public:
    ASGifImage();

public:
    // AngelScript ref counting
    void addRef() const noexcept;
    void release() const noexcept;

public:
    bool isNull() const;

public:
    int width() const;
    int height() const;

    void fill(const QColor &color);
    void clear();

    QColor pixelColor(int x, int y) const;
    void setPixelColor(int x, int y, const QColor &c);

    void setPenColor(const QColor &color);
    QColor penColor() const;

    void setPenWidth(int width);

    void setFont(const QString &family, int pointSize, bool bold, bool italic);

    void drawPoint(int x, int y);
    void drawLine(int x1, int y1, int x2, int y2);
    void drawArc(int x, int y, int width, int height, int startAngle,
                 int spanAngle);
    void drawChord(int x, int y, int width, int height, int startAngle,
                   int spanAngle);
    void drawPiedrawPie(int x, int y, int width, int height, int startAngle,
                        int spanAngle);
    void drawRect(int x, int y, int w, int h);
    void drawEllipse(int x, int y, int w, int h);

    void fillRect(int x, int y, int w, int h, const QColor &color);

    void drawText(int x, int y, const QString &text);
    void drawText(const QPoint &pt, const QString &text);

public:
    const QImage &image() const;
    void setFrame(const QImage &image);

private:
    void ensureValidFrameFormat();
    bool inBounds(int x, int y) const;
    void applyPainterState(QPainter &p) const;

private:
    mutable std::atomic<int> _refCount{1};

    QImage _frame;

    QPen _pen{Qt::black};
    QFont _font;
};

#endif // ASGIFIMAGE_H
