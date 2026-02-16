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

#ifndef GIFREADER_H
#define GIFREADER_H

#include "giflib/gif_lib.h"

#include <QImage>
#include <QCache>
#include <QObject>
#include <QString>
#include <QTemporaryDir>
#include <QVector>

#include <memory>

class GifReader : public QObject {
    Q_OBJECT

public:
    explicit GifReader(const QString &filename = QString(),
                       QObject *parent = nullptr);

    virtual ~GifReader();

    bool isNull() const;

    int width() const;

    int height() const;

    bool load(const QString &filename);

    int delay(qsizetype index) const;

    QImage image(qsizetype index) const;

    const QVector<int> delays() const;

    const QVector<QImage> images() const;

    QString frameFilePath(qsizetype index) const;

    qsizetype imageCount() const;

    QString comment() const;

signals:
    void sigUpdateUIProcess();

private:
    bool closeHandleWithError(GifFileType *handle);

    bool closeHandle(GifFileType *handle);

    void invalidCache();

    void clearCache();

private:
    qsizetype m_framesCount = -1;
    QSize m_frameSize;
    QVector<int> m_delays;
    QVector<QString> m_frameFiles;
    mutable QCache<qsizetype, QImage> m_frameCache;
    std::unique_ptr<QTemporaryDir> m_tmpDir;
    QString m_comment;
};

#endif // GIFREADER_H
