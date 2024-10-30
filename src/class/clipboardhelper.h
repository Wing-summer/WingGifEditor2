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

#ifndef CLIPBOARDHELPER_H
#define CLIPBOARDHELPER_H

#include <QImage>
#include <QObject>

#include "utilities.h"

class ClipBoardHelper : public QObject {
    Q_OBJECT
public:
    static void setImageFrames(const QVector<GifData> &frames);
    static void getImageFrames(QVector<GifData> &frames);
};

#endif // CLIPBOARDHELPER_H
