#include "logdialog.h"

LogDialog::LogDialog(QWidget *parent) : FramelessDialogBase(parent) {
    _log = new QTextBrowser(this);
    _log->setFocusPolicy(Qt::StrongFocus);
    _log->setOpenExternalLinks(true);
    _log->setUndoRedoEnabled(false);

    setWindowTitle(tr("Log"));

    buildUpContent(_log);
}

void LogDialog::log(const QString &message) { _log->append(message); }
