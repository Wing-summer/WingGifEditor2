#include "settingmanager.h"
#include "setting.h"

#include "class/skinmanager.h"
#include <QFileInfo>
#include <QMetaEnum>

const auto APP_LASTUSED_PATH = QStringLiteral("app.lastusedpath");
const auto SKIN_THEME = QStringLiteral("skin.theme");
const auto APP_WINDOWSIZE = QStringLiteral("app.windowsize");
const auto APP_LANGUAGE = QStringLiteral("app.lang");
const auto EDITOR_RECENTFILES = QStringLiteral("editor.recentfiles");
const auto OTHER_USE_NATIVE_TITLEBAR = QStringLiteral("sys.nativeTitleBar");

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
}

bool SettingManager::useNativeTitleBar() const { return m_useNativeTitleBar; }

void SettingManager::setUseNativeTitleBar(bool newUseNativeTitleBar) {
    m_useNativeTitleBar = newUseNativeTitleBar;
}

QStringList SettingManager::recentFiles() const { return m_recentFiles; }

void SettingManager::setRecentFiles(const QStringList &newRecentFiles) {
    m_recentFiles = newRecentFiles;
}

Qt::WindowState SettingManager::defaultWinState() const {
    return m_defaultWinState;
}

void SettingManager::setDefaultWinState(Qt::WindowState newDefaultWinState) {
    switch (newDefaultWinState) {
    case Qt::WindowNoState:
    case Qt::WindowMaximized:
    case Qt::WindowFullScreen:
        break;
    default:
        newDefaultWinState = Qt::WindowMaximized;
        break;
    }
    m_defaultWinState = newDefaultWinState;
}

void SettingManager::save() {
    HANDLE_CONFIG;
    WRITE_CONFIG(EDITOR_RECENTFILES, m_recentFiles);
    WRITE_CONFIG(APP_LASTUSED_PATH, m_lastUsedPath);

    WRITE_CONFIG(SKIN_THEME, m_themeID);
    WRITE_CONFIG(APP_LANGUAGE, m_defaultLang);
    WRITE_CONFIG(APP_WINDOWSIZE, m_defaultWinState);

#ifdef WINGHEX_USE_FRAMELESS
    WRITE_CONFIG(OTHER_USE_NATIVE_TITLEBAR, m_useNativeTitleBar);
#endif
}

void SettingManager::reset() {
    HANDLE_CONFIG;

    WRITE_CONFIG(SKIN_THEME, 0);
    WRITE_CONFIG(APP_LANGUAGE, QString());
    WRITE_CONFIG(APP_WINDOWSIZE, Qt::WindowMaximized);

#ifdef WINGHEX_USE_FRAMELESS
    WRITE_CONFIG(OTHER_USE_NATIVE_TITLEBAR, false);
#endif

    load();
}

QString SettingManager::defaultLang() const { return m_defaultLang; }

void SettingManager::setDefaultLang(const QString &newDefaultLang) {
    m_defaultLang = newDefaultLang;
}

SettingManager &SettingManager::instance() {
    static SettingManager ins;
    return ins;
}

SettingManager::~SettingManager() {}

int SettingManager::themeID() const { return m_themeID; }

void SettingManager::setThemeID(int newThemeID) { m_themeID = newThemeID; }
