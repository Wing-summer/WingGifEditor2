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
    return QStringLiteral(":/com.wingsummer.winggif/images/") + name +
           QStringLiteral(".png");
}

Q_DECL_UNUSED static inline QIcon ICONRES(const QString &name) {
    static QHash<QString, QIcon> cache;
    auto picon = cache.find(name);
    if (picon == cache.end()) {
        QIcon icon(NAMEICONRES(name));
        cache.insert(name, icon);
        return icon;
    } else {
        return *picon;
    }
}

class Utilities {
public:
    static QString processBytesCount(qint64 bytescount) {
        QStringList B{"B", "KB", "MB", "GB", "TB"};
        auto av = bytescount;
        auto r = av;

        for (int i = 0; i < 5; i++) {
            auto lld = lldiv(r, 1024);
            r = lld.quot;
            av = lld.rem;
            if (r == 0) {
                return QStringLiteral("%1 %2").arg(av).arg(B.at(i));
            }
        }

        return QStringLiteral("%1 TB").arg(av);
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
            QStandardPaths::AppDataLocation);
    }
};

#endif // UTILITIES_H
