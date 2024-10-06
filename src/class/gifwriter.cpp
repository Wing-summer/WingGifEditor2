#include "gifwriter.h"

#include <QPainter>

GifWriter::GifWriter(int width, int height, const QString &filename)
    : m_width(width), m_height(height), m_filename(filename) {
    Q_ASSERT(width > 0 && height > 0);
}

void GifWriter::push(const QImage &img, int delay) {
    m_data.append(
        img.scaled(m_width, m_height, m_mode, Qt::SmoothTransformation));
    m_delays.append(delay);
}

bool GifWriter::pushRange(const QVector<QImage> &imgs,
                          const QVector<int> &delays) {
    if (imgs.size() != delays.size()) {
        return false;
    }
    for (auto &img : imgs) {
        m_data.append(
            img.scaled(m_width, m_height, m_mode, Qt::SmoothTransformation));
    }
    for (auto &delay : delays) {
        m_delays.append(delay);
    }
    return true;
}

bool GifWriter::setDelay(qsizetype index, int delay) {
    if (index < 0 || index >= m_delays.size()) {
        return false;
    }
    m_delays[index] = delay;
    return true;
}

bool GifWriter::setImage(qsizetype index, const QImage &img) {
    if (index < 0 || index >= m_delays.size()) {
        return false;
    }
    m_data[index] =
        img.scaled(m_width, m_height, m_mode, Qt::SmoothTransformation);
    return true;
}

int GifWriter::delay(qsizetype index) const { return m_delays.at(index); }

QImage GifWriter::image(qsizetype index) const { return m_data.at(index); }

bool GifWriter::save(const QString &filename, unsigned int loopCount) {
    auto fn = filename.isEmpty() ? m_filename : filename;
    if (fn.isEmpty() || m_data.isEmpty() || m_delays.isEmpty()) {
        return false;
    }

    auto handle = EGifOpenFileName(fn.toUtf8(), false, nullptr);

    if (handle) {
        EGifSetGifVersion(handle, true);

        QImage key = m_data.first();

        Resources res;
        res.init(key);

        std::vector<Resources> resources;

        if (EGifPutScreenDesc(handle, key.width(), key.height(),
                              res.colorMapSize, 0, res.cmap.get()) == GIF_ERROR)
            return closeEHandleWithError(handle);

        unsigned char params[3] = {1, 0, 0};
        params[1] = (loopCount & 0xFF);
        params[2] = (loopCount >> 8) & 0xFF;

        if (EGifPutExtensionLeader(handle, APPLICATION_EXT_FUNC_CODE) ==
            GIF_ERROR)
            return closeEHandleWithError(handle);

        if (EGifPutExtensionBlock(handle, 11, (GifByteType *)"NETSCAPE2.0") ==
            GIF_ERROR)
            return closeEHandleWithError(handle);

        if (EGifPutExtensionBlock(handle, sizeof(params), params) == GIF_ERROR)
            return closeEHandleWithError(handle);

        if (EGifPutExtensionTrailer(handle) == GIF_ERROR)
            return closeEHandleWithError(handle);

        if (!m_extString.isEmpty()) {
            if (EGifPutComment(handle, m_extString.toLatin1().data()) ==
                GIF_ERROR)
                return closeEHandleWithError(handle);
        }

        if (!addFrame(handle, key, key.rect(), m_delays.at(0), resources))
            return closeEHandleWithError(handle);

        int delta = 0;

        for (qsizetype i = 1; i < m_data.size(); ++i) {
            bool result = false;

            std::tie(result, delta) = addFrame(
                handle, key, m_data.at(i), m_delays.at(i) + delta, resources);

            if (!result)
                return closeEHandleWithError(handle);
        }

        closeEHandle(handle);

        return true;
    }

    return false;
}

void GifWriter::clear() {
    m_data.clear();
    m_delays.clear();
}

void GifWriter::setAspectRatioMode(Qt::AspectRatioMode mode) { m_mode = mode; }

Qt::AspectRatioMode GifWriter::aspectRatioMode() const { return m_mode; }

QString GifWriter::extString() const { return m_extString; }

void GifWriter::setExtString(const QString &newExtString) {
    m_extString = newExtString.left(255);
}

bool GifWriter::closeEHandle(GifFileType *handle) {
    if (!EGifCloseFile(handle, nullptr))
        return false;
    else
        return true;
}

bool GifWriter::closeEHandleWithError(GifFileType *handle) {
    closeEHandle(handle);
    return false;
}

std::pair<bool, int> GifWriter::addFrame(GifFileType *handle, QImage &key,
                                         const QImage &frame, int delay,
                                         std::vector<Resources> &resources) {
    QImage img = frame;

    if (key.width() != img.width() || key.height() != img.height()) {
        img = QImage(key.width(), key.height(), QImage::Format_ARGB32);
        img.fill(Qt::black);

        QPainter p(&img);
        p.drawImage(
            frame.width() < key.width() ? (key.width() - frame.width()) / 2 : 0,
            frame.height() < key.height() ? (key.height() - frame.height()) / 2
                                          : 0,
            frame);
    }

    QImage tmp;
    QRect r;

    std::tie(tmp, r) = diffImage(key, img);

    bool ret = true;
    int delta = delay;

    if (r.width() && r.height()) {
        ret = addFrame(handle, tmp, r, delay, resources);

        delta = 0;

        key = img;
    }

    return std::make_pair(ret, delta);
}

bool GifWriter::addFrame(GifFileType *handle, const QImage &img, const QRect &r,
                         int delay, std::vector<Resources> &resources) {
    GraphicsControlBlock b;
    b.DelayTime = delay / 10;
    b.DisposalMode = DISPOSE_DO_NOT;
    b.TransparentColor = -1;
    b.UserInputFlag = false;

    GifByteType ext[4];

    const auto len = EGifGCBToExtension(&b, ext);

    if (EGifPutExtension(handle, GRAPHICS_EXT_FUNC_CODE, len, ext) == GIF_ERROR)
        return false;

    Resources res;
    res.init(img);
    resources.push_back(res);

    if (EGifPutImageDesc(handle, r.x(), r.y(), r.width(), r.height(), false,
                         res.cmap.get()) == GIF_ERROR)
        return false;

    if (EGifPutLine(handle, res.pixels.get(), img.width() * img.height()) ==
        GIF_ERROR)
        return false;

    return true;
}

std::pair<QImage, QRect> GifWriter::diffImage(const QImage &key,
                                              const QImage &img) {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    int i = 0;
    int j = 0;
    bool different = false;

    for (; i < key.width(); ++i) {
        for (j = 0; j < key.height(); ++j) {
            if (key.pixel(i, j) != img.pixel(i, j)) {
                different = true;

                break;
            }
        }

        if (different)
            break;
    }

    if (i < key.width())
        x = i;
    else
        return {{}, QRect(0, 0, 0, 0)};

    different = false;

    for (j = 0; j < key.height(); ++j) {
        for (i = x; i < key.width(); ++i) {
            if (key.pixel(i, j) != img.pixel(i, j)) {
                different = true;

                break;
            }
        }

        if (different)
            break;
    }

    y = j;

    different = false;

    for (i = key.width() - 1; i > x; --i) {
        for (j = y + 1; j < key.height(); ++j) {
            if (key.pixel(i, j) != img.pixel(i, j)) {
                different = true;

                break;
            }
        }

        if (different)
            break;
    }

    width = i - x + 1;

    different = false;

    for (j = key.height() - 1; j > y; --j) {
        for (i = x; i < x + width; ++i) {
            if (key.pixel(i, j) != img.pixel(i, j)) {
                different = true;

                break;
            }
        }

        if (different)
            break;
    }

    height = j - y + 1;

    const auto r = QRect(x, y, width, height);

    return {img.copy(r), r};
}
