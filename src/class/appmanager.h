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

#ifndef APPMANAGER_H
#define APPMANAGER_H

#include "dialog/mainwindow.h"

#include <QApplication>

class AppManager : public QApplication {
    Q_OBJECT
public:
    explicit AppManager(int &argc, char *argv[]);
    virtual ~AppManager();

    static AppManager *instance();

    MainWindow *mainWindow() const;

public slots:
    void openFile(const QString &file);

private:
    MainWindow *_w = nullptr;
    static AppManager *_instance;
};

#endif // APPMANAGER_H
