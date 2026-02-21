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

#ifndef REDUCEFRAMECOMMAND_H
#define REDUCEFRAMECOMMAND_H

#include "undocommand.h"

class ReduceFrameCommand : public UndoCommand {
public:
    explicit ReduceFrameCommand(GifContentModel *model, qsizetype begin,
                                qsizetype end, qsizetype step,
                                QUndoCommand *parent = nullptr);

public:
    void undo() override;
    void redo() override;

private:
    QMap<int, std::variant<QPair<int, int>, QSharedPointer<GifFrame>>> _cache;
};

#endif // REDUCEFRAMECOMMAND_H
