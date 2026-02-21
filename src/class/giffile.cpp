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

#include "giffile.h"

#include <QDir>
#include <QFile>
#include <QPainter>
#include <QTemporaryFile>

#ifdef _WIN32
#include <io.h>
#define dup _dup
#else
#include <unistd.h>
#endif

constexpr auto GIF_FRAME_COUNT_LIMIT = 400;

GifFile::GifFile(QObject *parent) : QObject(parent), m_frameCache(48) {}

GifFile::~GifFile() { clear(); }

bool GifFile::isNull() const { return m_frameSize.isEmpty(); }

int GifFile::width() const {
    return m_frameSize.width();
}

int GifFile::height() const {
    return m_frameSize.height();
}

QSize GifFile::size() const { return m_frameSize; }

void GifFile::clear() {
    m_frameSize = {};
    m_framesData.clear();
    m_frameCache.clear();
    m_tmpDir.reset();
    m_comment.clear();
}

GifFile::ErrorCode GifFile::load(const QString &filename) {
    clear();

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return ErrorCode::ERR_OPEN_FAILED;
    }

    m_tmpDir.reset(new QTemporaryDir);
    if (!m_tmpDir->isValid()) {
        return ErrorCode::ERR_OPEN_FAILED;
    }

    int error;
    auto fhandle = dup(file.handle());
    auto handle = DGifOpenFileHandle(fhandle, &error);
    if (handle) {
        int animDelay = -1;
        int disposalMode = -1;
        int transparentIndex = -1;
        GifRecordType recordType;
        QImage key;

        m_frameSize = QSize(handle->SWidth, handle->SHeight);

        do {
            if (DGifGetRecordType(handle, &recordType) == GIF_ERROR)
                return closeHandleWithError(handle);

            switch (recordType) {
            case IMAGE_DESC_RECORD_TYPE: {
                if (DGifGetImageDesc(handle) == GIF_ERROR)
                    return closeHandleWithError(handle);

                int topRow = handle->Image.Top;
                int leftCol = handle->Image.Left;
                int width = handle->Image.Width;
                int height = handle->Image.Height;

                if (width <= 0 || height <= 0 || width > (INT_MAX / height) ||
                    leftCol + width > handle->SWidth ||
                    topRow + height > handle->SHeight)
                    return closeHandleWithError(handle);

                QImage img(width, height, QImage::Format_Indexed8);
                img.fill(handle->SBackGroundColor);

                if (handle->Image.Interlace) {
                    int InterlacedOffset[] = {0, 4, 2, 1};
                    int InterlacedJumps[] = {8, 8, 4, 2};

                    for (int i = 0; i < 4; ++i) {
                        for (int row = topRow + InterlacedOffset[i];
                             row < topRow + height; row += InterlacedJumps[i]) {
                            if (DGifGetLine(handle, img.scanLine(row), width) ==
                                GIF_ERROR)
                                return closeHandleWithError(handle);
                        }
                    }
                } else {
                    for (int row = 0; row < height; ++row) {
                        if (DGifGetLine(handle, img.scanLine(row), width) ==
                            GIF_ERROR)
                            return closeHandleWithError(handle);
                    }
                }

                ColorMapObject *cm =
                    (handle->Image.ColorMap ? handle->Image.ColorMap
                                            : handle->SColorMap);

                if (!cm)
                    return closeHandleWithError(handle);

                img.setColorCount(cm->ColorCount);

                for (int i = 0; i < cm->ColorCount; ++i) {
                    GifColorType gifColor = cm->Colors[i];
                    QRgb color = gifColor.Blue | (gifColor.Green << 8) |
                                 (gifColor.Red << 16);

                    if (i != transparentIndex)
                        color |= (0xFF << 24);

                    img.setColor(i, color);
                }

                if (key.isNull()) {
                    key = img.convertToFormat(QImage::Format_ARGB32);
                } else {
                    QImage tmp = key;

                    {
                        QPainter p(&tmp);
                        p.drawImage(leftCol, topRow, img);
                    }

                    img = tmp;

                    if (disposalMode != DISPOSE_PREVIOUS)
                        key = img;
                }

                QTemporaryFile tfile(
                    m_tmpDir->filePath(QStringLiteral("frame_XXXXXX.png")));
                tfile.setAutoRemove(false);
                if (!tfile.open()) {
                    return closeHandleWithError(handle);
                }
                auto f = QSharedPointer<GifFrame>::create(img, animDelay,
                                                          tfile.fileName());
                if (!f->isValidFrame()) {
                    return closeHandleWithError(handle);
                }
                m_framesData.append(f);
            } break;
            case EXTENSION_RECORD_TYPE: {
                GifByteType *extData;
                int extFunction;

                if (DGifGetExtension(handle, &extFunction, &extData) ==
                    GIF_ERROR)
                    return closeHandleWithError(handle);

                while (extData != nullptr) {
                    switch (extFunction) {
                    case GRAPHICS_EXT_FUNC_CODE: {
                        GraphicsControlBlock b;
                        DGifExtensionToGCB(extData[0], extData + 1, &b);
                        animDelay = b.DelayTime * 10;
                        disposalMode = b.DisposalMode;
                        transparentIndex = b.TransparentColor;
                    } break;
                    case COMMENT_EXT_FUNC_CODE: {
                        auto len = extData[0];
                        m_comment += QString::fromUtf8(
                            reinterpret_cast<const char *>(extData + 1), len);
                    }
                    default:
                        break;
                    }

                    if (DGifGetExtensionNext(handle, &extData) == GIF_ERROR)
                        return closeHandleWithError(handle);
                }
            } break;

            case TERMINATE_RECORD_TYPE:
                break;

            default:
                break;
            }

            emit sigUpdateUIProcess();
        } while (recordType != TERMINATE_RECORD_TYPE);

        return closeHandle(handle);
    } else {
        return ErrorCode(error);
    }
}

int GifFile::delay(qsizetype index) const {
    Q_ASSERT(index >= 0 && index < m_framesData.size());
    return m_framesData[index]->delay();
}

QImage GifFile::image(qsizetype index) const {
    Q_ASSERT(index >= 0 && index < m_framesData.size());
    auto key = getFrameString(index);
    if (auto cached = m_frameCache.object(key)) {
        return *cached;
    }
    auto frame = m_framesData[index]->image();
    if (!frame.isNull()) {
        m_frameCache.insert(key, new QImage(frame));
    }
    return frame;
}

QImage GifFile::imageWithoutCache(qsizetype index) const {
    Q_ASSERT(index >= 0 && index < m_framesData.size());
    return m_framesData[index]->image();
}

qsizetype GifFile::frameCount() const { return m_framesData.size(); }

QString GifFile::comment() const { return m_comment; }

GifFile::const_iterator GifFile::constEnd() const noexcept {
    return m_framesData.constEnd();
}

GifFile::const_iterator GifFile::constBegin() const noexcept {
    return m_framesData.constBegin();
}

GifFile::const_iterator GifFile::cend() const noexcept {
    return m_framesData.cend();
}

GifFile::const_iterator GifFile::cbegin() const noexcept {
    return m_framesData.cbegin();
}

GifFile::iterator GifFile::begin() { return m_framesData.begin(); }

GifFile::iterator GifFile::end() { return m_framesData.end(); }

GifFile::const_iterator GifFile::begin() const { return m_framesData.begin(); }

GifFile::const_iterator GifFile::end() const { return m_framesData.end(); }

QSharedPointer<GifFrame> GifFile::frame(qsizetype index) const {
    return m_framesData[index];
}

void GifFile::replaceFrame(qsizetype index,
                           const QSharedPointer<GifFrame> &frame) {
    m_framesData[index] = frame;
}

bool GifFile::able2InsertFrames(qsizetype count) {
    return frameCount() + count <= GIF_FRAME_COUNT_LIMIT;
}

QSharedPointer<GifFrame> GifFile::generateFrame(const QImage &image,
                                                int delay) {
    if (m_tmpDir.isNull()) {
        return nullptr;
    }
    if (!isValidGifFrame(image, delay)) {
        return nullptr;
    }
    QTemporaryFile tfile(
        m_tmpDir->filePath(QStringLiteral("frame_XXXXXX.png")));
    tfile.setAutoRemove(false);
    if (!tfile.open()) {
        return nullptr;
    }
    return QSharedPointer<GifFrame>::create(image, delay, tfile.fileName());
}

void GifFile::insertFrame(qsizetype index,
                          const QSharedPointer<GifFrame> &frame) {
    if (!able2InsertFrames(1)) {
        return;
    }
    if (frame) {
        m_framesData.insert(index, frame);
    }
}

void GifFile::insertFrames(qsizetype index,
                           const QVector<QSharedPointer<GifFrame>> &frames) {
    if (!able2InsertFrames(frames.size())) {
        return;
    }

    auto n = frames.size();
    if (n == 0) {
        return;
    }
    auto oldSize = m_framesData.size();
    m_framesData.resize(oldSize + n);
    std::move_backward(std::next(m_framesData.begin(), index),
                       std::next(m_framesData.begin(), oldSize),
                       m_framesData.end());
    std::copy(frames.begin(), frames.end(),
              std::next(m_framesData.begin(), index));
}

QSharedPointer<GifFrame> GifFile::removeFrame(qsizetype index) {
    auto ret = m_framesData.takeAt(index);
    m_frameCache.remove(getFrameString(ret));
    return ret;
}

QVector<QSharedPointer<GifFrame>> GifFile::removeFrames(qsizetype index,
                                                        qsizetype count) {
    auto ret = m_framesData.mid(index, count);
    m_framesData.remove(index, ret.size());
    for (auto &frame : ret) {
        m_frameCache.remove(getFrameString(frame));
    }
    return ret;
}

void GifFile::removeFrameIf(
    const std::function<bool(qsizetype, QSharedPointer<GifFrame>)> &fn) {
    if (!fn) {
        return;
    }
    auto total = m_framesData.size();
    for (int i = total - 1; i >= 0; --i) {
        const auto &frame = m_framesData.at(i);
        if (fn(i, frame)) {
            m_frameCache.remove(getFrameString(frame));
            m_framesData.removeAt(i);
        }
    }
}

QPair<QSize, QVector<QSharedPointer<GifFrame>>>
GifFile::replaceFrames(const QSize &frameSize,
                       const QVector<QSharedPointer<GifFrame>> &frames) {
    auto r = qMakePair(m_frameSize, m_framesData);
    m_frameSize = frameSize;
    m_framesData = frames;
    m_frameCache.clear();
    return r;
}

QString GifFile::getFrameString(qsizetype index) const {
    Q_ASSERT(index >= 0 && index < m_framesData.size());
    return getFrameString(m_framesData[index]);
}

QString GifFile::getFrameString(const QSharedPointer<GifFrame> &frame) const {
    Q_ASSERT(frame);
    auto fname = frame->fileName();
    QFileInfo finfo(fname);
    return finfo.fileName();
}

GifFile::ErrorCode GifFile::closeHandleWithError(GifFileType *handle) {
    closeHandle(handle);
    clear();
    m_tmpDir.reset();
    return ErrorCode(handle->Error);
}

GifFile::ErrorCode GifFile::closeHandle(GifFileType *handle) {
    int error = D_GIF_SUCCEEDED;
    if (DGifCloseFile(handle, &error) == GIF_ERROR) {
        clear();
    }
    return ErrorCode(error);
}

bool GifFile::isValidGifFrame(const QImage &image, int delay) {
    if (image.isNull()) {
        return false;
    }
    if (delay <= delayLimitMin() || delay > delayLimitMax()) {
        return false;
    }
    return true;
}

int GifFile::delayLimitMin() { return 10; }

int GifFile::delayLimitMax() { return 65530; }
