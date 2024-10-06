#ifndef UTILITIES_H
#define UTILITIES_H

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QScreen>
#include <QStandardPaths>
#include <QWidget>

Q_DECL_UNUSED static inline QString NAMEICONRES(const QString &name) {
    return ":/com.wingsummer.winggif/images/" + name + ".png";
}

Q_DECL_UNUSED static inline QIcon ICONRES(const QString &name) {
    return QIcon(NAMEICONRES(name));
}

struct GifData {
    int delay;
    QImage image;
};

class Utilities {
public:
    static bool checkIsLittleEndian() {
        short s = 0x1122;
        auto l = *reinterpret_cast<char *>(&s);
        return l == 0x22;
    }

    static bool fileCanWrite(QString path) {
        return QFileInfo(path).permission(QFile::WriteUser);
    }

    static void moveToCenter(QWidget *window) {
        if (window == nullptr)
            return;
        auto screen = qApp->primaryScreen()->availableSize();
        window->move((screen.width() - window->width()) / 2,
                     (screen.height() - window->height()) / 2);
    }

    static QString getAppDataPath() {
        return QStandardPaths::writableLocation(
                   QStandardPaths::AppDataLocation) +
               QDir::separator() + APP_NAME;
    }
};

#endif // UTILITIES_H
