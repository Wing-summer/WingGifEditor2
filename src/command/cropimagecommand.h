#ifndef CROPIMAGECOMMAND_H
#define CROPIMAGECOMMAND_H

#include "class/gifcontentmodel.h"
#include <QUndoCommand>

class CropImageCommand : public QUndoCommand {
public:
    CropImageCommand(GifContentModel *model, const QRect &rect,
                     QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    GifContentModel *gif;
    QVector<QImage> buffer;
    QRect _rect;
};

#endif // CROPIMAGECOMMAND_H
