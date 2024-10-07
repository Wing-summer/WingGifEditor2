#ifndef IWINGPLUGIN_H
#define IWINGPLUGIN_H

#include <QList>
#include <QMenu>
#include <QObject>
#include <QToolBar>
#include <QToolButton>
#include <QUndoCommand>
#include <QWidget>
#include <QtCore>

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

/**
 * Don't try to modify this file, unless you are the dev
 * 不要尝试来修改该文件，除非你是开发者
 */

namespace WingGif {

Q_DECL_UNUSED constexpr auto SDKVERSION = 1;

Q_DECL_UNUSED static QString GETPLUGINQM(const QString &name) {
    return QCoreApplication::applicationDirPath() +
           QStringLiteral("/plglang/") + name;
};

Q_DECL_UNUSED static QString PLUGINDIR() {
    return QCoreApplication::applicationDirPath() + QStringLiteral("/plugin");
}

Q_DECL_UNUSED static QString HOSTRESPIMG(const QString &name) {
    return QStringLiteral(":/com.wingsummer.winggif/images/") + name +
           QStringLiteral(".png");
}

class IWingPlugin;

namespace WingPlugin {

class MessageBox : public QObject {
    Q_OBJECT
signals:
    void aboutQt(QWidget *parent = nullptr, const QString &title = QString());

    QMessageBox::StandardButton information(
        QWidget *parent, const QString &title, const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    QMessageBox::StandardButton question(
        QWidget *parent, const QString &title, const QString &text,
        QMessageBox::StandardButtons buttons =
            QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    QMessageBox::StandardButton
    warning(QWidget *parent, const QString &title, const QString &text,
            QMessageBox::StandardButtons buttons = QMessageBox::Ok,
            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    QMessageBox::StandardButton
    critical(QWidget *parent, const QString &title, const QString &text,
             QMessageBox::StandardButtons buttons = QMessageBox::Ok,
             QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    void about(QWidget *parent, const QString &title, const QString &text);

    QMessageBox::StandardButton
    msgbox(QWidget *parent, QMessageBox::Icon icon, const QString &title,
           const QString &text,
           QMessageBox::StandardButtons buttons = QMessageBox::NoButton,
           QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
};

class InputBox : public QObject {
    Q_OBJECT
signals:
    QString getText(QWidget *parent, const QString &title, const QString &label,
                    QLineEdit::EchoMode echo = QLineEdit::Normal,
                    const QString &text = QString(), bool *ok = nullptr,
                    Qt::InputMethodHints inputMethodHints = Qt::ImhNone);
    QString
    getMultiLineText(QWidget *parent, const QString &title,
                     const QString &label, const QString &text = QString(),
                     bool *ok = nullptr,
                     Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

    QString getItem(QWidget *parent, const QString &title, const QString &label,
                    const QStringList &items, int current = 0,
                    bool editable = true, bool *ok = nullptr,
                    Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

    int getInt(QWidget *parent, const QString &title, const QString &label,
               int value = 0, int minValue = -2147483647,
               int maxValue = 2147483647, int step = 1, bool *ok = nullptr);

    double getDouble(QWidget *parent, const QString &title,
                     const QString &label, double value = 0,
                     double minValue = -2147483647,
                     double maxValue = 2147483647, int decimals = 1,
                     bool *ok = nullptr, double step = 1);
};

} // namespace WingPlugin

struct WingPluginInfo {
    QString pluginName;
    QString pluginAuthor;
    uint pluginVersion;
    QString puid;
    QString pluginComment;
};

const auto WINGSUMMER = QStringLiteral("wingsummer");

class IWingPlugin : public QObject {
    Q_OBJECT
public:
    virtual int sdkVersion() const = 0;
    virtual const QString signature() const = 0;
    virtual ~IWingPlugin() = default;

    virtual bool init(const QList<WingPluginInfo> &loadedplugin) = 0;
    virtual void unload() = 0;
    virtual const QString pluginName() const = 0;
    virtual const QString pluginAuthor() const = 0;
    virtual uint pluginVersion() const = 0;
    virtual const QString pluginComment() const = 0;

    virtual QList<QToolButton *> registeredRibbonTools() const { return {}; }
    virtual QList<QToolButton *> registeredRibbonSettings() const { return {}; }

    virtual void onPreviewGifFrame(QImage &frame, qsizetype index, int delay) {
        Q_UNUSED(frame);
        Q_UNUSED(index);
        Q_UNUSED(delay);
    };

signals:
    // extension
    void toast(const QPixmap &icon, const QString &message);
    void trace(const QString &message);
    void debug(const QString &message);
    void warn(const QString &message);
    void error(const QString &message);
    void info(const QString &message);

    // basic
    bool isEditing();
    bool addFrame(qsizetype index, const QImage &frame, int delay);
    void removeFrame(qsizetype index, qsizetype count = 1);
    void clearFrame();
    QImage getFrame(qsizetype index);
    int getFrameDelay(qsizetype index);
    bool replaceFrame(qsizetype index, const QImage &image);
    bool replaceFrameDelay(qsizetype index, int delay);
    qsizetype frameCount();
    void scaleGif(int width, int height);

    // if you want to support undo/redo,
    // here is the stack, just push it
    bool pushCommand(QUndoCommand *command);

    // setting dialog
    QDialog *createSettingDialog(QWidget *content);

public:
    WingPlugin::MessageBox msgbox;
    WingPlugin::InputBox inputbox;
};

} // namespace WingGif

constexpr auto IWINGPLUGIN_INTERFACE_IID = "com.wingsummer.iwingplugin";

Q_DECLARE_INTERFACE(WingGif::IWingPlugin, IWINGPLUGIN_INTERFACE_IID)

#endif // IWINGPLUGIN_H
