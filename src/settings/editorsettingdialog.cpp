#include "editorsettingdialog.h"
#include "ui_editorsettingdialog.h"

#include <QMetaEnum>
#include <QPushButton>

#include "class/languagemanager.h"
#include "class/settingmanager.h"
#include "class/skinmanager.h"

EditorSettingDialog::EditorSettingDialog(QWidget *parent)
    : FramelessDialogBase(parent), ui(new Ui::EditorSettingDialog) {
    auto widget = new QWidget(this);
    widget->setMinimumSize(400, 400);
    ui->setupUi(widget);

    ui->cbLanguage->addItem(tr("SystemDefault"));
    ui->cbLanguage->addItems(LanguageManager::instance().langsDisplay());

    auto e = QMetaEnum::fromType<SkinManager::Theme>();
    for (int i = 0; i < e.keyCount(); ++i) {
        ui->cbTheme->addItem(SkinManager::tr(e.valueToKey(i)));
    }

    ui->cbWinState->addItems({tr("Normal"), tr("Maximized"), tr("FullScreen")});

    connect(ui->buttonBox, &QDialogButtonBox::clicked, this,
            &EditorSettingDialog::on_buttonBox_clicked);

    buildUpContent(widget);

    reload();

    setWindowTitle(tr("Setting"));
}

EditorSettingDialog::~EditorSettingDialog() { delete ui; }

void EditorSettingDialog::reload() {
    auto &set = SettingManager::instance();

    auto langs = LanguageManager::instance().langsDisplay();
    ui->cbLanguage->addItems(langs);
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
}

void EditorSettingDialog::on_buttonBox_clicked(QAbstractButton *button) {
    if (button == ui->buttonBox->button(QDialogButtonBox::Ok) ||
        button == ui->buttonBox->button(QDialogButtonBox::Apply)) {
        auto &set = SettingManager::instance();
        // TODO only one translation
        // set.setDefaultLang(LanguageManager::instance().langsDisplay());
        set.setThemeID(ui->cbTheme->currentIndex());
        auto s = ui->cbWinState->currentIndex();
        Qt::WindowState state;
        switch (s) {
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
        set.setDefaultWinState(state);
        set.setUseNativeTitleBar(ui->cbNativeTitile->isChecked());
        set.save();
        if (button == ui->buttonBox->button(QDialogButtonBox::Ok)) {
            done(1);
        }
    } else if (button == ui->buttonBox->button(QDialogButtonBox::Cancel)) {
        done(0);
    } else if (button ==
               ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)) {
        SettingManager::instance().reset();
        reload();
    }
}
