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
