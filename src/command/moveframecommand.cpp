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

#include "moveframecommand.h"

MoveFrameCommand::MoveFrameCommand(
    GifContentModel *model, GifContentModel::MoveFrameDirection direction,
    const QVector<int> &indices, QUndoCommand *parent)
    : UndoCommand(model, parent), dir(direction), oldindices(indices) {
    if (oldindices.isEmpty()) {
        setObsolete(true);
        return;
    }
    std::sort(oldindices.begin(), oldindices.end());
}

void MoveFrameCommand::undo() {
    auto gif = model();
    auto lv = gif->linkedListView();
    if (!lv || !lv->selectionModel()) {
        return;
    }

    auto cur = lv->currentIndex().row();
    auto sel = lv->selectionModel();
    sel->clearSelection();

    switch (dir) {
    case GifContentModel::MoveFrameDirection::Left: {
        for (auto pi = oldindices.crbegin(); pi != oldindices.crend(); ++pi) {
            if ((*pi) - 1 >= 0) {
                gif->moveFrames((*pi) - 1,
                                GifContentModel::MoveFrameDirection::Right);
                sel->select(gif->index(*pi), QItemSelectionModel::Select);
            }
        }
        if (gif->frameCount() > 0) {
            sel->select(gif->index(qBound(0, cur + 1,
                                          static_cast<int>(gif->frameCount() - 1))),
                        QItemSelectionModel::Current);
        }
        break;
    }
    case GifContentModel::MoveFrameDirection::Right: {
        for (auto pi = oldindices.cbegin(); pi != oldindices.cend(); ++pi) {
            if ((*pi) + 1 < gif->frameCount()) {
                gif->moveFrames((*pi) + 1,
                                GifContentModel::MoveFrameDirection::Left);
                sel->select(gif->index(*pi), QItemSelectionModel::Select);
            }
        }
        if (gif->frameCount() > 0) {
            sel->select(gif->index(qBound(0, cur - 1,
                                          static_cast<int>(gif->frameCount() - 1))),
                        QItemSelectionModel::Current);
        }
        break;
    }
    }
}

void MoveFrameCommand::redo() {
    auto gif = model();
    auto lv = gif->linkedListView();
    if (!lv || !lv->selectionModel()) {
        return;
    }

    auto cur = lv->currentIndex().row();
    auto sel = lv->selectionModel();
    sel->clearSelection();

    switch (dir) {
    case GifContentModel::MoveFrameDirection::Left: {
        for (auto pi = oldindices.cbegin(); pi != oldindices.cend(); ++pi) {
            if (*pi > 0) {
                gif->moveFrames(*pi, dir);
                sel->select(gif->index((*pi) - 1), QItemSelectionModel::Select);
            }
        }
        if (gif->frameCount() > 0) {
            sel->select(gif->index(qBound(0, cur - 1,
                                          static_cast<int>(gif->frameCount() - 1))),
                        QItemSelectionModel::Current);
        }
        break;
    }
    case GifContentModel::MoveFrameDirection::Right: {
        for (auto pi = oldindices.crbegin(); pi != oldindices.crend(); ++pi) {
            if (*pi + 1 < gif->frameCount()) {
                gif->moveFrames(*pi, dir);
                sel->select(gif->index((*pi) + 1), QItemSelectionModel::Select);
            }
        }
        if (gif->frameCount() > 0) {
            sel->select(gif->index(qBound(0, cur + 1,
                                          static_cast<int>(gif->frameCount() - 1))),
                        QItemSelectionModel::Current);
        }
        break;
    }
    }
}
