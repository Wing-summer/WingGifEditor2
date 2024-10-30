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

#ifndef SETTING_H
#define SETTING_H

#include <QApplication>
#include <QSettings>

#define HANDLE_CONFIG                                                          \
    QSettings set(QStringLiteral(APP_ORG), QStringLiteral(APP_NAME))

#define CONFIG set

#define WRITE_CONFIG(config, dvalue) set.setValue(config, dvalue)

#define READ_CONFIG(config, dvalue) set.value(config, dvalue)

#define READ_CONFIG_SAFE(var, config, dvalue, func)                            \
    {                                                                          \
        auto b = false;                                                        \
        var = READ_CONFIG(config, dvalue).func(&b);                            \
        if (!b) {                                                              \
            var = dvalue;                                                      \
        }                                                                      \
    }

#define READ_CONFIG_INT(var, config, dvalue)                                   \
    READ_CONFIG_SAFE(var, config, dvalue, toInt)

#define READ_CONFIG_BOOL(var, config, dvalue)                                  \
    var = READ_CONFIG(config, dvalue).toBool()

#define READ_CONFIG_QSIZETYPE(var, config, dvalue)                             \
    READ_CONFIG_SAFE(var, config, dvalue, toLongLong)

#define READ_CONFIG_INT_POSITIVE(var, config, dvalue)                          \
    {                                                                          \
        Q_ASSERT(dvalue > 0);                                                  \
        READ_CONFIG_SAFE(var, config, dvalue, toInt);                          \
        if (var <= 0) {                                                        \
            var = dvalue;                                                      \
        }                                                                      \
    }

#define READ_CONFIG_DOUBLE_POSITIVE(var, config, dvalue)                       \
    {                                                                          \
        Q_ASSERT(dvalue > 0);                                                  \
        READ_CONFIG_SAFE(var, config, dvalue, toDouble);                       \
        if (var <= 0) {                                                        \
            var = dvalue;                                                      \
        }                                                                      \
    }

#endif // SETTING_H
