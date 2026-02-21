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

#include "removeframecommand.h"

RemoveFrameCommand::RemoveFrameCommand(GifContentModel *model,
                                       const QVector<int> &findices,
                                       QUndoCommand *parent)
    : UndoCommand(model, parent) {
    if (findices.isEmpty()) {
        setObsolete(true);
        return;
    }

    _indices = findices;
    std::sort(_indices.begin(), _indices.end(), std::greater<int>());
    for (auto i : std::as_const(_indices)) {
        if (i < 0 || i >= model->frameCount()) {
            continue;
        }
        imgs.append(model->frame(i));
    }
}

void RemoveFrameCommand::undo() {
    auto gif = model();

    for (int i = 0; i < _indices.size() && i < imgs.size(); ++i) {
        gif->insertFrame(_indices.at(i), imgs.at(i));
    }

    if (auto lv = gif->linkedListView(); lv && !_indices.isEmpty()) {
        const auto target =
            qBound(0, _indices.last(), static_cast<int>(gif->frameCount() - 1));
        if (target >= 0) {
            lv->setCurrentIndex(gif->index(target));
        }
    }
}

void RemoveFrameCommand::redo() {
    auto gif = model();

    for (auto p = _indices.rbegin(); p < _indices.rend(); ++p) {
        if (*p >= 0 && *p < gif->frameCount()) {
            gif->removeFrame(*p);
        }
    }

    if (auto lv = gif->linkedListView(); lv && !_indices.isEmpty()) {
        auto l = _indices.last();
        const auto c = static_cast<int>(gif->frameCount());
        if (c > 0) {
            if (l >= c) {
                l = c - 1;
            }
            lv->setCurrentIndex(gif->index(l));
        }
    }
}
