#include "replaceframecommand.h"

ReplaceFrameCommand::ReplaceFrameCommand(GifContentModel *helper,
                                         const QVector<int> &indices,
                                         const QVector<QImage> &newimgs,
                                         QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper), olds(indices), bufferimage(newimgs) {}

void ReplaceFrameCommand::undo() {
    auto len = olds.size();
    if (len) {
        for (auto i = 0; i < len; i++) {
            auto index = olds.at(i);
            auto img = gif->image(index);
            gif->setFrameImage(index, bufferimage.at(i));
            bufferimage.replace(index, img);
        }
    } else {
        len = bufferimage.size();
        for (auto i = 0; i < len; i++) {
            auto img = gif->image(i);
            gif->setFrameImage(i, bufferimage.at(i));
            bufferimage.replace(i, img);
        }
    }
}

void ReplaceFrameCommand::redo() {
    auto len = olds.size();
    if (len) {
        for (auto i = 0; i < len; i++) {
            auto index = olds.at(i);
            auto img = gif->image(index);
            gif->setFrameImage(index, bufferimage.at(i));
            bufferimage.replace(index, img);
        }
    } else {
        len = bufferimage.size();
        for (auto i = 0; i < len; i++) {
            auto img = gif->image(i);
            gif->setFrameImage(i, bufferimage.at(i));
            bufferimage.replace(i, img);
        }
    }
}
