#include "reverseframecommand.h"

ReverseFrameCommand::ReverseFrameCommand(GifContentModel *helper,
                                         QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper) {}

void ReverseFrameCommand::undo() { gif->reverseFrames(); }

void ReverseFrameCommand::redo() { gif->reverseFrames(); }
