#ifndef INSERTFRAMECOMMAND_H
#define INSERTFRAMECOMMAND_H

#include <QUndoCommand>

#include "class/gifcontentmodel.h"
#include "utilities.h"

class InsertFrameCommand : public QUndoCommand {
public:
    InsertFrameCommand(GifContentModel *model, int index,
                       const QVector<GifData> &images,
                       QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

protected:
    GifContentModel *gif;
    int oldindex;
    QVector<GifData> oldimgs;
};

#endif // INSERTFRAMECOMMAND_H
