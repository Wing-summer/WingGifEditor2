#ifndef DELFRAMEDIRCOMMAND_H
#define DELFRAMEDIRCOMMAND_H

#include "class/gifcontentmodel.h"
#include <QListWidget>
#include <QUndoCommand>

enum class DelDirection { Before, After };

class DelFrameDirCommand : public QUndoCommand {
public:
    DelFrameDirCommand(GifContentModel *helper, int index, DelDirection dir,
                       QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    GifContentModel *gif;
    int oldindex;
    DelDirection olddir;

    QVector<QImage> oldimgs;
    QVector<int> olddelays;
};

#endif // DELFRAMEDIRCOMMAND_H
