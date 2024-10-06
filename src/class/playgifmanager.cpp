#include "playgifmanager.h"

PlayGifManager::PlayGifManager(QObject *parent) : QObject(parent) {}

void PlayGifManager::setTickIntervals(const QVector<int> &intervals) {
    m_intervals = intervals;
}

void PlayGifManager::play(int index) {
    if (index < 0 || index >= m_intervals.count())
        return;
    curpos = index;
    _isContinue = true;
    emit playStateChanged();
    QTimer::singleShot(m_intervals.at(curpos), this, &PlayGifManager::tick_p);
}

void PlayGifManager::stop() { _isContinue = false; }

bool PlayGifManager::isPlaying() const { return _isContinue; }

void PlayGifManager::tick_p() {
    emit tick(curpos++);
    if (curpos >= m_intervals.count()) {
        curpos = 0;
    }
    if (_isContinue) {
        QTimer::singleShot(m_intervals.at(curpos), this,
                           &PlayGifManager::tick_p);
    } else {
        emit playStateChanged();
    }
}
