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

#include "delayframecommand.h"

DelayFrameCommand::DelayFrameCommand(GifContentModel *helper,
                                     QVector<int> &indices, int delay,
                                     QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper), newdelay(delay), oldindices(indices) {
    if (indices.isEmpty()) {
        olddelay = gif->delays();
    } else {
        for (auto &item : indices) {
            olddelay.append(gif->delay(item));
        }
    }
}

void DelayFrameCommand::undo() {
    auto len = oldindices.count();
    if (len == 0) {
        len = gif->frameCount();
        for (auto i = 0; i < len; i++) {
            gif->setFrameDelay(i, olddelay.at(i));
        }
    } else {
        for (auto i = 0; i < len; i++) {
            gif->setFrameDelay(oldindices.at(i), olddelay.at(i));
        }
    }
}

void DelayFrameCommand::redo() {
    if (oldindices.isEmpty()) {
        gif->setFrameDelay(0, newdelay, -1);
    } else {
        for (auto item : oldindices) {
            gif->setFrameDelay(item, newdelay);
        }
    }
}

//=================================================

DelayScaleCommand::DelayScaleCommand(GifContentModel *helper,
                                     QVector<int> &indices, int scale,
                                     QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper), oldindices(indices) {
    if (indices.isEmpty()) {
        for (auto &time : gif->delays()) {
            olddelay.append(time);
            time = time * scale / 100;
            newdelay.append(time);
        }

    } else {
        for (auto i : indices) {
            auto time = gif->delay(i);
            olddelay.append(time);
            time = time * scale / 100;
            newdelay.append(time);
        }
    }
}

void DelayScaleCommand::undo() {
    auto len = oldindices.count();
    if (len) {
        for (auto i = 0; i < len; i++) {
            gif->setFrameDelay(oldindices[i], olddelay[i]);
        }
    } else {
        len = gif->frameCount();
        for (auto i = 0; i < len; i++) {
            gif->setFrameDelay(i, olddelay[i]);
        }
    }
}

void DelayScaleCommand::redo() {
    auto len = oldindices.count();
    if (len) {
        for (auto i = 0; i < len; i++) {
            gif->setFrameDelay(oldindices[i], newdelay[i]);
        }
    } else {
        len = gif->frameCount();
        for (auto i = 0; i < len; i++) {
            gif->setFrameDelay(i, newdelay[i]);
        }
    }
}
