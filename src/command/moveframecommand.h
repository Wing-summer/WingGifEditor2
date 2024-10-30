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

#ifndef MOVEFRAMECOMMAND_H
#define MOVEFRAMECOMMAND_H

#include "class/gifcontentmodel.h"
#include <QListWidget>
#include <QUndoCommand>

class MoveFrameCommand : public QUndoCommand {
public:
    MoveFrameCommand(GifContentModel *helper,
                     GifContentModel::MoveFrameDirection direction,
                     const QVector<int> &indices,
                     QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

protected:
    GifContentModel *gif;
    GifContentModel::MoveFrameDirection dir;
    QVector<int> oldindices;
};

#endif // MOVEFRAMECOMMAND_H
