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

#include "gifreader.h"

#include <QDir>
#include <QFile>
#include <QPainter>

GifReader::GifReader(const QString &filename, QObject *parent)
    : QObject(parent), m_frameCache(48) {
    if (QFile::exists(filename)) {
        load(filename);
    }
}

GifReader::~GifReader() { invalidCache(); }

bool GifReader::isNull() const { return m_framesCount < 0; }

int GifReader::width() const {
    if (isNull()) {
        return 0;
    }
    return m_frameSize.width();
}

int GifReader::height() const {
    if (isNull()) {
        return 0;
    }
    return m_frameSize.height();
}

bool GifReader::load(const QString &filename) {
    auto handle = DGifOpenFileName(filename.toLocal8Bit().data(), nullptr);

    if (handle) {
        int animDelay = -1;
        int disposalMode = -1;
        int transparentIndex = -1;
        GifRecordType recordType;
        QImage key;

        clearCache();

        m_tmpDir = std::make_unique<QTemporaryDir>();
        if (!m_tmpDir->isValid()) {
            return closeHandleWithError(handle);
        }

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

                ++m_framesCount;

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

                if (m_frameSize.isEmpty()) {
                    m_frameSize = img.size();
                }

                m_delays.push_back(animDelay);

                const auto framePath = m_tmpDir->path() + QDir::separator() +
                                       QStringLiteral("frame_%1.png")
                                           .arg(m_framesCount, 6, 10, QLatin1Char('0'));
                if (!img.save(framePath, "PNG")) {
                    return closeHandleWithError(handle);
                }

                m_frameFiles.push_back(framePath);
                m_frameCache.insert(m_framesCount, new QImage(img));
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
                        auto rawString = QByteArray(
                            reinterpret_cast<const char *>(extData + 1), len);
                        m_comment += QString::fromLatin1(rawString);
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
        return false;
    }
}

int GifReader::delay(qsizetype index) const {
    Q_ASSERT(m_framesCount >= 0);
    return m_delays.at(index);
}

QImage GifReader::image(qsizetype index) const {
    Q_ASSERT(m_framesCount >= 0);

    if (auto cached = m_frameCache.object(index)) {
        return *cached;
    }

    if (index < 0 || index >= m_frameFiles.size()) {
        return {};
    }

    QImage frame;
    frame.load(m_frameFiles.at(index));
    if (!frame.isNull()) {
        m_frameCache.insert(index, new QImage(frame));
    }
    return frame;
}

const QVector<int> GifReader::delays() const { return m_delays; }

const QVector<QImage> GifReader::images() const {
    QVector<QImage> result;
    result.reserve(m_frameFiles.size());
    for (qsizetype i = 0; i < m_frameFiles.size(); ++i) {
        result.push_back(image(i));
    }
    return result;
}

QString GifReader::frameFilePath(qsizetype index) const {
    if (index < 0 || index >= m_frameFiles.size()) {
        return {};
    }
    return m_frameFiles.at(index);
}

qsizetype GifReader::imageCount() const {
    return m_framesCount >= 0 ? m_framesCount : 0;
}

QString GifReader::comment() const { return m_comment; }

bool GifReader::closeHandleWithError(GifFileType *handle) {
    closeHandle(handle);
    invalidCache();
    return false;
}

bool GifReader::closeHandle(GifFileType *handle) {
    if (!DGifCloseFile(handle, nullptr)) {
        clearCache();

        return false;
    } else
        return true;
}

void GifReader::invalidCache() {
    clearCache();
    m_framesCount = -1;
}

void GifReader::clearCache() {
    m_framesCount = 0;
    m_frameSize = QSize();
    m_frameFiles.clear();
    m_delays.clear();
    m_comment.clear();
    m_frameCache.clear();
    m_tmpDir.reset();
}
