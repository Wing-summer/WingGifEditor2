#ifndef GIFREADER_H
#define GIFREADER_H

#include "giflib/gif_lib.h"

#include <QImage>
#include <QString>
#include <QVector>

class GifReader {
public:
    explicit GifReader(const QString &filename = QString());

    virtual ~GifReader();

    bool isNull() const;

    int width() const;

    int height() const;

    bool load(const QString &filename);

    int delay(qsizetype index) const;

    QImage image(qsizetype index) const;

    const QVector<int> delays() const;

    const QVector<QImage> images() const;

    qsizetype imageCount() const;

    QString comment() const;

private:
    bool closeHandleWithError(GifFileType *handle);

    bool closeHandle(GifFileType *handle);

    void invalidCache();

    void clearCache();

private:
    qsizetype m_framesCount = -1;
    QVector<int> m_delays;
    QVector<QImage> m_data;
    QString m_comment;
};

#endif // GIFREADER_H
