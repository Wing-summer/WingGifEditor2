#ifndef FILEINFODIALOG_H
#define FILEINFODIALOG_H

#include "framelessdialogbase.h"

class FileInfoDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    FileInfoDialog(const QString &filename, const QSize &gifSize,
                   const QString &comment, QWidget *parent = nullptr);
    virtual ~FileInfoDialog();
};

#endif // FILEINFODIALOG_H
