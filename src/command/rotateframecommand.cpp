#include "rotateframecommand.h"

RotateFrameCommand::RotateFrameCommand(GifContentModel *helper,
                                       bool isclockwise, QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper), clockwise(isclockwise) {}

void RotateFrameCommand::undo() { gif->rotateFrames(!clockwise); }

void RotateFrameCommand::redo() { gif->rotateFrames(clockwise); }
