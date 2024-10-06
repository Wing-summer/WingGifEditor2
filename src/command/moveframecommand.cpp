#include "moveframecommand.h"

MoveFrameCommand::MoveFrameCommand(
    GifContentModel *helper, GifContentModel::MoveFrameDirection direction,
    const QVector<int> &indices, QUndoCommand *parent)
    : QUndoCommand(parent), gif(helper), dir(direction), oldindices(indices) {
    std::sort(oldindices.begin(), oldindices.end());
}

void MoveFrameCommand::undo() {
    auto lv = gif->linkedListView();
    auto cur = lv->currentIndex().row();
    auto sel = lv->selectionModel();
    sel->clearSelection();

    switch (dir) {
    case GifContentModel::MoveFrameDirection::Left: {
        for (auto pi = oldindices.crbegin(); pi != oldindices.crend(); ++pi) {
            gif->moveFrames((*pi) - 1,
                            GifContentModel::MoveFrameDirection::Right);
            sel->select(gif->index(*pi), QItemSelectionModel::Select);
        }
        sel->select(gif->index(cur + 1), QItemSelectionModel::Current);
        break;
    }
    case GifContentModel::MoveFrameDirection::Right: {
        for (auto pi = oldindices.cbegin(); pi != oldindices.cend(); ++pi) {
            gif->moveFrames((*pi) + 1,
                            GifContentModel::MoveFrameDirection::Left);
            sel->select(gif->index(*pi), QItemSelectionModel::Select);
        }
        sel->select(gif->index(cur - 1), QItemSelectionModel::Current);
        break;
    }
    }
}

void MoveFrameCommand::redo() {
    auto lv = gif->linkedListView();
    auto cur = lv->currentIndex().row();
    auto sel = lv->selectionModel();
    sel->clearSelection();

    switch (dir) {
    case GifContentModel::MoveFrameDirection::Left: {
        for (auto pi = oldindices.cbegin(); pi != oldindices.cend(); ++pi) {
            gif->moveFrames(*pi, dir);
            sel->select(gif->index((*pi) - 1), QItemSelectionModel::Select);
        }
        sel->select(gif->index(cur - 1), QItemSelectionModel::Current);
        break;
    }
    case GifContentModel::MoveFrameDirection::Right: {
        for (auto pi = oldindices.crbegin(); pi != oldindices.crend(); ++pi) {
            gif->moveFrames(*pi, dir);
            sel->select(gif->index((*pi) + 1), QItemSelectionModel::Select);
        }
        sel->select(gif->index(cur + 1), QItemSelectionModel::Current);
        break;
    }
    }
}
