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

#ifndef DELAYFRAMECOMMAND_H
#define DELAYFRAMECOMMAND_H

#include "undocommand.h"

class DelayFrameCommand : public UndoCommand {
public:
    DelayFrameCommand(GifContentModel *model, const QVector<int> &indices,
                      int delay, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    int _newdelay;
    QMap<int, int> _olddelay;
};

class DelayScaleCommand : public UndoCommand {
public:
    DelayScaleCommand(GifContentModel *model, const QVector<int> &indices,
                      int scale, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    QMap<int, int> _olddelay, _newdelay;
};

#endif // DELAYFRAMECOMMAND_H
