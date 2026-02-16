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

#include "removeframecommand.h"

RemoveFrameCommand::RemoveFrameCommand(GifContentModel *helper,
                                       QVector<int> &frames,
                                       QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper) {
    indices = frames;
    std::sort(indices.begin(), indices.end(), std::greater<int>());

    for (auto i : indices) {
        if (i < 0 || i >= gif->frameCount()) {
            continue;
        }
        GifData d;
        d.delay = gif->delay(i);
        d.image = gif->image(i);
        imgs.append(d);
    }
}

void RemoveFrameCommand::undo() {
    for (int i = 0; i < indices.size() && i < imgs.size(); ++i) {
        gif->insertFrame(imgs.at(i), indices.at(i));
    }

    if (auto lv = gif->linkedListView(); lv && !indices.isEmpty()) {
        const auto target = qBound(0, indices.last(), static_cast<int>(gif->frameCount() - 1));
        if (target >= 0) {
            lv->setCurrentIndex(gif->index(target));
        }
    }
}

void RemoveFrameCommand::redo() {
    for (auto p = indices.rbegin(); p < indices.rend(); ++p) {
        if (*p >= 0 && *p < gif->frameCount()) {
            gif->removeFrames(*p);
        }
    }

    if (auto lv = gif->linkedListView(); lv && !indices.isEmpty()) {
        auto l = indices.last();
        const auto c = static_cast<int>(gif->frameCount());
        if (c > 0) {
            if (l >= c) {
                l = c - 1;
            }
            lv->setCurrentIndex(gif->index(l));
        }
    }
}
