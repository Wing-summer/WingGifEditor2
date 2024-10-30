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

#ifndef RECENTFILEMANAGER_H
#define RECENTFILEMANAGER_H

#include <QAction>
#include <QList>
#include <QMenu>
#include <QObject>

class RecentFileManager : public QObject {
    Q_OBJECT
public:
    explicit RecentFileManager(QMenu *menu);
    ~RecentFileManager();
    void addRecentFile(const QString &info);
    void clearFile();
    void apply(QWidget *parent, const QStringList &files);

    const QStringList &saveRecent() const;

signals:
    void triggered(const QString &rinfo);

private:
    bool existsPath(const QString &info);
    QString getDisplayFileName(const QString &info);

private:
    QMenu *m_menu;
    QWidget *m_parent;
    QStringList m_recents;
    QList<QAction *> hitems;
};

#endif // RECENTFILEMANAGER_H
