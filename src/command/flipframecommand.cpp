#include "flipframecommand.h"

FlipFrameCommand::FlipFrameCommand(GifContentModel *helper, Qt::Orientation dir,
                                   QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper), olddir(dir) {}

void FlipFrameCommand::undo() {
    gif->flipFrames(olddir, 0);
    // force update
    auto i = gif->linkedListView()->currentIndex();
    emit gif->linkedListView()->selectionModel()->currentRowChanged(i, i);
}

void FlipFrameCommand::redo() {
    gif->flipFrames(olddir, 0);
    // force update
    auto i = gif->linkedListView()->currentIndex();
    emit gif->linkedListView()->selectionModel()->currentRowChanged(i, i);
}
