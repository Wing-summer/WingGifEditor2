#ifndef REMOVEFRAMECOMMAND_H
#define REMOVEFRAMECOMMAND_H

#include "class/gifcontentmodel.h"
#include <QMap>
#include <QUndoCommand>

class RemoveFrameCommand : public QUndoCommand {
public:
    RemoveFrameCommand(GifContentModel *helper, QVector<int> &frames,
                       QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

protected:
    GifContentModel *gif;
    QVector<int> indices;
    QVector<GifData> imgs;
};

#endif // REMOVEFRAMECOMMAND_H
