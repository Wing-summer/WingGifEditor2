#ifndef REVERSEFRAMECOMMAND_H
#define REVERSEFRAMECOMMAND_H

#include "class/gifcontentmodel.h"
#include <QUndoCommand>

class ReverseFrameCommand : public QUndoCommand {
public:
    ReverseFrameCommand(GifContentModel *helper,
                        QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    GifContentModel *gif;
};

#endif // REVERSEFRAMECOMMAND_H
