#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "QWingRibbon/ribbon.h"
#include "QWingRibbon/ribbonbuttongroup.h"
#include "class/gifcontentmodel.h"
#include "class/playgifmanager.h"
#include "class/recentfilemanager.h"
#include "control/gifcontentgallery.h"
#include "control/gifeditor.h"
#include "dialog/cropgifdialog.h"
#include "dialog/framelessmainwindow.h"
#include "plugin/pluginsystem.h"
#include "utilities.h"

#include <QMainWindow>
#include <QShortcut>
#include <QUndoStack>

class MainWindow : public FramelessMainWindow {
    Q_OBJECT

    friend class PluginSystem;

public:
    enum ToolButtonIndex : uint { UNDO_ACTION, REDO_ACTION };

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void openGif(const QString &filename);

private:
    template <typename Func>
    inline QToolButton *
    addPannelAction(RibbonButtonGroup *pannel, const QString &iconName,
                    const QString &title, Func &&slot,
                    const QKeySequence &shortcut = QKeySequence(),
                    QMenu *menu = nullptr) {
        return addPannelAction(pannel, ICONRES(iconName), title, slot, shortcut,
                               menu);
    }

    template <typename Func>
    inline QToolButton *
    addPannelAction(RibbonButtonGroup *pannel, const QIcon &icon,
                    const QString &title, Func &&slot,
                    const QKeySequence &shortcut = QKeySequence(),
                    QMenu *menu = nullptr) {
        Q_ASSERT(pannel);
        auto a = new QToolButton(pannel);
        a->setText(title);
        a->setIcon(icon);
        a->setToolTip(
            shortcut.isEmpty()
                ? QStringLiteral("<p align=\"center\">%1</p>").arg(title)
                : QStringLiteral(
                      "<p align=\"center\">%1</p><p align=\"center\">%2</p>")
                      .arg(title, shortcut.toString()));

        if (!shortcut.isEmpty()) {
            auto shortCut = new QShortcut(shortcut, this);
            shortCut->setContext(Qt::WindowShortcut);
            connect(shortCut, &QShortcut::activated, a, &QToolButton::click);
        }

        a->setMenu(menu);
        if (menu) {
            a->setPopupMode(QToolButton::InstantPopup);
        }
        connect(a, &QToolButton::clicked, this, slot);
        pannel->addButton(a);
        return a;
    }

    template <typename Func>
    inline QToolButton *
    addCheckablePannelAction(RibbonButtonGroup *pannel, const QString &iconName,
                             const QString &title, Func &&slot,
                             const QKeySequence &shortcut = QKeySequence()) {
        Q_ASSERT(pannel);
        auto a = new QToolButton(pannel);
        a->setText(title);
        a->setIcon(ICONRES(iconName));
        a->setCheckable(true);
        a->setChecked(false);

        if (!shortcut.isEmpty()) {
            auto shortCut = new QShortcut(shortcut, this);
            shortCut->setContext(Qt::WindowShortcut);
            connect(shortCut, &QShortcut::activated, a, &QToolButton::click);
        }

        connect(a, &QToolButton::toggled, this, slot);
        pannel->addButton(a);
        return a;
    }

private:
    void buildUpRibbonBar();

private slots:
    void on_new_frompics();
    void on_new_fromgifs();
    void on_open();
    void on_save();
    void on_saveas();
    void on_export();
    void on_close();
    void on_setting();

    void on_undo();
    void on_redo();
    void on_copy();
    void on_cut();
    void on_paste();
    void on_del();
    void on_selall();
    void on_selreverse();
    void on_desel();
    void on_goto();

    void on_beginframe();
    void on_last();
    void on_play();
    void on_stop();
    void on_next();
    void on_endframe();

    void on_decreaseframe();
    void on_delbefore();
    void on_delafter();
    void on_reverse();
    void on_moveleft();
    void on_moveright();
    void on_createreverse();
    void on_setdelay();
    void on_scaledelay();
    void on_insertpic();
    void on_merge();

    void on_scalepic();
    void on_crop();
    void on_fliph();
    void on_flipv();
    void on_clockwise();
    void on_anticlockwise();
    void on_exportapply();
    void on_applypic();

    void on_fullscreen();
    void on_about();
    void on_sponsor();
    void on_wiki();

private:
    RibbonTabContent *buildFilePage(RibbonTabContent *tab);
    RibbonTabContent *buildEditPage(RibbonTabContent *tab);
    RibbonTabContent *buildViewPage(RibbonTabContent *tab);
    RibbonTabContent *buildPluginPage(RibbonTabContent *tab);
    RibbonTabContent *buildSettingPage(RibbonTabContent *tab);
    RibbonTabContent *buildAboutPage(RibbonTabContent *tab);

    bool readGif(const QString &gif);
    bool writeGif(const QString &gif, const QString &comment = QString());
    bool exportGifFrames(const QString &dirPath, const char *ext);
    bool loadfromImages(const QStringList &filenames, int newInterval,
                        qsizetype index = 0, QSize size = QSize());
    bool loadfromGifs(QStringList gifs, qsizetype index = 0,
                      QSize size = QSize());

    void setSaved(bool b);

    bool ensureSafeClose();

    bool checkIsGif(QString filename);

    void setEditModeEnabled(bool b);
    void updatePlayState();

    void loadCacheIcon();

    void updateGifMessage();

    int getNewFrameInterval();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ribbon *m_ribbon = nullptr;
    QStatusBar *m_status = nullptr;

    PlayGifManager *_player = nullptr;
    GifEditor *_editor = nullptr;
    GifContentModel *_model = nullptr;
    GifContentGallery *_gallery = nullptr;
    QString _curfilename;
    QString _lastusedpath;

    QMenu *m_recentMenu = nullptr;
    RecentFileManager *m_recentmanager = nullptr;

    CropGifDialog *_cuttingdlg;

    RibbonTabContent *ribbonPlg = nullptr;
    RibbonTabContent *ribbonSetting = nullptr;

    QToolButton *_btnPlayerStop = nullptr;
    QList<QWidget *> _playDisWidgets;

    QList<QWidget *> m_editStateWidgets;
    QMap<ToolButtonIndex, QToolButton *> m_toolBtneditors;

    QUndoStack undo;

    QIcon _infoSaved;
    QIcon _infoUnsaved;

    QToolButton *m_iSaved = nullptr;
};
#endif // MAINWINDOW_H
