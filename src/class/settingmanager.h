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
