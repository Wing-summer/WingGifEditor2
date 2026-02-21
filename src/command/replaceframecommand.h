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

#ifndef REPLACEFRAMECOMMAND_H
#define REPLACEFRAMECOMMAND_H

#include "undocommand.h"

class ReplaceFrameCommand : public UndoCommand {
public:
    explicit ReplaceFrameCommand(
        GifContentModel *model,
        const QMap<int, QSharedPointer<GifFrame>> &newimgs,
        QUndoCommand *parent = nullptr);

public:
    void undo() override;
    void redo() override;

private:
    QMap<int, QSharedPointer<GifFrame>> _cache;
};

#endif // REPLACEFRAMECOMMAND_H
