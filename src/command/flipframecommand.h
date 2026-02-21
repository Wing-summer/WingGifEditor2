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

#ifndef FLIPFRAMECOMMAND_H
#define FLIPFRAMECOMMAND_H

#include "undocommand.h"

class FlipFrameCommand : public UndoCommand {
public:
    explicit FlipFrameCommand(GifContentModel *model,
                              const QVector<int> &indices, Qt::Orientation dir,
                              QUndoCommand *parent = nullptr);

public:
    void undo() override;
    void redo() override;

private:
    Qt::Orientation _dir;
    QMap<int, QSharedPointer<GifFrame>> _oldcache, _newcache;
};

#endif // FLIPFRAMECOMMAND_H
