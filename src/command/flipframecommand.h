#ifndef FLIPFRAMECOMMAND_H
#define FLIPFRAMECOMMAND_H

#include "class/gifcontentmodel.h"
#include <QUndoCommand>

class FlipFrameCommand : public QUndoCommand {
public:
    FlipFrameCommand(GifContentModel *helper, Qt::Orientation dir,
                     QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    GifContentModel *gif;
    Qt::Orientation olddir;
};

#endif // FLIPFRAMECOMMAND_H
