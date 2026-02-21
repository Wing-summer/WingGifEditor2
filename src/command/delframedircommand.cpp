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

#include "delframedircommand.h"

DelFrameDirCommand::DelFrameDirCommand(GifContentModel *model, int index,
                                       DelDirection dir, QUndoCommand *parent)
    : UndoCommand(model, parent), _oldindex(index), _olddir(dir) {
    Q_ASSERT(index >= 0 && index < model->frameCount());
}

void DelFrameDirCommand::undo() {
    auto gif = model();

    switch (_olddir) {
    case DelDirection::Before:
        gif->insertFrames(0, _data);
        break;
    case DelDirection::After:
        gif->insertFrames(gif->frameCount(), _data);
        break;
    }

    if (auto lv = gif->linkedListView()) {
        lv->clearSelection();
        lv->setCurrentIndex(gif->index(_oldindex));
    }
}

void DelFrameDirCommand::redo() {
    auto gif = model();

    switch (_olddir) {
    case DelDirection::Before:
        _data = gif->removeFrames(0, _oldindex);
        break;
    case DelDirection::After:
        _data = gif->removeFrames(_oldindex + 1, -1);
        break;
    }

    if (auto lv = gif->linkedListView()) {
        lv->clearSelection();

        if (gif->frameCount() > 0) {
            const auto target = _olddir == DelDirection::Before ? 0 : _oldindex;
            lv->setCurrentIndex(gif->index(target));
        }
    }
}
