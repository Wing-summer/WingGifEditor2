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

#include "reduceframecommand.h"

ReduceFrameCommand::ReduceFrameCommand(GifContentModel *helper,
                                       const QVector<int> &indices,
                                       const QVector<int> &interval,
                                       QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper), inter(interval) {
    oldinter = gif->delays();

    for (auto &ind : indices) {
        oldimgs.insert(ind, gif->image(ind));
    }
}

void ReduceFrameCommand::undo() {
    for (auto p = oldimgs.constKeyValueBegin(); p != oldimgs.constKeyValueEnd();
         p++) {
        gif->insertFrame(p->second, 1, p->first); // real time will be set later
    }

    auto len = oldinter.count();
    for (auto i = 0; i < len; i++) {
        gif->setFrameDelay(i, oldinter.at(i));
    }
}

void ReduceFrameCommand::redo() {
    QMapIterator<int, QImage> it(oldimgs);
    it.toBack();
    while (it.hasPrevious()) {
        it.previous();
        gif->removeFrames(it.key());
    }

    auto len = inter.count();
    for (auto i = 0; i < len; i++) {
        gif->setFrameDelay(i, inter.at(i));
    }
}
