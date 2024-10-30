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

#ifndef DELAYFRAMECOMMAND_H
#define DELAYFRAMECOMMAND_H

#include "class/gifcontentmodel.h"
#include <QUndoCommand>

class DelayFrameCommand : public QUndoCommand {
public:
    DelayFrameCommand(GifContentModel *helper, QVector<int> &indices, int delay,
                      QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    GifContentModel *gif;
    int newdelay;
    QVector<int> olddelay, oldindices;
};

class DelayScaleCommand : public QUndoCommand {
public:
    DelayScaleCommand(GifContentModel *helper, QVector<int> &indices, int scale,
                      QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    GifContentModel *gif;
    QVector<int> olddelay, newdelay, oldindices;
};

#endif // DELAYFRAMECOMMAND_H
