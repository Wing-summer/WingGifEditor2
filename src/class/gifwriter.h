#ifndef GIFWRITER_H
#define GIFWRITER_H

#include "giflib/gif_lib.h"

#include <QImage>
#include <QMap>
#include <QObject>
#include <QString>

#include <memory>

class GifWriter : public QObject {
    Q_OBJECT
private:
    struct Color {
        unsigned char red = 0;
        unsigned char green = 0;
        unsigned char blue = 0;

        bool operator==(const Color &c) const {
            return (red == c.red && green == c.green && blue == c.blue);
        }

        bool operator<(const Color &c) const {
            return (red < c.red ||
                    (red == c.red && green < c.green ||
                     (red == c.red && green == c.green && blue < c.blue)));
        }
    };

    size_t qHash(const Color &c) {
        return (c.red << 16 | c.green << 8 | c.blue);
    }

    enum ColorComponent { Red, Green, Blue };

    struct ColorRange {
        unsigned char lowest = 0;
        unsigned char highest = 0;
    };

private:
    static QPair<ColorComponent, ColorRange>
    longestSide(const QMap<Color, long long int> &s) {
        ColorRange red = {s.firstKey().red, 0};
        ColorRange green = {s.firstKey().green, 0};
        ColorRange blue = {s.firstKey().blue, 0};

        for (auto k = s.keyBegin(); k != s.keyEnd(); k++) {
            if (k->red < red.lowest)
                red.lowest = k->red;

            if (k->red > red.highest)
                red.highest = k->red;

            if (k->green < green.lowest)
                green.lowest = k->green;

            if (k->green > green.highest)
                green.highest = k->green;

            if (k->blue < blue.lowest)
                blue.lowest = k->blue;

            if (k->blue > blue.highest)
                blue.highest = k->blue;
        }

        unsigned char redDistance =
            qRound((float)(red.highest - red.lowest) * 0.299f);
        unsigned char greenDistance =
            qRound((float)(green.highest - green.lowest) * 0.587f);
        unsigned char blueDistance =
            qRound((float)(blue.highest - blue.lowest) * 0.114f);

        const QMap<unsigned char, ColorComponent> m = {
            {redDistance, Red}, {greenDistance, Green}, {blueDistance, Blue}};

        switch (m.last()) {
        case Red:
            return {Red, red};

        case Green:
            return {Green, green};

        case Blue:
            return {Blue, blue};

        default:
            return {Red, red};
        }
    }

    static void
    splitByLongestSide(const QMap<Color, long long int> &s,
                       QVector<QMap<Color, long long int>> &appendTo) {
        QMap<Color, long long int> left, right;

        if (!s.isEmpty()) {
            const auto side = longestSide(s);
            const unsigned char middle =
                (side.second.highest - side.second.lowest) / 2 +
                side.second.lowest;

            for (auto it = s.cbegin(), last = s.cend(); it != last; ++it) {
                switch (side.first) {
                case Red: {
                    if (it.key().red < middle)
                        left.insert(it.key(), it.value());
                    else
                        right.insert(it.key(), it.value());
                } break;

                case Green: {
                    if (it.key().green < middle)
                        left.insert(it.key(), it.value());
                    else
                        right.insert(it.key(), it.value());
                } break;

                case Blue: {
                    if (it.key().blue < middle)
                        left.insert(it.key(), it.value());
                    else
                        right.insert(it.key(), it.value());
                } break;

                default:
                    break;
                }
            }
        }

        appendTo.push_back(left);
        appendTo.push_back(right);
    }

    static QRgb colorForSet(const QMap<Color, long long int> &s) {
        long long int red = 0, green = 0, blue = 0;
        long long int count = 0;

        if (!s.isEmpty()) {
            for (auto it = s.cbegin(), last = s.cend(); it != last; ++it) {
                red += it.key().red * it.value();
                green += it.key().green * it.value();
                blue += it.key().blue * it.value();
                count += it.value();
            }

            Q_ASSERT(count > 0);

            return qRgb(red / count, green / count, blue / count);
        } else
            return qRgb(0, 0, 0);
    }

    static uint
    indexOfColor(const QColor &c,
                 const QVector<QMap<Color, long long int>> &indexed) {
        uint i = 0;
        const auto cc = Color{static_cast<unsigned char>(c.red()),
                              static_cast<unsigned char>(c.green()),
                              static_cast<unsigned char>(c.blue())};

        for (; i < indexed.size(); ++i) {
            if (indexed[i].contains(cc))
                return i;
        }

        return 0;
    }

    static QImage quantizeImageToKColors(const QImage &img, long long int k) {
        if (k == 0 || k == 1)
            return QImage();

        long long int n = 1;

        while (n < k)
            n <<= 1;

        k = n;

        QVector<QMap<Color, long long int>> indexed;
        indexed.push_back({});

        for (long long int y = 0; y < img.height(); ++y) {
            for (long long int x = 0; x < img.width(); ++x) {
                const auto ic = img.pixelColor(x, y);
                const auto c = Color{static_cast<unsigned char>(ic.red()),
                                     static_cast<unsigned char>(ic.green()),
                                     static_cast<unsigned char>(ic.blue())};

                if (!indexed.front().contains(c))
                    indexed.front()[c] = 1;
                else
                    ++indexed.front()[c];
            }
        }

        while (n != 1) {
            QVector<QMap<Color, long long int>> tmp;

            for (const auto &s : std::as_const(indexed))
                splitByLongestSide(s, tmp);

            std::swap(indexed, tmp);

            n /= 2;
        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QList<QRgb> newColors;
#else

        QVector<QRgb> newColors;
#endif

        for (const auto &s : std::as_const(indexed))
            newColors.push_back(colorForSet(s));

        QImage res(img.size(), QImage::Format_Indexed8);
        res.setColorCount(k);
        res.setColorTable(newColors);

        for (long long int y = 0; y < img.height(); ++y) {
            for (long long int x = 0; x < img.width(); ++x)
                res.setPixel(x, y, indexOfColor(img.pixelColor(x, y), indexed));
        }

        return res;
    }

    template <typename T>
    struct ArrayDeleter {
        void operator()(T *p) { delete[] p; }
    };

    struct Resources {
        std::shared_ptr<ColorMapObject> cmap;
        std::shared_ptr<GifColorType> colors;
        std::shared_ptr<GifPixelType> pixels;
        int colorMapSize = 256;

        void init(const QImage &img) {
            const auto q = quantizeImageToKColors(img, colorMapSize);

            cmap = std::make_shared<ColorMapObject>();
            cmap->ColorCount = colorMapSize;
            cmap->BitsPerPixel = 8;

            colors = std::shared_ptr<GifColorType>(
                new GifColorType[colorMapSize], ArrayDeleter<GifColorType>());

            cmap->Colors = colors.get();

            std::map<QRgb, unsigned char> ccm;

            const auto ct = q.colorTable();

            for (int c = 0; c < colorMapSize; ++c) {
                colors.get()[c].Red = qRed(ct[c]);
                colors.get()[c].Green = qGreen(ct[c]);
                colors.get()[c].Blue = qBlue(ct[c]);

                ccm[ct[c]] = static_cast<unsigned char>(c);
            }

            pixels = std::shared_ptr<GifPixelType>(
                new GifPixelType[img.width() * img.height()],
                ArrayDeleter<GifPixelType>());

            for (int y = 0; y < img.height(); ++y) {
                for (int x = 0; x < img.width(); ++x) {
                    pixels.get()[img.width() * y + x] =
                        static_cast<unsigned char>(q.pixelIndex(x, y));
                }
            }
        }
    };

public:
    explicit GifWriter(int width, int height,
                       const QString &filename = QString(),
                       QObject *parent = nullptr);

    void push(const QImage &img, int delay);

    bool pushRange(const QVector<QImage> &imgs, const QVector<int> &delays);

    bool setDelay(qsizetype index, int delay);

    bool setImage(qsizetype index, const QImage &img);

    int delay(qsizetype index) const;

    QImage image(qsizetype index) const;

    bool save(const QString &filename = QString(), unsigned int loopCount = 0);

    void clear();

    void setAspectRatioMode(Qt::AspectRatioMode mode);

    Qt::AspectRatioMode aspectRatioMode() const;

    QString extString() const;

    void setExtString(const QString &newExtString);

signals:
    void sigUpdateUIProcess();

private:
    bool closeEHandle(GifFileType *handle);

    bool closeEHandleWithError(GifFileType *handle);

    std::pair<bool, int> addFrame(GifFileType *handle, QImage &key,
                                  const QImage &frame, int delay,
                                  std::vector<Resources> &resources);

    bool addFrame(GifFileType *handle, const QImage &img, const QRect &r,
                  int delay, std::vector<Resources> &resources);

    std::pair<QImage, QRect> diffImage(const QImage &key, const QImage &img);

private:
    QString m_filename;

    QString m_extString;

    QVector<int> m_delays;
    QVector<QImage> m_data;

    int m_width;
    int m_height;

    Qt::AspectRatioMode m_mode = Qt::AspectRatioMode::IgnoreAspectRatio;
};

#endif // GIFWRITER_H
