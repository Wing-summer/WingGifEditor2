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

#include "insertframecommand.h"

InsertFrameCommand::InsertFrameCommand(
    GifContentModel *model, int index,
    const std::function<QPair<int, QImage>(int index)> &imgProc, int total,
    QUndoCommand *parent)
    : UndoCommand(model, parent), _oldindex(index) {
    for (int i = 0; i < total; ++i) {
        auto d = imgProc(i);
        _data.append(model->generateFrame(d.second, d.first));
    }
}

InsertFrameCommand::InsertFrameCommand(
    GifContentModel *model, int index,
    const QVector<QSharedPointer<GifFrame>> &data, QUndoCommand *parent)
    : UndoCommand(model, parent), _oldindex(index), _data(data) {}

void InsertFrameCommand::undo() {
    auto gif = model();
    gif->removeFrames(_oldindex, _data.size());
    if (auto lv = gif->linkedListView()) {
        lv->clearSelection();
        if (gif->frameCount() > 0) {
            const auto target =
                qBound(0, _oldindex, static_cast<int>(gif->frameCount() - 1));
            lv->setCurrentIndex(gif->index(target));
        }
    }
}

void InsertFrameCommand::redo() {
    auto gif = model();
    gif->insertFrames(_oldindex, _data);
    if (auto lv = gif->linkedListView()) {
        lv->clearSelection();
        if (gif->frameCount() > 0) {
            const auto target =
                qBound(0, _oldindex, static_cast<int>(gif->frameCount() - 1));
            lv->setCurrentIndex(gif->index(target));
        }
    }
}
