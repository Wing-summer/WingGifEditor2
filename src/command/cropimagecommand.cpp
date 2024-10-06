#include "cropimagecommand.h"

CropImageCommand::CropImageCommand(GifContentModel *helper, const QRect &rect,
                                   QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper) {
    _rect = rect;
    buffer = helper->images();
}

void CropImageCommand::undo() { gif->swapFrames(buffer); }

void CropImageCommand::redo() { gif->cropFrames(_rect); }
