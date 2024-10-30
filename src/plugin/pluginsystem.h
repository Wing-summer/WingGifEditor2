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

#ifndef PLUGINSYSTEM_H
#define PLUGINSYSTEM_H

#include "iwingplugin.h"

#include <QObject>

using namespace WingGif;

class MainWindow;

class PluginSystem : public QObject {
    Q_OBJECT
public:
    static PluginSystem &instance();

    void setMainWindow(MainWindow *win);
    QWidget *mainWindow() const;

    const QList<IWingPlugin *> &plugins() const;

    const IWingPlugin *plugin(qsizetype index) const;

    void LoadPlugin();

    void callPluginProcess(QImage &image, int delay, qsizetype index);

private:
    PluginSystem(QObject *parent = nullptr);
    ~PluginSystem();

private:
    bool checkThreadAff();
    void connectUIInterface(IWingPlugin *plg);

private:
    void loadPlugin(const QFileInfo &fileinfo);

    bool loadPlugin(IWingPlugin *p);
    void connectBaseInterface(IWingPlugin *plg);
    void connectInterface(IWingPlugin *plg);

    QString packLogMessage(const char *header, const QString &msg);

private:
    MainWindow *_win = nullptr;

    QStringList loadedpuid;
    QList<IWingPlugin *> loadedplgs;
};

#endif // PLUGINSYSTEM_H
