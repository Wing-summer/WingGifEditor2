#include "scaleframecommand.h"

ScaleFrameCommand::ScaleFrameCommand(GifContentModel *helper, int w, int h,
                                     QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper), _w(w), _h(h) {
    buffer = helper->images();
}

void ScaleFrameCommand::undo() { gif->swapFrames(buffer); }

void ScaleFrameCommand::redo() { gif->scaleFrames(_w, _h); }
