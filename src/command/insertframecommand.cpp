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

#include "insertframecommand.h"

InsertFrameCommand::InsertFrameCommand(GifContentModel *model, int index,
                                       const QVector<GifData> &images,
                                       QUndoCommand *parent)
    : QUndoCommand(parent), gif(model), oldindex(index), oldimgs(images) {}

void InsertFrameCommand::undo() {
    gif->removeFrames(oldindex, oldimgs.size());
    if (auto lv = gif->linkedListView()) {
        lv->clearSelection();
        if (gif->frameCount() > 0) {
            const auto target = qBound(0, oldindex,
                                       static_cast<int>(gif->frameCount() - 1));
            lv->setCurrentIndex(gif->index(target));
        }
    }
}

void InsertFrameCommand::redo() {
    gif->insertFrames(oldimgs, oldindex);

    if (auto lv = gif->linkedListView()) {
        lv->clearSelection();
        if (gif->frameCount() > 0) {
            const auto target = qBound(0, oldindex,
                                       static_cast<int>(gif->frameCount() - 1));
            lv->setCurrentIndex(gif->index(target));
        }
    }
}
