#ifndef REDUCEFRAMECOMMAND_H
#define REDUCEFRAMECOMMAND_H

#include "class/gifcontentmodel.h"
#include <QMap>
#include <QUndoCommand>

class ReduceFrameCommand : public QUndoCommand {
public:
    ReduceFrameCommand(GifContentModel *helper, const QVector<int> &indices,
                       const QVector<int> &interval,
                       QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    GifContentModel *gif;
    QVector<int> inter, oldinter;
    QMap<int, QImage> oldimgs;
};

#endif // REDUCEFRAMECOMMAND_H
