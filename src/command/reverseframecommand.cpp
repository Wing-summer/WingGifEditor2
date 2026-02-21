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

#include "reverseframecommand.h"

ReverseFrameCommand::ReverseFrameCommand(GifContentModel *model,
                                         qsizetype begin, qsizetype end,
                                         QUndoCommand *parent)
    : UndoCommand(model, parent), _begin(begin), _end(end) {}

void ReverseFrameCommand::undo() {
    auto gif = model();
    gif->reverseFrames(_begin, _end);
}

void ReverseFrameCommand::redo() {
    auto gif = model();
    gif->reverseFrames(_begin, _end);
}
