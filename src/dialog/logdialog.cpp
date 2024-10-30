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

#include "logdialog.h"

LogDialog::LogDialog(QWidget *parent) : FramelessDialogBase(parent) {
    _log = new QTextBrowser(this);
    _log->setFocusPolicy(Qt::StrongFocus);
    _log->setOpenExternalLinks(true);
    _log->setUndoRedoEnabled(false);

    setWindowTitle(tr("Log"));

    buildUpContent(_log);
}

void LogDialog::log(const QString &message) { _log->append(message); }
