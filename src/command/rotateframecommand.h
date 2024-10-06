#ifndef ROTATEFRAMECOMMAND_H
#define ROTATEFRAMECOMMAND_H

#include "class/gifcontentmodel.h"
#include <QUndoCommand>

class RotateFrameCommand : public QUndoCommand {
public:
    RotateFrameCommand(GifContentModel *helper, bool isclockwise,
                       QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    GifContentModel *gif;
    bool clockwise;
};

#endif // ROTATEFRAMECOMMAND_H
