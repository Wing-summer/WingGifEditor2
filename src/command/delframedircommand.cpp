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

#include "delframedircommand.h"

#include <QDir>
#include <utility>

DelFrameDirCommand::DelFrameDirCommand(GifContentModel *helper, int index,
                                       DelDirection dir, QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper), oldindex(index), olddir(dir) {
    bufferDir = std::make_unique<QTemporaryDir>();
    if (!bufferDir->isValid()) {
        bufferDir.reset();
        return;
    }

    const auto delays = helper->delays();
    const auto count = helper->frameCount();

    qsizetype start = 0;
    qsizetype end = 0;
    if (olddir == DelDirection::Before) {
        start = 0;
        end = qBound<qsizetype>(0, index, count);
    } else {
        start = qBound<qsizetype>(0, index + 1, count);
        end = count;
    }

    oldimgFiles.reserve(end - start);
    olddelays.reserve(end - start);
    for (qsizetype i = start; i < end; ++i) {
        const auto path = bufferDir->path() + QDir::separator() +
                          QStringLiteral("deldir_%1.png").arg(i, 6, 10, QLatin1Char('0'));
        helper->image(i).save(path, "PNG");
        oldimgFiles.push_back(path);
        olddelays.push_back(delays.at(i));
    }
}

void DelFrameDirCommand::undo() {
    if (!gif || oldimgFiles.isEmpty()) {
        return;
    }

    QVector<QImage> oldimgs;
    oldimgs.reserve(oldimgFiles.size());
    for (const auto &path : std::as_const(oldimgFiles)) {
        QImage img;
        img.load(path);
        if (!img.isNull()) {
            oldimgs.push_back(img);
        }
    }
    if (oldimgs.size() != olddelays.size()) {
        return;
    }

    switch (olddir) {
    case DelDirection::Before:
        gif->insertFrames(oldimgs, olddelays, 0);
        break;
    case DelDirection::After:
        gif->insertFrames(oldimgs, olddelays, -1);
        break;
    }

    if (auto lv = gif->linkedListView()) {
        lv->clearSelection();
        const auto target = qBound(0, oldindex, static_cast<int>(gif->frameCount() - 1));
        if (target >= 0) {
            lv->setCurrentIndex(gif->index(target));
        }
    }
}

void DelFrameDirCommand::redo() {
    if (!gif || gif->frameCount() == 0) {
        return;
    }

    switch (olddir) {
    case DelDirection::Before:
        if (oldindex > 0) {
            gif->removeFrames(0, oldindex);
        }
        break;
    case DelDirection::After:
        if (oldindex + 1 < gif->frameCount()) {
            gif->removeFrames(oldindex + 1, -1);
        }
        break;
    }

    if (auto lv = gif->linkedListView()) {
        lv->clearSelection();
        if (gif->frameCount() > 0) {
            const auto target = qBound(0, oldindex,
                                       static_cast<int>(gif->frameCount() - 1));
            lv->setCurrentIndex(gif->index(target));
        }
    }
}
