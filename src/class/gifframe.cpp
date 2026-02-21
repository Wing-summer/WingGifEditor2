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

#include "gifframe.h"

GifFrame::GifFrame(const QImage &img, int delay, const QString &cachePath)
    : QtLockedFile(cachePath) {
    setDelay(delay);
    if (open(ReadWrite)) {
        if (!img.save(this, "PNG")) {
            close();
            return;
        }
        if (!lock(WriteLock)) {
            close();
            remove();
            return;
        }
    }
}

GifFrame::~GifFrame() {
    unlock();
    close();
    remove();
}

QImage GifFrame::image() const {
    auto self = const_cast<GifFrame *>(this);
    self->reset();
    QImage img;
    img.load(self, "PNG");
    return img;
}

bool GifFrame::isValidFrame() const { return isOpen() && _delay > 0; }

int GifFrame::delay() const { return _delay; }

void GifFrame::setDelay(int newDelay) {
    Q_ASSERT(newDelay > 0);
    auto div = std::div(newDelay, 10);
    _delay = (div.quot + (div.quot >= 5 ? 1 : 0)) * 10;
}
