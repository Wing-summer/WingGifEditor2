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

#ifndef DELFRAMEDIRCOMMAND_H
#define DELFRAMEDIRCOMMAND_H

#include "undocommand.h"

enum class DelDirection { Before, After };

class DelFrameDirCommand : public UndoCommand {
public:
    explicit DelFrameDirCommand(GifContentModel *model, int index,
                                DelDirection dir,
                                QUndoCommand *parent = nullptr);

public:
    void undo() override;
    void redo() override;

private:
    int _oldindex;
    DelDirection _olddir;
    QVector<QSharedPointer<GifFrame>> _data;
};

#endif // DELFRAMEDIRCOMMAND_H
