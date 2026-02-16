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

#include "reduceframecommand.h"

#include <QDir>

ReduceFrameCommand::ReduceFrameCommand(GifContentModel *helper,
                                       const QVector<int> &indices,
                                       const QVector<int> &interval,
                                       QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper), inter(interval) {
    oldinter = gif->delays();

    bufferDir = std::make_unique<QTemporaryDir>();
    if (!bufferDir->isValid()) {
        bufferDir.reset();
        return;
    }

    for (auto ind : indices) {
        if (ind < 0 || ind >= gif->frameCount()) {
            continue;
        }
        const auto path = bufferDir->path() + QDir::separator() +
                          QStringLiteral("reduce_%1.png").arg(ind, 6, 10, QLatin1Char('0'));
        gif->image(ind).save(path, "PNG");
        oldimgFiles.insert(ind, path);
    }
}

void ReduceFrameCommand::undo() {
    for (auto p = oldimgFiles.constKeyValueBegin(); p != oldimgFiles.constKeyValueEnd();
         ++p) {
        QImage img;
        img.load(p->second);
        if (!img.isNull()) {
            gif->insertFrame(img, 1, p->first); // delay restored below
        }
    }

    const auto len = qMin(oldinter.count(), static_cast<int>(gif->frameCount()));
    for (int i = 0; i < len; ++i) {
        gif->setFrameDelay(i, oldinter.at(i));
    }
}

void ReduceFrameCommand::redo() {
    QMapIterator<int, QString> it(oldimgFiles);
    it.toBack();
    while (it.hasPrevious()) {
        it.previous();
        gif->removeFrames(it.key());
    }

    const auto len = qMin(inter.count(), static_cast<int>(gif->frameCount()));
    for (int i = 0; i < len; ++i) {
        gif->setFrameDelay(i, inter.at(i));
    }
}
