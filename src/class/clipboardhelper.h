#ifndef CLIPBOARDHELPER_H
#define CLIPBOARDHELPER_H

#include <QImage>
#include <QObject>

#include "utilities.h"

class ClipBoardHelper : public QObject {
    Q_OBJECT
public:
    static void setImageFrames(const QList<GifData> &frames);
    static void getImageFrames(QList<GifData> &frames);
};

#endif // CLIPBOARDHELPER_H
