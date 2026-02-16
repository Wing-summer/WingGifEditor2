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

#include "cropimagecommand.h"

#include <QDir>
#include <utility>

CropImageCommand::CropImageCommand(GifContentModel *helper, const QRect &rect,
                                   QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper), _rect(rect) {
    bufferDir = std::make_unique<QTemporaryDir>();
    if (!bufferDir->isValid()) {
        bufferDir.reset();
        return;
    }

    const auto count = helper->frameCount();
    bufferFiles.reserve(count);
    for (qsizetype i = 0; i < count; ++i) {
        const auto path = bufferDir->path() + QDir::separator() +
                          QStringLiteral("crop_%1.png").arg(i, 6, 10, QLatin1Char('0'));
        helper->image(i).save(path, "PNG");
        bufferFiles.push_back(path);
    }
}

void CropImageCommand::undo() {
    if (!gif || bufferFiles.isEmpty()) {
        return;
    }

    QVector<QImage> frames;
    frames.reserve(bufferFiles.size());
    for (const auto &path : std::as_const(bufferFiles)) {
        QImage img;
        img.load(path);
        if (!img.isNull()) {
            frames.push_back(img);
        }
    }

    if (frames.size() == gif->frameCount()) {
        gif->swapFrames(frames);
        if (auto editor = gif->linkedGifEditor()) {
            editor->fitOpenSize();
        }
    }
}

void CropImageCommand::redo() {
    if (!gif) {
        return;
    }

    gif->cropFrames(_rect);
    if (auto editor = gif->linkedGifEditor()) {
        editor->fitOpenSize();
    }
}
