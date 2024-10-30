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

#include "moveframecommand.h"

MoveFrameCommand::MoveFrameCommand(
    GifContentModel *helper, GifContentModel::MoveFrameDirection direction,
    const QVector<int> &indices, QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper), dir(direction), oldindices(indices) {
    std::sort(oldindices.begin(), oldindices.end());
}

void MoveFrameCommand::undo() {
    auto lv = gif->linkedListView();
    auto cur = lv->currentIndex().row();
    auto sel = lv->selectionModel();
    sel->clearSelection();

    switch (dir) {
    case GifContentModel::MoveFrameDirection::Left: {
        for (auto pi = oldindices.crbegin(); pi != oldindices.crend(); ++pi) {
            gif->moveFrames((*pi) - 1,
                            GifContentModel::MoveFrameDirection::Right);
            sel->select(gif->index(*pi), QItemSelectionModel::Select);
        }
        sel->select(gif->index(cur + 1), QItemSelectionModel::Current);
        break;
    }
    case GifContentModel::MoveFrameDirection::Right: {
        for (auto pi = oldindices.cbegin(); pi != oldindices.cend(); ++pi) {
            gif->moveFrames((*pi) + 1,
                            GifContentModel::MoveFrameDirection::Left);
            sel->select(gif->index(*pi), QItemSelectionModel::Select);
        }
        sel->select(gif->index(cur - 1), QItemSelectionModel::Current);
        break;
    }
    }
}

void MoveFrameCommand::redo() {
    auto lv = gif->linkedListView();
    auto cur = lv->currentIndex().row();
    auto sel = lv->selectionModel();
    sel->clearSelection();

    switch (dir) {
    case GifContentModel::MoveFrameDirection::Left: {
        for (auto pi = oldindices.cbegin(); pi != oldindices.cend(); ++pi) {
            gif->moveFrames(*pi, dir);
            sel->select(gif->index((*pi) - 1), QItemSelectionModel::Select);
        }
        sel->select(gif->index(cur - 1), QItemSelectionModel::Current);
        break;
    }
    case GifContentModel::MoveFrameDirection::Right: {
        for (auto pi = oldindices.crbegin(); pi != oldindices.crend(); ++pi) {
            gif->moveFrames(*pi, dir);
            sel->select(gif->index((*pi) + 1), QItemSelectionModel::Select);
        }
        sel->select(gif->index(cur + 1), QItemSelectionModel::Current);
        break;
    }
    }
}
