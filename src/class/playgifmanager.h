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

#ifndef PLAYGIFMANAGER_H
#define PLAYGIFMANAGER_H

#include <QObject>
#include <QTimer>

class PlayGifManager : public QObject {
    Q_OBJECT
public:
    explicit PlayGifManager(QObject *parent = nullptr);
    void setTickIntervals(const std::function<int(qsizetype)> &delegent);
    void play(int index);
    void stop();

    bool isPlaying() const;

signals:
    void tick(int index);

    void playStateChanged();

private:
    void tick_p();

private:
    int curpos = 0;
    bool _isContinue = false;

    std::function<int(qsizetype)> _delegent;
};

#endif // PLAYGIFMANAGER_H
