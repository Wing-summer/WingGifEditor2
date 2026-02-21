/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#ifndef GIFFRAME_H
#define GIFFRAME_H

#include "qtlockedfile.h"

#include <QImage>
#include <QScopedPointer>

class GifFrame : public QtLockedFile {
    Q_OBJECT

public:
    explicit GifFrame(const QImage &img, int delay, const QString &cachePath);
    virtual ~GifFrame();

public:
    QImage image() const;

public:
    bool isValidFrame() const;

    int delay() const;
    void setDelay(int newDelay);

private:
    int _delay = 0;
};

#endif // GIFFRAME_H
