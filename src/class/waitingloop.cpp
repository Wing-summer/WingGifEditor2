#include "waitingloop.h"

#include <QApplication>

WaitingLoop::WaitingLoop(const QString &tip) {
    dw.dialog()->setCancelButton(nullptr);
    dw.dialog()->setLabelText(tip);
    dw.dialog()->setRange(0, 0);
    dw.pdialog()->setModal(true);
    dw.pdialog()->setWindowTitle(tr("Waiting"));
    dw.pdialog()->show();
}

WaitingLoop::~WaitingLoop() { dw.pdialog()->close(); }
