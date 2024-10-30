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

#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QByteArray>
#include <QFont>
#include <QObject>
#include <QString>
#include <QStringList>

class SettingManager : public QObject {
    Q_OBJECT

public:
    static SettingManager &instance();

    ~SettingManager();

    int themeID() const;
    void setThemeID(int newThemeID);

    QString defaultLang() const;
    void setDefaultLang(const QString &newDefaultLang);

    Qt::WindowState defaultWinState() const;
    void setDefaultWinState(Qt::WindowState newDefaultWinState);

    void save();
    void reset();

    QStringList recentFiles() const;
    void setRecentFiles(const QStringList &newRecentFiles);

    QString lastUsedPath() const;
    void setLastUsedPath(const QString &newLastUsedPath);

    bool useNativeTitleBar() const;
    void setUseNativeTitleBar(bool newUseNativeTitleBar);

private:
    SettingManager();

    void load();

    Q_DISABLE_COPY(SettingManager)
private:
    int m_themeID = 0;
    QString m_defaultLang;

    QStringList m_recentFiles;
    Qt::WindowState m_defaultWinState = Qt::WindowMaximized;

    QString m_lastUsedPath;

    bool m_useNativeTitleBar = false;
};

#endif // SETTINGMANAGER_H
