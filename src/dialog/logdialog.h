#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include "framelessdialogbase.h"

#include <QTextBrowser>

class LogDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    explicit LogDialog(QWidget *parent = nullptr);

public slots:
    void log(const QString &message);

private:
    QTextBrowser *_log;
};

#endif // LOGDIALOG_H
