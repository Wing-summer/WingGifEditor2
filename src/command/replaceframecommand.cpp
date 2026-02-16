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

#include "replaceframecommand.h"

ReplaceFrameCommand::ReplaceFrameCommand(GifContentModel *helper,
                                         const QVector<int> &indices,
                                         const QVector<QImage> &newimgs,
                                         QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper), olds(indices), bufferimage(newimgs) {}

void ReplaceFrameCommand::undo() {
    auto len = olds.size();
    if (len) {
        len = qMin(len, bufferimage.size());
        for (int i = 0; i < len; ++i) {
            const auto index = olds.at(i);
            if (index < 0 || index >= gif->frameCount()) {
                continue;
            }
            const auto img = gif->image(index);
            gif->setFrameImage(index, bufferimage.at(i));
            bufferimage.replace(i, img);
        }
    } else {
        len = qMin(bufferimage.size(), static_cast<int>(gif->frameCount()));
        for (int i = 0; i < len; ++i) {
            const auto img = gif->image(i);
            gif->setFrameImage(i, bufferimage.at(i));
            bufferimage.replace(i, img);
        }
    }
}

void ReplaceFrameCommand::redo() { undo(); }
