#include "fileinfodialog.h"
#include "utilities.h"
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QMimeDatabase>
#include <QPicture>
#include <QTextBrowser>
#include <QVBoxLayout>

FileInfoDialog::FileInfoDialog(const QString &filename, const QSize &gifSize,
                               const QString &comment, QWidget *parent)
    : FramelessDialogBase(parent) {
    static const QString dfmt("yyyy/MM/dd hh:mm:ss ddd");

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    auto b = new QTextBrowser(this);

    if (!filename.isEmpty()) {
        QFileInfo finfo(filename);
        b->append(tr("FileName:") + finfo.fileName());
        b->append(tr("Size:") + QStringLiteral("%1 x %2")
                                    .arg(gifSize.width())
                                    .arg(gifSize.height()));
        b->append(tr("Comment:") + comment);
        b->append(tr("FilePath:") + finfo.filePath());
        b->append(tr("FileSize:") + Utilities::processBytesCount(finfo.size()));
        b->append(
            tr("FileBirthTime:") +
            finfo.fileTime(QFile::FileTime::FileBirthTime).toString(dfmt));
        b->append(
            tr("FileAccessTime:") +
            finfo.fileTime(QFile::FileTime::FileAccessTime).toString(dfmt));
        b->append(tr("FileModificationTime:") +
                  finfo.fileTime(QFile::FileTime::FileModificationTime)
                      .toString(dfmt));
        b->append(tr("LastRead:") + finfo.lastRead().toString(dfmt));
        b->append(tr("LastMod:") + finfo.lastModified().toString(dfmt));
    } else {
        b->append(tr("FileName:") + tr("<NewFile>"));
        b->append(tr("Size:") + (gifSize.isValid() ? QStringLiteral("%1 x %2")
                                                         .arg(gifSize.width())
                                                         .arg(gifSize.height())
                                                   : QStringLiteral("-")));
        b->append(tr("Comment:") + comment);
    }

    layout->addWidget(b);

    buildUpContent(widget);

    setWindowTitle(tr("FileInfo"));
    this->resize(500, 450);
}

FileInfoDialog::~FileInfoDialog() {}