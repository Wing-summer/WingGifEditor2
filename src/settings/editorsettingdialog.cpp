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

#include "editorsettingdialog.h"
#include "ui_editorsettingdialog.h"

#include <QMetaEnum>
#include <QPushButton>

#include "class/languagemanager.h"
#include "class/settingmanager.h"
#include "class/skinmanager.h"
#include "utilities.h"

EditorSettingDialog::EditorSettingDialog(QWidget *parent)
    : FramelessDialogBase(parent), ui(new Ui::EditorSettingDialog) {
    auto widget = new QWidget(this);
    widget->setMinimumSize(400, 400);
    ui->setupUi(widget);

    auto set = &SettingManager::instance();
    ui->cbLanguage->addItem(tr("SystemDefault"));

    auto &lang = LanguageManager::instance();
    const auto langs = lang.langs();
    for (const auto &l : langs) {
        ui->cbLanguage->addItem(lang.langDisplay(l), l);
    }
    connect(ui->cbLanguage, &QComboBox::currentIndexChanged, set,
            [this](int index) {
                auto data = ui->cbLanguage->itemData(index).toString();
                SettingManager::instance().setDefaultLang(data);
            });

    auto e = QMetaEnum::fromType<SkinManager::Theme>();
    for (int i = 0; i < e.keyCount(); ++i) {
        ui->cbTheme->addItem(SkinManager::tr(e.valueToKey(i)));
    }
    connect(ui->cbTheme, &QComboBox::currentIndexChanged, set,
            &SettingManager::setThemeID);

    connect(ui->cbNativeTitile, &QCheckBox::toggled, set,
            &SettingManager::setUseNativeTitleBar);

    ui->cbWinState->addItems({tr("Normal"), tr("Maximized"), tr("FullScreen")});
    connect(ui->cbWinState, &QComboBox::currentIndexChanged, this,
            [](int index) {
                Qt::WindowState state = Qt::WindowState::WindowMaximized;
                switch (index) {
                case 0:
                    state = Qt::WindowState::WindowNoState;
                    break;
                case 1:
                    state = Qt::WindowState::WindowMaximized;
                    break;
                default:
                    state = Qt::WindowState::WindowFullScreen;
                    break;
                }
                SettingManager::instance().setDefaultWinState(state);
            });

    buildUpContent(widget);

    reload();

    setWindowTitle(tr("Setting"));
    setWindowIcon(ICONRES(QStringLiteral("setting")));
}

EditorSettingDialog::~EditorSettingDialog() { delete ui; }

void EditorSettingDialog::reload() {
    ui->cbLanguage->blockSignals(true);
    ui->cbTheme->blockSignals(true);
    ui->cbNativeTitile->blockSignals(true);
    ui->cbWinState->blockSignals(true);

    auto &set = SettingManager::instance();
    auto langs = LanguageManager::instance().langsDisplay();
    auto lang = set.defaultLang();
    if (lang.isEmpty()) {
        ui->cbLanguage->setCurrentIndex(0);
    } else {
        ui->cbLanguage->setCurrentIndex(langs.indexOf(lang) + 1);
    }

    ui->cbTheme->setCurrentIndex(set.themeID());

    ui->cbNativeTitile->setChecked(
        SettingManager::instance().useNativeTitleBar());

    int s;
    switch (set.defaultWinState()) {
    case Qt::WindowNoState:
    case Qt::WindowActive:
        s = 0;
        break;
    case Qt::WindowMinimized:
    case Qt::WindowMaximized:
        s = 1;
        break;
    case Qt::WindowFullScreen:
        s = 2;
        break;
    }

    ui->cbWinState->setCurrentIndex(s);

    ui->cbLanguage->blockSignals(false);
    ui->cbTheme->blockSignals(false);
    ui->cbNativeTitile->blockSignals(false);
    ui->cbWinState->blockSignals(false);
}
