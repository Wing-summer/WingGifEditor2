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

#include "scaleframecommand.h"

ScaleFrameCommand::ScaleFrameCommand(GifContentModel *model, int w, int h,
                                     QUndoCommand *parent)
    : UndoCommand(model, parent), _w(w), _h(h) {}

void ScaleFrameCommand::undo() {
    auto gif = model();
    gif->replaceFrames(_cached.olddata.first, _cached.olddata.second);
}

void ScaleFrameCommand::redo() {
    auto gif = model();
    if (_cached.isValid()) {
        gif->replaceFrames(_cached.newdata.first, _cached.newdata.second);
    } else {
        _cached = gif->scaleFrames(_w, _h);
    }
}
