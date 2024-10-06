#include "delframedircommand.h"

DelFrameDirCommand::DelFrameDirCommand(GifContentModel *helper, int index,
                                       DelDirection dir, QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper), oldindex(index), olddir(dir) {
    auto imgs = helper->images();
    auto delays = helper->delays();

    switch (olddir) {
    case DelDirection::Before: {
        oldimgs = imgs.mid(0, index);
        olddelays = delays.mid(0, index);
        break;
    }
    case DelDirection::After: {
        oldimgs = imgs.mid(index + 1);
        olddelays = delays.mid(index + 1);
        break;
    }
    }
}

void DelFrameDirCommand::undo() {
    switch (olddir) {
    case DelDirection::Before: {
        gif->insertFrames(oldimgs, olddelays, 0);
        gif->linkedListView()->clearSelection();
        gif->linkedListView()->setCurrentIndex(gif->index(oldindex));
        break;
    }
    case DelDirection::After: {
        gif->insertFrames(oldimgs, olddelays, -1);
        gif->linkedListView()->clearSelection();
        gif->linkedListView()->setCurrentIndex(gif->index(oldindex));
        break;
    }
    }
}

void DelFrameDirCommand::redo() {
    switch (olddir) {
    case DelDirection::Before: {
        gif->removeFrames(0, oldindex);
        gif->linkedListView()->clearSelection();
        gif->linkedListView()->setCurrentIndex(gif->index(0));
        break;
    }
    case DelDirection::After: {
        gif->removeFrames(oldindex + 1, -1);
        gif->linkedListView()->clearSelection();
        gif->linkedListView()->setCurrentIndex(gif->index(oldindex));
        break;
    }
    }
}
