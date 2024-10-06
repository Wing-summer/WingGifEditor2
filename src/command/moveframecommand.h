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
