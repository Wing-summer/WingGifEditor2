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

#include "appmanager.h"

#include <QFont>

#include "languagemanager.h"
#include "skinmanager.h"

#include "class/logger.h"
#include "dbghelper.h"
#include "dialog/mainwindow.h"

AppManager *AppManager::_instance = nullptr;

AppManager::AppManager(int &argc, char *argv[]) : QApplication(argc, argv) {
    ASSERT_SINGLETON;

    setApplicationName(APP_NAME);
    setOrganizationName(APP_ORG);
    setApplicationVersion(WINGGIF_VERSION);

    Logger::instance();

    auto args = arguments();
    SkinManager::instance();
    LanguageManager::instance();

    _w = new MainWindow;

    if (args.size() > 1) {
        for (auto var = args.begin() + 1; var != args.end(); ++var) {
            openFile(*var);
        }
    }

    _instance = this;
}

AppManager::~AppManager() {
    _w->deleteLater();
    _w = nullptr;
}

AppManager *AppManager::instance() { return _instance; }

MainWindow *AppManager::mainWindow() const { return _w; }

void AppManager::openFile(const QString &file) { _w->openGif(file); }
