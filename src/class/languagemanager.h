#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QHash>
#include <QLocale>
#include <QObject>
#include <QStringList>

class LanguageManager : public QObject {
    Q_OBJECT

public:
    static LanguageManager &instance();

    QStringList langsDisplay() const;

    QString currentLang() const;

    QLocale defaultLocale() const;

private:
    LanguageManager();

    Q_DISABLE_COPY(LanguageManager)

private:
    QLocale _defaultLocale;

    QStringList m_langs;
    QStringList m_langsDisplay;
    QHash<QString, QString> m_langMap;
    QHash<QString, QLocale> m_localeMap;
};

#endif // LANGUAGEMANAGER_H
