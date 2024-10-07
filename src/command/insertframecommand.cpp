#include "insertframecommand.h"

InsertFrameCommand::InsertFrameCommand(GifContentModel *model, int index,
                                       const QVector<GifData> &images,
                                       QUndoCommand *parent)
    : QUndoCommand(parent), gif(model), oldindex(index), oldimgs(images) {}

void InsertFrameCommand::undo() {
    gif->removeFrames(oldindex, oldimgs.size());
    auto lv = gif->linkedListView();
    lv->clearSelection();
    lv->setCurrentIndex(gif->index(oldindex));
}

void InsertFrameCommand::redo() {
    gif->insertFrames(oldimgs, oldindex);

    auto lv = gif->linkedListView();
    lv->clearSelection();
    lv->setCurrentIndex(gif->index(oldindex));
}
