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

#ifndef INSERTFRAMECOMMAND_H
#define INSERTFRAMECOMMAND_H

#include "undocommand.h"

#include <functional>

class InsertFrameCommand : public UndoCommand {
public:
    explicit InsertFrameCommand(
        GifContentModel *model, int index,
        const std::function<QPair<int, QImage>(int index)> &imgProc, int total,
        QUndoCommand *parent = nullptr);

    explicit InsertFrameCommand(GifContentModel *model, int index,
                                const QVector<QSharedPointer<GifFrame>> &data,
                                QUndoCommand *parent = nullptr);

public:
    void undo() override;
    void redo() override;

protected:
    int _oldindex;
    QVector<QSharedPointer<GifFrame>> _data;
};

#endif // INSERTFRAMECOMMAND_H
