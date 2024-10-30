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

#include "class/appmanager.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    /* 有关对在 QT5 的 Win 平台禁用高 dpi 支持
     * 的原因说明：
     * - 因为启用的话会导致样式表对组件边界绘制出现失效
     * - 但 QT6 支持的好，而 Linux 不会有问题
     */

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) &&                                \
     QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#ifndef Q_OS_WIN
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    AppManager a(argc, argv);
    auto w = a.mainWindow();
    w->show();
    Utilities::moveToCenter(w);

    return a.exec();
}
