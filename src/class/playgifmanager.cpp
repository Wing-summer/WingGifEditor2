/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#include "playgifmanager.h"

PlayGifManager::PlayGifManager(QObject *parent) : QObject(parent) {}

void PlayGifManager::setTickIntervals(
    const std::function<int(qsizetype)> &delegent) {
    _delegent = delegent;
}

void PlayGifManager::play(int index) {
    if (!_delegent) {
        return;
    }
    curpos = index;
    _isContinue = true;
    emit playStateChanged();
    QTimer::singleShot(_delegent(curpos), this, &PlayGifManager::tick_p);
}

void PlayGifManager::stop() { _isContinue = false; }

bool PlayGifManager::isPlaying() const { return _isContinue; }

void PlayGifManager::tick_p() {
    emit tick(curpos++);
    if (_delegent(curpos) < 0) {
        curpos = 0;
    }
    if (_isContinue) {
        QTimer::singleShot(_delegent(curpos), this, &PlayGifManager::tick_p);
    } else {
        emit playStateChanged();
    }
}
