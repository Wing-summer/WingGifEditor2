#include "gifreader.h"

#include <QFile>
#include <QPainter>

GifReader::GifReader(const QString &filename) {
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
    return m_data.first().width();
}

int GifReader::height() const {
    if (isNull()) {
        return 0;
    }
    return m_data.first().height();
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
                    img.convertTo(QImage::Format_ARGB32);
                    key = img;
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

                m_delays.push_back(animDelay);
                m_data.push_back(img);
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
    return m_data.at(index);
}

const QVector<int> GifReader::delays() const { return m_delays; }

const QVector<QImage> GifReader::images() const { return m_data; }

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
    m_data.clear();
    m_delays.clear();
    m_comment.clear();
}
