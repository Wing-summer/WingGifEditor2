#ifndef WAITINGLOOP_H
#define WAITINGLOOP_H

#include "class/wingprogressdialog.h"

#include <QObject>
#include <QString>

class WaitingLoop : public QObject {
    Q_OBJECT
public:
    WaitingLoop(const QString &tip);

    ~WaitingLoop();

private:
    WingProgressDialog dw;
};

#endif // WAITINGLOOP_H
