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

#include "settingmanager.h"
#include "settings.h"

#include "class/skinmanager.h"

#include <QFileInfo>
#include <QMetaEnum>

Q_GLOBAL_STATIC_WITH_ARGS(QString, APP_LASTUSED_PATH, ("app.lastusedpath"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, SKIN_THEME, ("skin.theme"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, APP_WINDOWSIZE, ("app.windowsize"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, APP_LANGUAGE, ("app.lang"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, EDITOR_RECENTFILES, ("editor.recentfiles"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, OTHER_USE_NATIVE_TITLEBAR,
                          ("sys.nativeTitleBar"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, OTHER_USESYS_FILEDIALOG,
                          ("sys.nativeDialog"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, OTHER_SET_LAYOUT, ("set.layout"))

SettingManager::SettingManager() { load(); }

QString SettingManager::lastUsedPath() const { return m_lastUsedPath; }

void SettingManager::setLastUsedPath(const QString &newLastUsedPath) {
    m_lastUsedPath = newLastUsedPath;
}

void SettingManager::load() {
    HANDLE_CONFIG;
    READ_CONFIG_INT(m_themeID, SKIN_THEME, 0);
    m_themeID = qBound(0, m_themeID,
                       QMetaEnum::fromType<SkinManager::Theme>().keyCount());
    m_defaultLang = READ_CONFIG(APP_LANGUAGE, QString()).toString();

    this->setDefaultWinState(READ_CONFIG(APP_WINDOWSIZE, Qt::WindowMaximized)
                                 .value<Qt::WindowState>());

#ifdef WINGHEX_USE_FRAMELESS
    READ_CONFIG_BOOL(m_useNativeTitleBar, OTHER_USE_NATIVE_TITLEBAR, false);
#else
    m_useNativeTitleBar = true;
#endif

    m_recentFiles =
        READ_CONFIG(EDITOR_RECENTFILES, QStringList()).toStringList();

    m_lastUsedPath = READ_CONFIG(APP_LASTUSED_PATH, QString()).toString();
    if (!m_lastUsedPath.isEmpty()) {
        QFileInfo info(m_lastUsedPath);
        if (info.exists() && info.isDir()) {
        } else {
            m_lastUsedPath.clear();
        }
    }

    m_setLayout = READ_CONFIG(OTHER_SET_LAYOUT, QByteArray()).toByteArray();
}

QByteArray SettingManager::editorLayout() const { return m_setLayout; }

void SettingManager::setEditorLayout(const QByteArray &newSetLayout) {
    if (m_setLayout != newSetLayout) {
        HANDLE_CONFIG;
        WRITE_CONFIG(OTHER_SET_LAYOUT, newSetLayout);
        m_setLayout = newSetLayout;
    }
}

bool SettingManager::useNativeFileDialog() const {
    return m_useNativeFileDialog;
}

void SettingManager::setUseNativeFileDialog(bool newUseNativeFileDialog) {
    if (m_useNativeFileDialog != newUseNativeFileDialog) {
        HANDLE_CONFIG;
        WRITE_CONFIG(OTHER_USESYS_FILEDIALOG, newUseNativeFileDialog);
        m_useNativeFileDialog = newUseNativeFileDialog;
    }
}

bool SettingManager::useNativeTitleBar() const { return m_useNativeTitleBar; }

void SettingManager::setUseNativeTitleBar(bool newUseNativeTitleBar) {
    if (m_useNativeTitleBar != newUseNativeTitleBar) {
        HANDLE_CONFIG;
        WRITE_CONFIG(OTHER_USE_NATIVE_TITLEBAR, newUseNativeTitleBar);
        m_useNativeTitleBar = newUseNativeTitleBar;
    }
}

QStringList SettingManager::recentFiles() const { return m_recentFiles; }

void SettingManager::setRecentFiles(const QStringList &newRecentFiles) {
    if (m_recentFiles != newRecentFiles) {
        HANDLE_CONFIG;
        WRITE_CONFIG(EDITOR_RECENTFILES, newRecentFiles);
        m_recentFiles = newRecentFiles;
    }
}

Qt::WindowState SettingManager::defaultWinState() const {
    return m_defaultWinState;
}

void SettingManager::setDefaultWinState(Qt::WindowState newDefaultWinState) {
    if (m_defaultWinState != newDefaultWinState) {
        switch (newDefaultWinState) {
        case Qt::WindowNoState:
        case Qt::WindowMaximized:
        case Qt::WindowFullScreen:
            break;
        default:
            newDefaultWinState = Qt::WindowMaximized;
            break;
        }
        HANDLE_CONFIG;
        WRITE_CONFIG(APP_WINDOWSIZE, newDefaultWinState);
        m_defaultWinState = newDefaultWinState;
    }
}

QString SettingManager::defaultLang() const { return m_defaultLang; }

void SettingManager::setDefaultLang(const QString &newDefaultLang) {
    if (m_defaultLang != newDefaultLang) {
        HANDLE_CONFIG;
        WRITE_CONFIG(APP_LANGUAGE, newDefaultLang);
        m_defaultLang = newDefaultLang;
    }
}

SettingManager &SettingManager::instance() {
    static SettingManager ins;
    return ins;
}

SettingManager::~SettingManager() {}

int SettingManager::themeID() const { return m_themeID; }

void SettingManager::setThemeID(int newThemeID) {
    if (m_themeID != newThemeID) {
        HANDLE_CONFIG;
        WRITE_CONFIG(SKIN_THEME, newThemeID);
        m_themeID = newThemeID;
    }
}
