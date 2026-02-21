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

#include "delayframecommand.h"

DelayFrameCommand::DelayFrameCommand(GifContentModel *model,
                                     const QVector<int> &indices, int delay,
                                     QUndoCommand *parent)
    : UndoCommand(model, parent), _newdelay(delay) {
    if (indices.isEmpty()) {
        auto total = model->frameCount();
        for (qsizetype i = 0; i < total; ++i) {
            _olddelay.insert(i, model->delay(i));
        }
    } else {
        for (auto &i : indices) {
            _olddelay.insert(i, model->delay(i));
        }
    }
}

void DelayFrameCommand::undo() {
    auto gif = model();
    for (auto &&[idx, delay] : _olddelay.asKeyValueRange()) {
        gif->setFrameDelay(idx, delay);
    }
}

void DelayFrameCommand::redo() {
    auto gif = model();
    auto total = gif->frameCount();
    for (qsizetype i = 0; i < total; ++i) {
        gif->setFrameDelay(i, _newdelay);
    }
}

//=================================================

DelayScaleCommand::DelayScaleCommand(GifContentModel *model,
                                     const QVector<int> &indices, int scale,
                                     QUndoCommand *parent)
    : UndoCommand(model, parent) {
    if (indices.isEmpty()) {
        auto total = model->frameCount();
        for (qsizetype i = 0; i < total; ++i) {
            auto time = model->delay(i);
            _olddelay.insert(i, time);
            time = qBound(GifFile::delayLimitMin(), time * scale / 100,
                          GifFile::delayLimitMax());
            _newdelay.insert(i, time);
        }
    } else {
        for (auto &i : indices) {
            auto time = model->delay(i);
            _olddelay.insert(i, time);
            time = qBound(GifFile::delayLimitMin(), time * scale / 100,
                          GifFile::delayLimitMax());
            _newdelay.insert(i, time);
        }
    }
}

void DelayScaleCommand::undo() {
    auto gif = model();
    for (auto &&[idx, delay] : _olddelay.asKeyValueRange()) {
        gif->setFrameDelay(idx, delay);
    }
}

void DelayScaleCommand::redo() {
    auto gif = model();
    for (auto &&[idx, delay] : _newdelay.asKeyValueRange()) {
        gif->setFrameDelay(idx, delay);
    }
}
