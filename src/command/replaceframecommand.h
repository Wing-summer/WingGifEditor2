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

#ifndef REPLACEFRAMECOMMAND_H
#define REPLACEFRAMECOMMAND_H

#include "class/gifcontentmodel.h"
#include <QUndoCommand>

class ReplaceFrameCommand : public QUndoCommand {
public:
    ReplaceFrameCommand(GifContentModel *helper, const QVector<int> &indices,
                        const QVector<QImage> &newimgs,
                        QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    GifContentModel *gif;
    QVector<int> olds;
    QVector<QImage> bufferimage;
};

#endif // REPLACEFRAMECOMMAND_H
