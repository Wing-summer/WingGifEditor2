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
