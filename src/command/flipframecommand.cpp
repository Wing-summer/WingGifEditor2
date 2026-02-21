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

#include "flipframecommand.h"

FlipFrameCommand::FlipFrameCommand(GifContentModel *model,
                                   const QVector<int> &indices,
                                   Qt::Orientation dir, QUndoCommand *parent)
    : UndoCommand(model, parent), _dir(dir) {
    if (indices.isEmpty()) {
        setObsolete(true);
    }
    for (auto &index : indices) {
        _oldcache.insert(index, model->frame(index));
    }
}

void FlipFrameCommand::undo() {
    auto gif = model();

    for (auto &&[index, frame] : _oldcache.asKeyValueRange()) {
        gif->replaceFrame(index, frame);
    }

    // force update
    if (auto lv = gif->linkedListView(); lv && lv->selectionModel()) {
        auto i = lv->currentIndex();
        emit lv->selectionModel()->currentRowChanged(i, i);
    }
}

void FlipFrameCommand::redo() {
    auto gif = model();

    if (_newcache.isEmpty()) {
        _newcache = gif->flipFrames(_oldcache.keys(), _dir);
    } else {
        for (auto &&[index, frame] : _newcache.asKeyValueRange()) {
            gif->replaceFrame(index, frame);
        }
    }

    // force update
    if (auto lv = gif->linkedListView(); lv && lv->selectionModel()) {
        auto i = lv->currentIndex();
        emit lv->selectionModel()->currentRowChanged(i, i);
    }
}
