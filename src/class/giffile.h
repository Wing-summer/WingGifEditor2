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

#ifndef GIFFILE_H
#define GIFFILE_H

#include "giflib/gif_lib.h"

#include "gifframe.h"

#include <QImage>
#include <QCache>
#include <QObject>
#include <QString>
#include <QTemporaryDir>
#include <QVector>

class GifFile : public QObject {
    Q_OBJECT

    Q_DISABLE_COPY(GifFile)

public:
    enum class ErrorCode {
        SUCCEEDED = D_GIF_SUCCEEDED,
        ERR_OPEN_FAILED = D_GIF_ERR_OPEN_FAILED,
        ERR_READ_FAILED = D_GIF_ERR_READ_FAILED,
        ERR_NOT_GIF_FILE = D_GIF_ERR_NOT_GIF_FILE,
        ERR_NO_SCRN_DSCR = D_GIF_ERR_NO_SCRN_DSCR,
        ERR_NO_IMAG_DSCR = D_GIF_ERR_NO_IMAG_DSCR,
        ERR_NO_COLOR_MAP = D_GIF_ERR_NO_COLOR_MAP,
        ERR_WRONG_RECORD = D_GIF_ERR_WRONG_RECORD,
        ERR_DATA_TOO_BIG = D_GIF_ERR_DATA_TOO_BIG,
        ERR_NOT_ENOUGH_MEM = D_GIF_ERR_NOT_ENOUGH_MEM,
        ERR_CLOSE_FAILED = D_GIF_ERR_CLOSE_FAILED,
        ERR_NOT_READABLE = D_GIF_ERR_NOT_READABLE,
        ERR_IMAGE_DEFECT = D_GIF_ERR_IMAGE_DEFECT,
        ERR_EOF_TOO_SOON = D_GIF_ERR_EOF_TOO_SOON
    };

    using value_type = QSharedPointer<GifFrame>;
    using data_type = QVector<value_type>;
    using const_iterator = data_type::const_iterator;
    using iterator = data_type::iterator;

public:
    explicit GifFile(QObject *parent = nullptr);

    virtual ~GifFile();

    bool isNull() const;

    int width() const;

    int height() const;

    QSize size() const;

    ErrorCode load(const QString &filename);

    void clear();

    int delay(qsizetype index) const;

    QImage image(qsizetype index) const;

    QImage imageWithoutCache(qsizetype index) const;

    qsizetype frameCount() const;

    QString comment() const;

public:
    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;

    const_iterator constBegin() const noexcept;
    const_iterator constEnd() const noexcept;

    QSharedPointer<GifFrame> frame(qsizetype index) const;

    void replaceFrame(qsizetype index, const QSharedPointer<GifFrame> &frame);

public:
    bool able2InsertFrames(qsizetype count);

    static bool isValidGifFrame(const QImage &image, int delay);

    QSharedPointer<GifFrame> generateFrame(const QImage &image, int delay);

    void insertFrame(qsizetype index, const QSharedPointer<GifFrame> &frame);

    void insertFrames(qsizetype index,
                      const QVector<QSharedPointer<GifFrame>> &frames);

    QSharedPointer<GifFrame> removeFrame(qsizetype index);

    QVector<QSharedPointer<GifFrame>> removeFrames(qsizetype index,
                                                   qsizetype count);

    void removeFrameIf(
        const std::function<bool(qsizetype, QSharedPointer<GifFrame>)> &fn);

    QPair<QSize, QVector<QSharedPointer<GifFrame>>>
    replaceFrames(const QSize &frameSize,
                  const QVector<QSharedPointer<GifFrame>> &frames);

signals:
    void sigUpdateUIProcess();

private:
    QString getFrameString(qsizetype index) const;
    QString getFrameString(const QSharedPointer<GifFrame> &frame) const;

private:
    ErrorCode closeHandleWithError(GifFileType *handle);

    ErrorCode closeHandle(GifFileType *handle);

public:
    static int delayLimitMin();

    static int delayLimitMax();

private:
    QSize m_frameSize;
    QVector<QSharedPointer<GifFrame>> m_framesData;
    mutable QCache<QString, QImage> m_frameCache;

    QScopedPointer<QTemporaryDir> m_tmpDir;
    QString m_comment;

    QStringView m_lastError;
};

#endif // GIFFILE_H
