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
#include "class/gifframe.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QDataStream>
#include <QMimeData>
#include <QTemporaryFile>

bool ClipBoardHelper::setImageFrames(
    const QVector<QSharedPointer<GifFrame>> &frames) {
    auto clipboard = qApp->clipboard();
    qsizetype total = frames.count();
    QByteArray buffer;
    if (total > 0) {
        QBuffer bu(&buffer);
        auto b = bu.open(QBuffer::WriteOnly);
        Q_ASSERT(b);
        QDataStream s(&bu);
        s << QString::fromLatin1(APP_NAME) << total;

        for (auto &frame : frames) {
            s << frame->delay() << frame->image();
            if (buffer.size() > ClipboardMemLimit) {
                return false;
            }
        }
    }

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/octet-stream", buffer);
    clipboard->setMimeData(mimeData);
    return true;
}

void ClipBoardHelper::getImageFrames(
    QVector<QSharedPointer<GifFrame>> &frames) {

    frames.clear();
    auto clipboard = qApp->clipboard();
    auto buffer = clipboard->mimeData()->data("application/octet-stream");
    QBuffer bu(&buffer);
    auto b = bu.open(QBuffer::ReadOnly);
    Q_ASSERT(b);
    QDataStream s(&bu);

    QString sig;
    s >> sig;
    if (sig != APP_NAME) {
        return;
    }

    qsizetype total;
    s >> total;
    for (qsizetype i = 0; i < total; ++i) {
        int delay;
        QImage image;
        s >> delay >> image;

        QTemporaryFile tfile(QStringLiteral("frame_XXXXXX.png"));
        tfile.setAutoRemove(false);
        if (!tfile.open()) {
            continue;
        }

        auto img =
            QSharedPointer<GifFrame>::create(image, delay, tfile.fileName());
        if (!img->isValidFrame()) {
            continue;
        }

        frames.append(img);
    }
}
