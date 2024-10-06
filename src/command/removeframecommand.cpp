#include "removeframecommand.h"

RemoveFrameCommand::RemoveFrameCommand(GifContentModel *helper,
                                       QVector<int> &frames,
                                       QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper) {
    indices = frames;
    std::sort(indices.begin(), indices.end(), std::greater<int>());

    for (auto i : indices) {
        GifData d;
        d.delay = gif->delay(i);
        d.image = gif->image(i);
        imgs.append(d);
    }
}

void RemoveFrameCommand::undo() {
    for (int i = 0; i < indices.size(); ++i) {
        gif->insertFrame(imgs.at(i), indices.at(i));
    }

    auto l = indices.last();
    gif->linkedListView()->setCurrentIndex(gif->index(l));
}

void RemoveFrameCommand::redo() {
    for (auto p = indices.rbegin(); p < indices.rend(); p++)
        gif->removeFrames(*p);

    auto l = indices.last();
    auto c = gif->frameCount();
    if (l > c)
        l = c;
    gif->linkedListView()->setCurrentIndex(gif->index(l));
}
