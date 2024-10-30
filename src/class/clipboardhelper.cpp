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

#include "clipboardhelper.h"
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QMimeData>
#include <QPainter>

void ClipBoardHelper::setImageFrames(const QVector<GifData> &frames) {
    auto clipboard = qApp->clipboard();
    QByteArray buffer;
    auto len = frames.count();
    buffer.append(APP_NAME, strlen(APP_NAME));
    buffer.append(reinterpret_cast<const char *>(&len), sizeof(int));
    for (auto &frame : frames) {
        QByteArray img;
        QBuffer bu(&img);
        auto b = bu.open(QBuffer::WriteOnly);
        Q_ASSERT(b);
        frame.image.save(&bu, "PNG");
        len = img.size();
        buffer.append(reinterpret_cast<const char *>(&len), sizeof(int));
        buffer.append(img);
        buffer.append(reinterpret_cast<const char *>(&frame.delay),
                      sizeof(int));
    }

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/octet-stream", buffer);
    clipboard->setMimeData(mimeData);
}

void ClipBoardHelper::getImageFrames(QVector<GifData> &frames) {
    frames.clear();
    auto clipboard = qApp->clipboard();
    auto buffer = clipboard->mimeData()->data("application/octet-stream");
    if (!buffer.isEmpty() && buffer.startsWith(APP_NAME)) {
        auto pb = buffer.data() + strlen(APP_NAME);
        auto epb = pb + buffer.size();
        int count = *reinterpret_cast<int *>(pb);
        if (count < 0)
            return;
        pb += sizeof(int);
        for (auto i = 0; i < count && pb < epb; i++) {
            auto imgbytes = *reinterpret_cast<int *>(pb);
            pb += sizeof(int);
            QImage img;
            if (img.loadFromData(reinterpret_cast<const uchar *>(pb),
                                 imgbytes)) {
                GifData d;
                QImage im(img.size(), QImage::Format_RGBA8888);
                QPainter p(&im);
                p.drawImage(QPoint(), img);
                d.image.swap(im);
                pb += imgbytes;
                d.delay = *reinterpret_cast<int *>(pb);
                pb += sizeof(int);
                frames.append(d);
            } else {
                pb += (ulong(imgbytes) + sizeof(int));
            }
        }
    }
}
