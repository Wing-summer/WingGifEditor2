#ifndef SCALEFRAMECOMMAND_H
#define SCALEFRAMECOMMAND_H

#include "class/gifcontentmodel.h"
#include <QUndoCommand>

class ScaleFrameCommand : public QUndoCommand {
public:
    ScaleFrameCommand(GifContentModel *helper, int w, int h,
                      QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    GifContentModel *gif;
    QVector<QImage> buffer;
    int _w, _h;
};

#endif // SCALEFRAMECOMMAND_H
