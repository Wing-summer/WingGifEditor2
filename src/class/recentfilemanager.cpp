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

#include "recentfilemanager.h"
#include "control/toast.h"
#include "utilities.h"
#include "winginputdialog.h"

#include <QFile>
#include <QMenu>

RecentFileManager::RecentFileManager(QMenu *menu) : QObject(), m_menu(menu) {
    Q_ASSERT(menu);
    menu->setToolTipsVisible(true);
}

void RecentFileManager::apply(QWidget *parent, const QStringList &files) {
    Q_ASSERT(parent);
    this->setParent(parent);
    m_parent = parent;

    QAction *a;
    a = new QAction(m_menu);
    a->setText(tr("ClearHistory"));
    a->setIcon(ICONRES("clearhis"));
    connect(a, &QAction::triggered, this, &RecentFileManager::clearFile);
    m_menu->addAction(a);

    a = new QAction(m_menu);
    a->setText(tr("RemoveItem"));
    a->setIcon(ICONRES("del"));
    connect(a, &QAction::triggered, this, [this] {
        if (hitems.count() == 0) {
            Toast::toast(m_parent, NAMEICONRES(QStringLiteral("clearhis")),
                         tr("NoHistoryDel"));
            return;
        }
        QStringList binfos;
        for (auto &info : m_recents) {
            binfos.append(getDisplayFileName(info));
        }
        bool ok;
        auto ret = WingInputDialog::getItem(m_parent, tr("PleaseChoose"),
                                            tr("Select to remove:"), binfos, 0,
                                            false, &ok);
        if (ok) {
            auto ind = binfos.indexOf(ret);
            Q_ASSERT(ind >= 0);
            m_recents.removeAt(ind);
            auto action = hitems.takeAt(ind);
            m_menu->removeAction(action);
            action->deleteLater();
        }
    });
    m_menu->addAction(a);

    m_menu->addSeparator();

    for (auto &item : files) {
        addRecentFile(item);
    }
}

const QStringList &RecentFileManager::saveRecent() const { return m_recents; }

bool RecentFileManager::existsPath(const QString &info) {
    return QFile::exists(info);
}

QString RecentFileManager::getDisplayFileName(const QString &info) {
    return QFileInfo(info).fileName();
}

RecentFileManager::~RecentFileManager() {}

void RecentFileManager::addRecentFile(const QString &info) {
    int o = 0;
    if (existsPath(info) && (o = m_recents.indexOf(info)) < 0) {
        while (m_recents.count() >= 10) {
            m_recents.pop_back();
        }
        auto a = new QAction(m_menu);
        a->setData(info);
        a->setText(getDisplayFileName(info));
        a->setToolTip(a->text());
        connect(a, &QAction::triggered, this, [=] {
            auto send = qobject_cast<QAction *>(sender());
            if (send) {
                auto f = send->data().toString();
                if (existsPath(f)) {
                    emit triggered(f);
                } else {
                    auto index = hitems.indexOf(send);
                    if (index < 0) {
                        m_menu->removeAction(send);
                        hitems.removeAt(index);
                        m_recents.removeAt(index);
                        Toast::toast(m_parent,
                                     NAMEICONRES(QStringLiteral("clearhis")),
                                     tr("FileNotExistClean"));
                    }
                }
            }
        });
        m_recents.push_front(info);
        if (hitems.count())
            m_menu->insertAction(hitems.first(), a);
        else
            m_menu->addAction(a);
        hitems.push_front(a);
    } else {
        if (hitems.count() > 1) {
            auto a = hitems.at(o);
            m_menu->removeAction(a);
            m_menu->insertAction(hitems.first(), a);
            hitems.move(o, 0);
        }
    }
}

void RecentFileManager::clearFile() {
    if (hitems.count() == 0) {
        Toast::toast(m_parent, NAMEICONRES(QStringLiteral("clearhis")),
                     tr("NoHistoryDel"));
        return;
    }
    for (auto &item : hitems) {
        m_menu->removeAction(item);
        item->deleteLater();
    }
    m_recents.clear();
    hitems.clear();
    Toast::toast(m_parent, NAMEICONRES(QStringLiteral("clearhis")),
                 tr("HistoryClearFinished"));
}
