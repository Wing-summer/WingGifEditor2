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
