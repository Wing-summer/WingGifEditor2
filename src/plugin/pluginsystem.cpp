#include "pluginsystem.h"

#include "QWingRibbon/ribbontabcontent.h"
#include "class/logger.h"
#include "class/winginputdialog.h"
#include "class/wingmessagebox.h"
#include "control/toast.h"
#include "dialog/mainwindow.h"

PluginSystem::PluginSystem(QObject *parent) : QObject(parent) {}

PluginSystem::~PluginSystem() {
    for (auto &item : loadedplgs) {
        item->unload();
        delete item;
    }
}

PluginSystem &PluginSystem::instance() {
    static PluginSystem ins;
    return ins;
}

void PluginSystem::setMainWindow(MainWindow *win) { _win = win; }

QWidget *PluginSystem::mainWindow() const { return _win; }

const QList<IWingPlugin *> &PluginSystem::plugins() const { return loadedplgs; }

const IWingPlugin *PluginSystem::plugin(qsizetype index) const {
    return loadedplgs.at(index);
}

bool PluginSystem::checkThreadAff() {
    if (QThread::currentThread() != qApp->thread()) {
        Logger::warning(
            tr("Creating UI widget is not allowed in non-UI thread"));
        return false;
    }
    return true;
}

void PluginSystem::connectUIInterface(IWingPlugin *plg) {
    auto msgbox = &plg->msgbox;
    connect(msgbox, &WingPlugin::MessageBox::aboutQt, _win,
            [this](QWidget *parent, const QString &title) -> void {
                if (checkThreadAff()) {
                    WingMessageBox::aboutQt(parent, title);
                }
            });
    connect(msgbox, &WingPlugin::MessageBox::information, _win,
            [=](QWidget *parent, const QString &title, const QString &text,
                QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton)
                -> QMessageBox::StandardButton {
                if (checkThreadAff()) {
                    return WingMessageBox::information(parent, title, text,
                                                       buttons, defaultButton);
                }
                return QMessageBox::StandardButton::NoButton;
            });
    connect(msgbox, &WingPlugin::MessageBox::question, _win,
            [=](QWidget *parent, const QString &title, const QString &text,
                QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton)
                -> QMessageBox::StandardButton {
                if (checkThreadAff()) {
                    return WingMessageBox::question(parent, title, text,
                                                    buttons, defaultButton);
                }
                return QMessageBox::StandardButton::NoButton;
            });
    connect(msgbox, &WingPlugin::MessageBox::warning, _win,
            [=](QWidget *parent, const QString &title, const QString &text,
                QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton)
                -> QMessageBox::StandardButton {
                if (checkThreadAff()) {
                    return WingMessageBox::warning(parent, title, text, buttons,
                                                   defaultButton);
                }
                return QMessageBox::StandardButton::NoButton;
            });
    connect(msgbox, &WingPlugin::MessageBox::critical, _win,
            [=](QWidget *parent, const QString &title, const QString &text,
                QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton)
                -> QMessageBox::StandardButton {
                if (checkThreadAff()) {
                    return WingMessageBox::critical(parent, title, text,
                                                    buttons, defaultButton);
                }
                return QMessageBox::StandardButton::NoButton;
            });
    connect(msgbox, &WingPlugin::MessageBox::about, _win,
            [=](QWidget *parent, const QString &title,
                const QString &text) -> void {
                if (checkThreadAff()) {
                    WingMessageBox::about(parent, title, text);
                }
            });
    connect(msgbox, &WingPlugin::MessageBox::msgbox, _win,
            [=](QWidget *parent, QMessageBox::Icon icon, const QString &title,
                const QString &text, QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton)
                -> QMessageBox::StandardButton {
                if (checkThreadAff()) {
                    return WingMessageBox::msgbox(parent, icon, title, text,
                                                  buttons, defaultButton);
                }
                return QMessageBox::StandardButton::NoButton;
            });

    auto inputbox = &plg->inputbox;
    connect(inputbox, &WingPlugin::InputBox::getText, _win,
            [=](QWidget *parent, const QString &title, const QString &label,
                QLineEdit::EchoMode echo, const QString &text, bool *ok,
                Qt::InputMethodHints inputMethodHints) -> QString {
                if (checkThreadAff()) {
                    return WingInputDialog::getText(parent, title, label, echo,
                                                    text, ok, inputMethodHints);
                }
                return {};
            });
    connect(inputbox, &WingPlugin::InputBox::getMultiLineText, _win,
            [=](QWidget *parent, const QString &title, const QString &label,
                const QString &text, bool *ok,
                Qt::InputMethodHints inputMethodHints) -> QString {
                if (checkThreadAff()) {
                    return WingInputDialog::getMultiLineText(
                        parent, title, label, text, ok, inputMethodHints);
                }
                return {};
            });
    connect(inputbox, &WingPlugin::InputBox::getItem, _win,
            [=](QWidget *parent, const QString &title, const QString &label,
                const QStringList &items, int current, bool editable, bool *ok,
                Qt::InputMethodHints inputMethodHints) -> QString {
                if (checkThreadAff()) {
                    return WingInputDialog::getItem(parent, title, label, items,
                                                    current, editable, ok,
                                                    inputMethodHints);
                }
                return {};
            });
    connect(
        inputbox, &WingPlugin::InputBox::getInt, _win,
        [=](QWidget *parent, const QString &title, const QString &label,
            int value, int minValue, int maxValue, int step, bool *ok) -> int {
            if (checkThreadAff()) {
                return WingInputDialog::getInt(parent, title, label, value,
                                               minValue, maxValue, step, ok);
            }
            return 0;
        });
    connect(inputbox, &WingPlugin::InputBox::getDouble, _win,
            [=](QWidget *parent, const QString &title, const QString &label,
                double value, double minValue, double maxValue, int decimals,
                bool *ok, double step) -> double {
                if (checkThreadAff()) {
                    return WingInputDialog::getDouble(parent, title, label,
                                                      value, minValue, maxValue,
                                                      decimals, ok, step);
                }
                return qQNaN();
            });
}

bool PluginSystem::loadPlugin(IWingPlugin *p) {
    QList<WingPluginInfo> loadedplginfos;
    try {

        if (p->signature() != WINGSUMMER) {
            throw tr("ErrLoadPluginSign");
        }

        if (p->sdkVersion() != SDKVERSION) {
            throw tr("ErrLoadPluginSDKVersion");
        }

        if (!p->pluginName().trimmed().length()) {
            throw tr("ErrLoadPluginNoName");
        }

        auto prop = p->property("puid").toString().trimmed();
        auto puid =
            prop.isEmpty() ? QString(p->metaObject()->className()) : prop;
        if (loadedpuid.contains(puid)) {
            throw tr("ErrLoadLoadedPlugin");
        }

        if (!p->init(loadedplginfos)) {
            throw tr("ErrLoadInitPlugin");
        }

        WingPluginInfo info;
        info.puid = puid;
        info.pluginName = p->pluginName();
        info.pluginAuthor = p->pluginAuthor();
        info.pluginComment = p->pluginComment();
        info.pluginVersion = p->pluginVersion();

        loadedplginfos.push_back(info);
        loadedplgs.push_back(p);
        loadedpuid << puid;

        Logger::warning(tr("PluginName :") + info.pluginName);
        Logger::warning(tr("PluginAuthor :") + info.pluginAuthor);
        Logger::warning(tr("PluginWidgetRegister"));

        auto ribbonToolboxInfos = p->registeredRibbonTools();
        if (!ribbonToolboxInfos.isEmpty()) {
            auto cat = _win->ribbonPlg;
            auto group = cat->addGroup(info.pluginName);
            for (auto &item : ribbonToolboxInfos) {
                group->addButton(item);
            }
        }

        auto ribbonSettings = p->registeredRibbonSettings();
        if (!ribbonSettings.isEmpty()) {
            auto cat = _win->ribbonSetting;
            auto group = cat->addGroup(info.pluginName);
            for (auto &item : ribbonSettings) {
                group->addButton(item);
            }
        }

        connectInterface(p);
    } catch (const QString &error) {
        Logger::critical(error);
        return false;
    }
    return true;
}

void PluginSystem::connectBaseInterface(IWingPlugin *plg) {
    connect(plg, &IWingPlugin::toast, this,
            [=](const QPixmap &icon, const QString &message) {
                if (message.isEmpty()) {
                    return;
                }
                Toast::toast(_win, icon, message);
            });
    connect(plg, &IWingPlugin::trace, this, [=](const QString &message) {
        Logger::trace(packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPlugin::debug, this, [=](const QString &message) {
        Logger::debug(packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPlugin::info, this, [=](const QString &message) {
        Logger::info(packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPlugin::warn, this, [=](const QString &message) {
        Logger::warning(
            packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPlugin::error, this, [=](const QString &message) {
        Logger::critical(
            packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPlugin::addFrame, this,
            [=](qsizetype index, const QImage &frame, int delay) -> bool {
                return _win->_model->insertFrame(frame, delay, index);
            });
    connect(plg, &IWingPlugin::removeFrame, this,
            [=](qsizetype index, qsizetype count) {
                _win->_model->removeFrames(index, count);
            });
    connect(plg, &IWingPlugin::clearFrame, this,
            [=]() { _win->_model->clearData(); });
    connect(plg, &IWingPlugin::getFrame, this, [=](qsizetype index) -> QImage {
        return _win->_model->image(index);
    });
    connect(plg, &IWingPlugin::getFrameDelay, this,
            [=](qsizetype index) -> int { return _win->_model->delay(index); });
    connect(plg, &IWingPlugin::replaceFrameDelay, this,
            [=](qsizetype index, int delay) {
                _win->_model->setFrameDelay(index, delay);
            });
    connect(plg, &IWingPlugin::replaceFrame, this,
            [=](qsizetype index, const QImage &image) {
                return _win->_model->setFrameImage(index, image);
            });
    connect(plg, &IWingPlugin::frameCount, this,
            [=]() -> qsizetype { return _win->_model->frameCount(); });
    connect(plg, &IWingPlugin::pushCommand, this,
            [=](QUndoCommand *command) -> bool {
                if (_win->_curfilename.isEmpty()) {
                    return false;
                }
                _win->undo.push(command);
                return true;
            });
    connect(plg, &IWingPlugin::createSettingDialog, this,
            [=](QWidget *content) -> QDialog * {
                if (content == nullptr) {
                    return nullptr;
                }
                auto d = new FramelessDialogBase;
                d->buildUpContent(content);
                return d;
            });
}

void PluginSystem::connectInterface(IWingPlugin *plg) {
    connectBaseInterface(plg);
    connectUIInterface(plg);
}

QString PluginSystem::packLogMessage(const char *header, const QString &msg) {
    return QStringLiteral("{") + header + QStringLiteral("} ") + msg;
}