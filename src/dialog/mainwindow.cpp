#include "mainwindow.h"

#include <QSplitter>
#include <QStatusBar>

#include "QWingRibbon/ribbontabcontent.h"
#include "class/appmanager.h"
#include "class/clipboardhelper.h"
#include "class/gifcontentmodel.h"
#include "class/gifreader.h"
#include "class/gifwriter.h"
#include "class/logger.h"
#include "class/qkeysequences.h"
#include "class/settingmanager.h"
#include "class/waitingloop.h"
#include "class/winginputdialog.h"
#include "class/wingmessagebox.h"
#include "command/cropimagecommand.h"
#include "command/delayframecommand.h"
#include "command/delframedircommand.h"
#include "command/flipframecommand.h"
#include "command/insertframecommand.h"
#include "command/moveframecommand.h"
#include "command/reduceframecommand.h"
#include "command/removeframecommand.h"
#include "command/replaceframecommand.h"
#include "command/reverseframecommand.h"
#include "command/rotateframecommand.h"
#include "command/scaleframecommand.h"
#include "control/gifcontentgallery.h"
#include "control/toast.h"
#include "dialog/aboutsoftwaredialog.h"
#include "dialog/createreversedialog.h"
#include "dialog/exportdialog.h"
#include "dialog/fileinfodialog.h"
#include "dialog/gifsavedialog.h"
#include "dialog/newdialog.h"
#include "dialog/reduceframedialog.h"
#include "dialog/scalegifdialog.h"
#include "settings/editorsettingdialog.h"
#include "settings/pluginsettingdialog.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QPainter>

constexpr auto EMPTY_FUNC = [] {};

MainWindow::MainWindow(QWidget *parent) : FramelessMainWindow(parent) {
    this->setUpdatesEnabled(false);

    // recent file manager init
    m_recentMenu = new QMenu(this);
    m_recentmanager = new RecentFileManager(m_recentMenu);
    connect(
        m_recentmanager, &RecentFileManager::triggered, this,
        [=](const QString &info) { AppManager::instance()->openFile(info); });
    m_recentmanager->apply(this, SettingManager::instance().recentFiles());

    // build up UI
    buildUpRibbonBar();

    auto cw = new QWidget(this);
    cw->setSizePolicy(
        QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    auto layout = new QVBoxLayout(cw);
    layout->setContentsMargins(1, 0, 1, 0);
    layout->setSpacing(0);
    layout->addWidget(q_check_ptr(m_ribbon));

    auto splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);

    _editor = new GifEditor(QImage(NAMEICONRES("icon")), this);
    splitter->addWidget(_editor);
    _gallery = new GifContentGallery(this);
    splitter->addWidget(_gallery);
    _model = new GifContentModel(_gallery);
    _model->setLinkedListView(_gallery);
    _gallery->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _gallery->setMaximumHeight(300);
    connect(_gallery->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this,
            [=](const QModelIndex &current, const QModelIndex & /*previous*/) {
                auto i = current.row();
                auto img = _model->image(i);
                auto delay = _model->delay(i);
                PluginSystem::instance().callPluginProcess(img, delay, i);
                _editor->setImage(img);
                updateGifMessage();
            });
    connect(_model, &QAbstractListModel::rowsInserted, this,
            &MainWindow::updateGifMessage);
    connect(_model, &QAbstractListModel::rowsRemoved, this,
            &MainWindow::updateGifMessage);

    layout->addWidget(splitter, 1);

    m_status = new QStatusBar(this);

    // init statusbar
    loadCacheIcon();
    auto disableStyle = QStringLiteral("border:none;background:transparent;");
    m_iSaved = new QToolButton(m_status);
    m_iSaved->setIcon(_infoSaved);
    m_iSaved->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_iSaved->setToolTip(tr("InfoSave"));
    m_iSaved->setStyleSheet(disableStyle);

    m_status->addPermanentWidget(m_iSaved);

    layout->addWidget(m_status);

    buildUpContent(cw);

    _player = new PlayGifManager(this);
    connect(_player, &PlayGifManager::tick, this, [=](int index) {
        auto i = _model->index(index);
        _gallery->setCurrentIndex(i);
    });
    connect(_player, &PlayGifManager::playStateChanged, this,
            &MainWindow::updatePlayState);

    connect(&undo, &QUndoStack::canUndoChanged, this, [=](bool b) {
        m_toolBtneditors.value(ToolButtonIndex::UNDO_ACTION)->setEnabled(b);
    });

    connect(&undo, &QUndoStack::canRedoChanged, this, [=](bool b) {
        m_toolBtneditors.value(ToolButtonIndex::REDO_ACTION)->setEnabled(b);
    });
    connect(&undo, &QUndoStack::cleanChanged, this, [=](bool clean) {
        this->setSaved(clean && _curfilename != QStringLiteral(":"));
    });

    _cuttingdlg = new CropGifDialog(this);
    connect(_cuttingdlg, &CropGifDialog::selRectChanged, _editor,
            &GifEditor::setSelRect);
    connect(_cuttingdlg, &CropGifDialog::crop, this,
            [=](qreal x, qreal y, qreal w, qreal h) {
                _editor->setCropMode(false);
                this->setEditModeEnabled(true);
                if (x < 0 || y < 0 || w < 0 || h < 0) {
                    return;
                }
                undo.push(new CropImageCommand(_model, QRect(x, y, w, h)));
            });
    connect(_cuttingdlg, &CropGifDialog::pressCancel, this, [=] {
        _editor->setCropMode(false);
        this->setEditModeEnabled(true);
    });
    connect(_editor, &GifEditor::selRectChanged, _cuttingdlg,
            &CropGifDialog::setSelRect);

    _logdialog = new LogDialog(this);
    auto log = Logger::instance();
    log->setLogLevel(Logger::Level::q5TRACE);
    connect(log, &Logger::log, _logdialog, &LogDialog::log);

    auto &plg = PluginSystem::instance();
    plg.setMainWindow(this);
    plg.LoadPlugin();

    setEditModeEnabled(false);

    setWindowTitle(tr("WingGifEditor"));
    setWindowIcon(ICONRES("icon"));

    this->setUpdatesEnabled(true);

    // Don't call without QTimer::singleShot.
    // I don't know why it doesn't work with direct call.
    QTimer::singleShot(0, this, [this] {
        auto &set = SettingManager::instance();
        switch (set.defaultWinState()) {
        case Qt::WindowNoState:
            break;
        case Qt::WindowMinimized:
            this->showMinimized();
            break;
        case Qt::WindowActive:
        case Qt::WindowMaximized:
            this->showMaximized();
            break;
        case Qt::WindowFullScreen:
            this->showFullScreen();
            break;
        }
    });
}

MainWindow::~MainWindow() {}

void MainWindow::buildUpRibbonBar() {
    m_ribbon = new Ribbon(this);
    buildFilePage(m_ribbon->addTab(tr("File")));
    auto a = buildEditPage(m_ribbon->addTab(tr("Edit")));
    m_editStateWidgets << a;
    _playDisWidgets << a;
    m_editStateWidgets << buildViewPage(m_ribbon->addTab(tr("View")));
    ribbonPlg = buildPluginPage(m_ribbon->addTab(tr("Plugin")));
    _playDisWidgets << ribbonPlg;
    ribbonSetting = buildSettingPage(m_ribbon->addTab(tr("Setting")));
    _playDisWidgets << ribbonSetting;
    _playDisWidgets << buildAboutPage(m_ribbon->addTab(tr("About")));
    connect(m_ribbon, &Ribbon::onDragDropFiles, this,
            [=](const QStringList &files) {
                if (files.size() == 1) {
                    readGif(files.first());
                    return;
                }
                bool ok;
                auto f = WingInputDialog::getItem(this, tr("ChooseFile"),
                                                  tr("Choose to open"), files,
                                                  0, false, &ok);
                if (ok) {
                    readGif(f);
                }
            });
}

void MainWindow::on_new_frompics() {
    _player->stop();
    if (ensureSafeClose()) {
        NewDialog d(NewType::FromPics, this);
        if (d.exec()) {
            WingProgressDialog dw;
            dw.dialog()->setCancelButton(nullptr);
            dw.dialog()->setLabelText(tr("NewFromPicsGif"));
            dw.dialog()->setRange(0, 0);
            if (loadfromImages(d.getResult(), getNewFrameInterval())) {
                _curfilename = QStringLiteral(":"); // 表示新建
                setSaved(false);

                _gallery->setCurrentIndex(_model->index(0));
                setEditModeEnabled(true);
            } else {
            }
            dw.pdialog()->close();
        }
    }
}

void MainWindow::on_new_fromgifs() {
    _player->stop();
    if (ensureSafeClose()) {
        NewDialog d(NewType::FromGifs, this);
        if (d.exec()) {
            // WaitingDialog dw;
            // dw.start(tr("NewFromGifsGif"));
            if (loadfromGifs(d.getResult())) {
                _curfilename = QStringLiteral(":"); // 表示新建
                setSaved(false);

                _gallery->setCurrentIndex(_model->index(0));
                setEditModeEnabled(true);
            } else {
            }
            // dw.close();
        }
    }
}

void MainWindow::on_open() {
    _player->stop();
    if (ensureSafeClose()) {
        auto filename = QFileDialog::getOpenFileName(
            this, tr("ChooseFile"), _lastusedpath, "gif (*.gif)");
        if (filename.isEmpty())
            return;

        _lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();

        openGif(filename);
    }
}

void MainWindow::on_save() {
    _player->stop();
    if (_curfilename == QStringLiteral(":")) {
        on_saveas();
        return;
    }

    GifSaveDialog d;
    GifSaveResult r;
    if (d.exec()) {
        r = d.getResult();
    } else {
        Toast::toast(this, NAMEICONRES("save"), tr("CaneledByUser"));
        return;
    }

    WingProgressDialog dw;
    dw.dialog()->setCancelButton(nullptr);
    dw.dialog()->setLabelText(tr("SaveGif"));
    dw.dialog()->setRange(0, 0);

    if (writeGif(_curfilename, r.loop, r.comment)) {
        undo.setClean();
        Toast::toast(this, NAMEICONRES("save"), tr("SaveSuccess"));
    } else {
        Toast::toast(this, NAMEICONRES("save"), tr("SaveFail"));
    }
}

void MainWindow::on_saveas() {
    _player->stop();

    GifSaveDialog d;
    GifSaveResult r;
    if (d.exec()) {
        r = d.getResult();
    } else {
        Toast::toast(this, NAMEICONRES("saveas"), tr("CaneledByUser"));
        return;
    }

    auto filename =
        QFileDialog::getSaveFileName(this, tr("ChooseSaveFile"), _lastusedpath,
                                     QStringLiteral("gif (*.gif)"));
    if (filename.isEmpty())
        return;
    _lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();

    WaitingLoop wl(tr("SaveAsGif"));

    if (writeGif(filename, r.loop, r.comment)) {
        Toast::toast(this, NAMEICONRES("saveas"), tr("SaveAsSuccess"));
        _curfilename = filename;
        m_recentmanager->addRecentFile(filename);
    } else {
        Toast::toast(this, NAMEICONRES("saveas"), tr("SaveAsFail"));
    }
}

void MainWindow::on_export() {
    _player->stop();

    ExportDialog d;
    if (d.exec()) {
        auto res = d.getResult();
        const char *ext = nullptr;
        switch (res.type) {
        case ExportImageType::PNG: {
            ext = "png";
            break;
        }
        case ExportImageType::JPG: {
            ext = "jpg";
            break;
        }
        case ExportImageType::TIFF: {
            ext = "tiff";
            break;
        }
        case ExportImageType::WEBP: {
            ext = "webp";
            break;
        }
        }

        WaitingLoop wl(tr("ExportFrames"));

        if (exportGifFrames(res.path, ext)) {
            Toast::toast(this, NAMEICONRES("saveas"), tr("ExportSuccess"));
        } else {
            Toast::toast(this, NAMEICONRES("saveas"), tr("ExportFail"));
        }
    }
}

void MainWindow::on_close() {
    _player->stop();
    if (ensureSafeClose()) {

        // editor->setBackgroudPix(QPixmap(":/images/icon.png"));
        // editor->fitPicEditor();
        // editor->scale(0.5, 0.5);
        // iSaved->setPixmap(infoSaveg);
        // iReadWrite->setPixmap(inforwg);
        // status->showMessage("");

        _model->clearData();

        _curfilename.clear();
        undo.clear();
        setEditModeEnabled(false);
    }
}

void MainWindow::on_setting() {}

void MainWindow::on_undo() {
    _player->stop();
    undo.undo();
}

void MainWindow::on_redo() {
    _player->stop();
    undo.redo();
}

void MainWindow::on_copy() {
    _player->stop();
    auto sels = _gallery->selectionModel()->selectedRows();
    QVector<GifData> sel;
    for (auto &i : sels) {
        auto index = i.row();
        GifData d;
        d.delay = _model->delay(index);
        d.image = _model->image(index);
        sel.append(d);
    }
    ClipBoardHelper::setImageFrames(sel);
}

void MainWindow::on_cut() {
    _player->stop();
    auto sels = _gallery->selectionModel()->selectedRows();
    QVector<GifData> sel;
    QVector<int> indices;
    for (auto &i : sels) {
        GifData d;
        auto index = i.row();
        indices.append(index);

        d.delay = _model->delay(index);
        d.image = _model->image(i.row());

        sel.append(d);
    }
    ClipBoardHelper::setImageFrames(sel);
    undo.push(new RemoveFrameCommand(_model, indices));
}

void MainWindow::on_paste() {
    _player->stop();
    auto pos = _gallery->currentIndex().row() + 1;
    QVector<GifData> imgs;
    ClipBoardHelper::getImageFrames(imgs);
    if (imgs.count()) {
        undo.push(new InsertFrameCommand(_model, pos, imgs));
    }
}

void MainWindow::on_del() {
    _player->stop();
    QVector<int> indices;
    for (auto &item : _gallery->selectionModel()->selectedIndexes()) {
        indices.append(item.row());
    }
    undo.push(new RemoveFrameCommand(_model, indices));
}

void MainWindow::on_selall() {
    _player->stop();
    _gallery->selectAll();
}

void MainWindow::on_selreverse() {
    _player->stop();
    auto len = _model->frameCount();
    for (auto i = 0; i < len; i++) {
        auto sel = _gallery->selectionModel();
        auto index = _model->index(i);
        sel->select(index, QItemSelectionModel::Toggle);
    }
}

void MainWindow::on_desel() {
    _player->stop();
    _gallery->clearSelection();
}

void MainWindow::on_goto() {
    _player->stop();
    bool ok;
    auto index = WingInputDialog::getInt(
        this, tr("Goto"), tr("PleaseInputIndex"),
        _gallery->currentIndex().row() + 1, 1, _model->frameCount(), 1, &ok);
    if (ok) {
        _gallery->setCurrentIndex(_model->index(index - 1));
    }
}

void MainWindow::on_beginframe() {
    _player->stop();
    _gallery->setCurrentIndex(_model->index(0));
}

void MainWindow::on_last() {
    _player->stop();
    auto index = _gallery->currentIndex();
    if (index.row() == 0) {
        index = _model->index(_model->frameCount() - 1);
    } else {
        index = _model->index(index.row() - 1);
    }
    _gallery->setCurrentIndex(index);
}

void MainWindow::on_play() {
    _player->setTickIntervals(_model->delays());
    _player->play(_gallery->currentIndex().row());
}

void MainWindow::on_stop() { _player->stop(); }

void MainWindow::on_next() {
    _player->stop();
    auto index = _gallery->currentIndex();
    if (index.row() >= _model->frameCount() - 1) {
        index = _model->index(0);
    } else {
        index = _model->index(index.row() + 1);
    }
    _gallery->setCurrentIndex(index);
}

void MainWindow::on_endframe() {
    _player->stop();
    _gallery->setCurrentIndex(_model->index(_model->frameCount() - 1));
}

void MainWindow::on_decreaseframe() {
    _player->stop();
    ReduceFrameDialog d(_model->frameCount(), this);
    if (d.exec()) {
        auto res = d.getResult();
        auto from = res.start;
        auto step = res.stepcount;
        auto to = res.end;
        QVector<int> delindices, modinter;
        auto ii = from;
        auto q = step + 1;
        for (auto i = ii; i <= to; i++) {
            if (i == ii + step) {
                ii += q;
                delindices.append(i);
            } else {
                modinter.append(_model->delay(i) * (q + 1) / q);
            }
        }

        undo.push(new ReduceFrameCommand(_model, delindices, modinter));
    }
}

void MainWindow::on_delbefore() {
    _player->stop();
    auto pos = _gallery->currentIndex().row();
    undo.push(new DelFrameDirCommand(_model, pos, DelDirection::Before));
}

void MainWindow::on_delafter() {
    _player->stop();
    auto pos = _gallery->currentIndex().row();
    undo.push(new DelFrameDirCommand(_model, pos, DelDirection::After));
}

void MainWindow::on_reverse() {
    _player->stop();
    undo.push(new ReverseFrameCommand(_model));
}

void MainWindow::on_moveleft() {
    _player->stop();

    auto sels = _gallery->selectionModel()->selectedRows();
    QVector<int> indices;
    for (auto &item : sels) {
        auto index = item.row();
        if (index == 0) {
            return;
        }
        indices.append(index);
    }

    undo.push(new MoveFrameCommand(
        _model, GifContentModel::MoveFrameDirection::Left, indices));
}

void MainWindow::on_moveright() {
    _player->stop();

    auto sels = _gallery->selectionModel()->selectedRows();
    QVector<int> indices;
    for (auto &item : sels) {
        auto index = item.row();
        if (index == _model->frameCount() - 1) {
            return;
        }
        indices.append(index);
    }

    undo.push(new MoveFrameCommand(
        _model, GifContentModel::MoveFrameDirection::Right, indices));
}

void MainWindow::on_createreverse() {
    if (_model->frameCount() == 0) {
        return;
    }

    _player->stop();
    CreateReverseDialog d(_model->frameCount(), this);
    if (d.exec()) {
        auto res = d.getResult();
        QVector<GifData> datas;
        for (int i = res.end; i >= res.start; --i) {
            GifData d;
            d.delay = _model->delay(i);
            d.image = _model->image(i);
            datas.append(d);
        }
        undo.push(new InsertFrameCommand(_model, res.end, datas));
    }
}

void MainWindow::on_setdelay() {
    if (_model->frameCount() == 0) {
        return;
    }

    _player->stop();

    auto mod = QGuiApplication::keyboardModifiers();
    auto indices = _gallery->selectionModel()->selectedRows();
    bool ok;
    bool isGlobal = mod == Qt::KeyboardModifier::ControlModifier;

    auto time = WingInputDialog::getInt(
        this, isGlobal ? tr("DelayTime") + tr("(Global)") : tr("DelayTime"),
        tr("Inputms"), _model->delay(0), 1, INT_MAX, 1, &ok);
    if (ok) {
        QVector<int> is;
        if (!isGlobal) {
            for (auto &i : indices) {
                is.append(i.row());
            }
        }
        undo.push(new DelayFrameCommand(_model, is, time));
    }
}

void MainWindow::on_scaledelay() {
    _player->stop();

    auto mod = QGuiApplication::keyboardModifiers();
    bool ok;
    bool isGlobal = mod == Qt::KeyboardModifier::ControlModifier;

    auto scale = WingInputDialog::getInt(
        this,
        isGlobal ? tr("ScaleDelayTime") + tr("(Global)") : tr("ScaleDelayTime"),
        tr("InputPercent"), 100, 1, 100, 1, &ok);

    QVector<int> is;
    if (ok) {
        if (!isGlobal) {
            auto indices = _gallery->selectionModel()->selectedRows();

            for (auto &i : indices) {
                is.append(i.row());
            }
        }
        undo.push(new DelayScaleCommand(_model, is, scale));
    }
}

void MainWindow::on_insertpic() {
    _player->stop();

    auto filenames =
        QFileDialog::getOpenFileNames(this, tr("ChooseFile"), _lastusedpath,
                                      tr("Images (*.jpg *.tiff *.png)"));
    if (filenames.isEmpty())
        return;
    _lastusedpath = QFileInfo(filenames.first()).absoluteDir().absolutePath();

    WaitingLoop wl(tr("InsertPics"));

    auto pos = _gallery->currentIndex().row() + 1;
    if (loadfromImages(filenames, pos, getNewFrameInterval(),
                       _model->frameSize())) {
        Toast::toast(this, NAMEICONRES("pics"), tr("InsertPicsSuccess"));
    }
}

void MainWindow::on_merge() {
    _player->stop();

    auto filenames = QFileDialog::getOpenFileNames(
        this, tr("ChooseFile"), _lastusedpath, "gif (*.gif)");
    if (filenames.isEmpty())
        return;
    _lastusedpath = QFileInfo(filenames.first()).absoluteDir().absolutePath();

    WaitingLoop wl(tr("InsertGifs"));

    auto pos = _gallery->currentIndex().row() + 1;
    if (loadfromGifs(filenames, pos, _model->frameSize())) {
        Toast::toast(this, NAMEICONRES("gifs"), tr("InsertGifsSuccess"));
    }
}

void MainWindow::on_scalepic() {
    _player->stop();
    ScaleGIFDialog d(_model->frameSize(), this);
    if (d.exec()) {
        auto res = d.getResult();
        undo.push(new ScaleFrameCommand(_model, res.width, res.height));
    }
}

void MainWindow::on_crop() {
    _player->stop();

    setEditModeEnabled(false);
    auto s = _model->frameSize();
    _cuttingdlg->setMaxSize(s);

    _editor->setSelRect(0, 0, s.width(), s.height());
    _cuttingdlg->setSelRect(_editor->selRect());

    _editor->setCropMode(true);

    _cuttingdlg->show();
}

void MainWindow::on_fliph() {
    _player->stop();
    undo.push(new FlipFrameCommand(_model, Qt::Horizontal));
}

void MainWindow::on_flipv() {
    _player->stop();
    undo.push(new FlipFrameCommand(_model, Qt::Vertical));
}

void MainWindow::on_clockwise() {
    _player->stop();
    undo.push(new RotateFrameCommand(_model, true));
}

void MainWindow::on_anticlockwise() {
    _player->stop();
    undo.push(new RotateFrameCommand(_model, false));
}

void MainWindow::on_exportapply() {
    _player->stop();
    auto filename = QFileDialog::getSaveFileName(this, tr("ChooseSaveFile"),
                                                 _lastusedpath, "png (*.png)");
    if (filename.isEmpty())
        return;
    _lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();
    QImage img(_model->frameSize(), QImage::Format_RGBA8888);
    img.fill(Qt::transparent);

    QPainter p(&img);
    p.drawImage(img.rect(), QImage(NAMEICONRES("icon")));

    img.save(filename);
    Toast::toast(this, NAMEICONRES("blank"), tr("ExportSuccess"));
}

void MainWindow::on_applypic() {
    _player->stop();

    if (QGuiApplication::keyboardModifiers() ==
        Qt::KeyboardModifier::ControlModifier) {

        auto filename = QFileDialog::getOpenFileName(
            this, tr("ChooseFile"), _lastusedpath, "png (*.png)");
        if (filename.isEmpty())
            return;

        _lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();

        QVector<QImage> imgs;
        QVector<int> empty;
        QImage img;
        if (img.load(filename)) {
            if (img.size() == _model->frameSize()) {
                auto frames = _model->images();
                for (auto &f : frames) {
                    QImage bimg = f.copy();
                    QPainter painter(&bimg);
                    painter.drawImage(QPoint(), img, img.rect());
                    imgs.append(bimg);
                }
                undo.push(new ReplaceFrameCommand(_model, empty, imgs));
                return;
            }
        }
    } else {
        auto indices = _gallery->selectionModel()->selectedRows();
        if (!indices.size()) {
            Toast::toast(this, NAMEICONRES("model"), tr("NoSelection"));
            return;
        }

        auto filename = QFileDialog::getOpenFileName(
            this, tr("ChooseFile"), _lastusedpath, "png (*.png)");
        if (filename.isEmpty())
            return;

        _lastusedpath = QFileInfo(filename).absoluteDir().absolutePath();

        QVector<int> rows;
        QVector<QImage> imgs;

        QImage img;
        if (img.load(filename)) {
            if (img.size() == _model->frameSize()) {
                for (auto &i : indices) {
                    auto index = i.row();
                    rows.append(index);
                    QImage bimg = _model->image(index).copy();
                    QPainter painter(&bimg);
                    painter.drawImage(QPoint(), img, img.rect());
                    imgs.append(bimg);
                }
                undo.push(new ReplaceFrameCommand(_model, rows, imgs));
                return;
            }
        }
    }
    Toast::toast(this, NAMEICONRES("model"), tr("InvalidModel"));
}

void MainWindow::on_fullscreen() { this->showFullScreen(); }

void MainWindow::on_about() { AboutSoftwareDialog().exec(); }

void MainWindow::on_sponsor() {}

void MainWindow::on_wiki() {}

RibbonTabContent *MainWindow::buildFilePage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();

    {
        auto pannel = tab->addGroup(tr("Basic"));
        addPannelAction(pannel, QStringLiteral("pics"), tr("NewFromPics"),
                        &MainWindow::on_new_frompics);

        addPannelAction(pannel, QStringLiteral("gifs"), tr("NewFromGifs"),
                        &MainWindow::on_new_fromgifs);

        addPannelAction(pannel, QStringLiteral("open"), tr("Open"),
                        &MainWindow::on_open, QKeySequence::Open);
        addPannelAction(pannel, QStringLiteral("recent"), tr("RecentFiles"),
                        EMPTY_FUNC, {}, m_recentMenu);
    }

    {
        auto pannel = tab->addGroup(tr("Save"));

        auto a = addPannelAction(pannel, QStringLiteral("save"), tr("Save"),
                                 &MainWindow::on_save, QKeySequence::Save);
        m_editStateWidgets << a;

        a = addPannelAction(pannel, QStringLiteral("saveas"), tr("SaveAs"),
                            &MainWindow::on_saveas,
                            shortcuts.keySequence(QKeySequences::Key::SAVE_AS));
        m_editStateWidgets << a;

        a = addPannelAction(pannel, QStringLiteral("export"), tr("Export"),
                            &MainWindow::on_export,
                            shortcuts.keySequence(QKeySequences::Key::EXPORT));
        m_editStateWidgets << a;
        a = addPannelAction(
            pannel, QStringLiteral("info"), tr("FileInfo"), [=] {
                FileInfoDialog(_curfilename, _model->frameSize(), _comment)
                    .exec();
            });
        m_editStateWidgets << a;
    }

    return tab;
}

RibbonTabContent *MainWindow::buildEditPage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();
    {
        auto pannel = tab->addGroup(tr("General"));
        auto a = addPannelAction(pannel, QStringLiteral("undo"), tr("Undo"),
                                 &MainWindow::on_undo, QKeySequence::Undo);
        a->setEnabled(false);
        m_toolBtneditors.insert(ToolButtonIndex::UNDO_ACTION, a);

        a = addPannelAction(pannel, QStringLiteral("redo"), tr("Redo"),
                            &MainWindow::on_redo,
                            shortcuts.keySequence(QKeySequences::Key::REDO));
        a->setEnabled(false);
        m_toolBtneditors.insert(ToolButtonIndex::REDO_ACTION, a);

        addPannelAction(pannel, QStringLiteral("cut"), tr("Cut"),
                        &MainWindow::on_cut, QKeySequence::Cut);
        addPannelAction(pannel, QStringLiteral("copy"), tr("Copy"),
                        &MainWindow::on_copy, QKeySequence::Copy);
        addPannelAction(pannel, QStringLiteral("paste"), tr("Paste"),
                        &MainWindow::on_paste, QKeySequence::Paste);
        addPannelAction(pannel, QStringLiteral("del"), tr("Delete"),
                        &MainWindow::on_del, QKeySequence::Delete);
    }

    {
        auto pannel = tab->addGroup(tr("Frame"));
        addPannelAction(
            pannel, QStringLiteral("rmframe"), tr("ReduceFrame"),
            &MainWindow::on_decreaseframe,
            shortcuts.keySequence(QKeySequences::Key::REDUCE_FRAME));
        addPannelAction(pannel, QStringLiteral("rml"), tr("DeleteBefore"),
                        &MainWindow::on_delbefore,
                        shortcuts.keySequence(QKeySequences::Key::DEL_BEFORE));
        addPannelAction(pannel, QStringLiteral("rmr"), tr("DeleteAfter"),
                        &MainWindow::on_delafter,
                        shortcuts.keySequence(QKeySequences::Key::DEL_AFTER));
        addPannelAction(pannel, QStringLiteral("mvf"), tr("MoveLeft"),
                        &MainWindow::on_moveleft,
                        shortcuts.keySequence(QKeySequences::Key::MOV_LEFT));
        addPannelAction(pannel, QStringLiteral("mvb"), tr("MoveRight"),
                        &MainWindow::on_moveright,
                        shortcuts.keySequence(QKeySequences::Key::MOV_RIGHT));
        addPannelAction(pannel, QStringLiteral("reverse"), tr("Reverse"),
                        &MainWindow::on_reverse,
                        shortcuts.keySequence(QKeySequences::Key::REV_ALL));
        addPannelAction(pannel, QStringLiteral("setdelay"), tr("SetDelay"),
                        &MainWindow::on_setdelay,
                        shortcuts.keySequence(QKeySequences::Key::SET_DELAY));
    }

    {
        auto pannel = tab->addGroup(tr("Image"));
        addPannelAction(pannel, QStringLiteral("scale"), tr("ScaleGif"),
                        &MainWindow::on_scalepic,
                        shortcuts.keySequence(QKeySequences::Key::SCALE_PIC));
        addPannelAction(pannel, QStringLiteral("cutpic"), tr("CutGif"),
                        &MainWindow::on_crop,
                        shortcuts.keySequence(QKeySequences::Key::CUT_PIC));
        addPannelAction(pannel, QStringLiteral("fliph"), tr("FilpH"),
                        &MainWindow::on_fliph);
        addPannelAction(pannel, QStringLiteral("flipv"), tr("FlipV"),
                        &MainWindow::on_flipv);
        addPannelAction(pannel, QStringLiteral("rotatel"), tr("RotateLeft"),
                        &MainWindow::on_clockwise);
        addPannelAction(pannel, QStringLiteral("rotater"), tr("RotateR"),
                        &MainWindow::on_anticlockwise);
    }

    {
        auto pannel = tab->addGroup(tr("Effect"));
        addPannelAction(pannel, QStringLiteral("blank"), tr("ExportBlank"),
                        &MainWindow::on_exportapply);
        addPannelAction(pannel, QStringLiteral("model"), tr("ApplyModel"),
                        &MainWindow::on_exportapply);
        addPannelAction(pannel, QStringLiteral("reverseplus"),
                        tr("CreateReverse"), &MainWindow::on_createreverse);
        addPannelAction(pannel, QStringLiteral("scaledelay"), tr("ScaleDelay"),
                        &MainWindow::on_scaledelay,
                        shortcuts.keySequence(QKeySequences::Key::SCALE_DELAY));
    }

    {
        auto pannel = tab->addGroup(tr("Merge"));
        addPannelAction(pannel, QStringLiteral("pics"), tr("InsertPics"),
                        &MainWindow::on_insertpic);
        addPannelAction(pannel, QStringLiteral("gifs"), tr("MergeGIfs"),
                        &MainWindow::on_merge);
    }

    return tab;
}

RibbonTabContent *MainWindow::buildViewPage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();

    {
        auto pannel = tab->addGroup(tr("Player"));
        _playDisWidgets << addPannelAction(
            pannel, QStringLiteral("first"), tr("FirstFrame"),
            &MainWindow::on_beginframe, QKeySequence(Qt::Key_Home));
        _playDisWidgets << addPannelAction(
            pannel, QStringLiteral("back"), tr("LastFrame"),
            &MainWindow::on_last, QKeySequence(Qt::Key_Left));

        _playDisWidgets << addPannelAction(pannel, QStringLiteral("gifplay"),
                                           tr("Play"), &MainWindow::on_play,
                                           QKeySequence(Qt::Key_F5));
        _btnPlayerStop =
            addPannelAction(pannel, QStringLiteral("pause"), tr("Stop"),
                            &MainWindow::on_stop, QKeySequence(Qt::Key_F10));
        _playDisWidgets << addPannelAction(
            pannel, QStringLiteral("foreword"), tr("NextFrame"),
            &MainWindow::on_next, QKeySequence(Qt::Key_Right));
        _playDisWidgets << addPannelAction(
            pannel, QStringLiteral("last"), tr("EndFrame"),
            &MainWindow::on_endframe, QKeySequence(Qt::Key_End));
    }

    {
        auto pannel = tab->addGroup(tr("LoopUp"));

        auto menu = new QMenu(this);
        menu->addAction(
            newAction(QStringLiteral("80%"), [this] { _editor->setZoom(80); }));
        menu->addAction(
            newAction(QStringLiteral("90%"), [this] { _editor->setZoom(90); }));
        menu->addAction(newAction(QStringLiteral("100%"),
                                  [this] { _editor->setZoom(100); }));
        menu->addSeparator();
        menu->addAction(newAction(QStringLiteral("120%"),
                                  [this] { _editor->setZoom(120); }));
        menu->addAction(newAction(QStringLiteral("150%"),
                                  [this] { _editor->setZoom(150); }));
        menu->addAction(newAction(QStringLiteral("200%"),
                                  [this] { _editor->setZoom(200); }));
        menu->addAction(newAction(QStringLiteral("250%"),
                                  [this] { _editor->setZoom(250); }));
        menu->addAction(newAction(QStringLiteral("300%"),
                                  [this] { _editor->setZoom(300); }));
        addPannelAction(pannel, QStringLiteral("scaleview"), tr("Scale"),
                        EMPTY_FUNC, {}, menu);

        addPannelAction(pannel, QStringLiteral("selall"), tr("SelectAll"),
                        &MainWindow::on_selall, QKeySequence::SelectAll);
        addPannelAction(pannel, QStringLiteral("desel"), tr("Deselect"),
                        &MainWindow::on_desel, QKeySequence::Deselect);
        addPannelAction(pannel, QStringLiteral("selrev"),
                        tr("ReverseSelection"), &MainWindow::on_selreverse,
                        shortcuts.keySequence(QKeySequences::Key::SEL_REV));
        addPannelAction(pannel, QStringLiteral("jmp"), tr("Goto"),
                        &MainWindow::on_goto,
                        shortcuts.keySequence(QKeySequences::Key::SEL_REV));
        _playDisWidgets << pannel;
    }

    return tab;
}

RibbonTabContent *MainWindow::buildPluginPage(RibbonTabContent *tab) {
    auto pannel = tab->addGroup(tr("Info"));
    addPannelAction(pannel, QStringLiteral("plugin"), tr("PlgInfo"),
                    [=] { PluginSettingDialog().exec(); });
    return tab;
}

RibbonTabContent *MainWindow::buildSettingPage(RibbonTabContent *tab) {
    auto pannel = tab->addGroup(tr("General"));
    addPannelAction(pannel, QStringLiteral("setting"), tr("Settings"),
                    [=] { EditorSettingDialog().exec(); });
    return tab;
}

RibbonTabContent *MainWindow::buildAboutPage(RibbonTabContent *tab) {
    auto pannel = tab->addGroup(tr("Info"));

    addPannelAction(pannel, QStringLiteral("soft"), tr("Software"),
                    &MainWindow::on_about);

    addPannelAction(pannel, QStringLiteral("sponsor"), tr("Sponsor"),
                    &MainWindow::on_sponsor);

    addPannelAction(pannel, QStringLiteral("wiki"), tr("Wiki"),
                    &MainWindow::on_wiki);

    addPannelAction(pannel, QStringLiteral("qt"), tr("AboutQT"),
                    [this] { WingMessageBox::aboutQt(this); });

    addPannelAction(pannel, QStringLiteral("log"), tr("Log"),
                    [=] { _logdialog->show(); });

    return tab;
}

void MainWindow::openGif(const QString &filename) {
    if (filename == _curfilename) {
        return;
    }

    _model->clearData();
    undo.clear();

    if (checkIsGif(filename) && readGif(filename)) {
        _curfilename = filename;
        setSaved(true);
        setEditModeEnabled(true);
        m_recentmanager->addRecentFile(filename);
    } else {
        Toast::toast(this, QStringLiteral("open"), tr("InvalidGif"));
    }
}

bool MainWindow::readGif(const QString &gif) {
    GifReader reader;
    if (!reader.load(gif)) {
        return false;
    }
    _comment = reader.comment();
    _model->readGifReader(&reader);
    _gallery->setCurrentIndex(_model->index(0));
    return true;
}

bool MainWindow::writeGif(const QString &gif, unsigned int loopCount,
                          const QString &comment) {
    GifWriter writer(_model->width(), _model->height());

    writer.setExtString(comment);
    writer.pushRange(_model->images(), _model->delays());
    auto ret = writer.save(gif, loopCount);
    if (ret) {
        _comment = comment;
    }
    return ret;
}

bool MainWindow::exportGifFrames(const QString &dirPath, const char *ext) {
    if (_model->frameCount() == 0) {
        return false;
    }

    QDir dir(dirPath);
    if (!dir.exists()) {
        return false;
    }

    for (int i = 0; i < _model->frameCount(); ++i) {
        _model->image(i).save(dir.absoluteFilePath(QString::number(i)), ext);
    }

    return true;
}

bool MainWindow::loadfromImages(const QStringList &filenames, int newInterval,
                                qsizetype index, QSize size) {
    if (index < 0) {
        index = qMax(_model->frameCount() - 1, qsizetype(0));
    }
    Q_ASSERT(index < _model->frameCount());

    QVector<GifData> frames;
    for (auto &f : filenames) {
        QImage img;
        if (img.load(f)) {
            GifData d;
            d.delay = newInterval;
            d.image = img;
            frames.append(d);
        }
    }

    if (frames.isEmpty()) {
        return false;
    }

    auto oimg = frames.first().image;
    auto osize = size == QSize() ? oimg.size() : size;

    if (osize.width() > UINT16_MAX || osize.height() > UINT16_MAX) {
        osize.scale(UINT16_MAX, UINT16_MAX, Qt::KeepAspectRatio);
    }

    frames.first().image = oimg.scaled(osize, Qt::KeepAspectRatio);

    undo.push(new InsertFrameCommand(_model, index, frames));

    return true;
}

bool MainWindow::loadfromGifs(QStringList gifs, qsizetype index, QSize size) {
    QVector<GifData> datas;
    for (auto &gif : gifs) {
        GifReader reader;
        if (reader.load(gif)) {
            for (int i = 0; i < reader.imageCount(); ++i) {
                GifData d;
                d.image = reader.image(i);
                d.delay = reader.delay(i);
                datas.append(d);
            }
        }
    }

    if (datas.isEmpty()) {
        return false;
    }

    auto oimg = datas.first().image;
    auto osize = size == QSize() ? oimg.size() : size;

    if (osize.width() > UINT16_MAX || osize.height() > UINT16_MAX) {
        osize.scale(UINT16_MAX, UINT16_MAX, Qt::KeepAspectRatio);
    }

    datas.first().image = oimg.scaled(osize, Qt::KeepAspectRatio);
    undo.push(new InsertFrameCommand(_model, index, datas));

    return true;
}

void MainWindow::setSaved(bool b) {
    m_iSaved->setIcon(b ? _infoSaved : _infoUnsaved);
}

bool MainWindow::ensureSafeClose() {
    if (!undo.isClean()) {
        if (WingMessageBox::question(this, tr("ConfirmClose"),
                                     tr("ConfirmSave") + "\n" +
                                         (_curfilename == QStringLiteral(":")
                                              ? tr("Untitled")
                                              : _curfilename)) ==
            QMessageBox::No)
            return false;
    }
    return true;
}

bool MainWindow::checkIsGif(QString filename) {
    QFile f(filename);
    if (f.open(QFile::ReadOnly)) {
        char *bu = new char[GIF_STAMP_LEN];
        f.read(bu, GIF_STAMP_LEN);
        return !memcmp(GIF_STAMP, bu, GIF_STAMP_LEN) ||
               !memcmp(GIF87_STAMP, bu, GIF_STAMP_LEN) ||
               !memcmp(GIF89_STAMP, bu, GIF_STAMP_LEN);
    }
    return false;
}

void MainWindow::setEditModeEnabled(bool b) {
    updatePlayState();
    updateGifMessage();

    for (auto &item : m_editStateWidgets) {
        item->setEnabled(b);
    }
    m_iSaved->setEnabled(b);
}

void MainWindow::updatePlayState() {
    auto b = _player->isPlaying();
    _btnPlayerStop->setEnabled(b);
    for (auto &item : _playDisWidgets) {
        item->setEnabled(!b);
    }
}

void MainWindow::loadCacheIcon() {
    _infoSaved = ICONRES(QStringLiteral("saved"));
    _infoUnsaved = ICONRES(QStringLiteral("unsaved"));
}

void MainWindow::updateGifMessage() {
    if (_curfilename.isEmpty()) {
        m_status->showMessage(QStringLiteral("-"));
    } else {
        m_status->showMessage(tr("Frame: %1/%2")
                                  .arg(_gallery->currentIndex().row() + 1)
                                  .arg(_model->frameCount()));
    }
}

int MainWindow::getNewFrameInterval() {
    bool ok;
    auto ret = WingInputDialog::getInt(this, tr("NewFrame"),
                                       tr("InputNewFrameInterval(keep10ms)"),
                                       60, 10, 65530, 10, &ok);
    if (ok) {
        return ret - ret % 10;
    } else {
        return -1;
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    _player->stop();
    if (ensureSafeClose()) {
        auto &set = SettingManager::instance();
        set.setRecentFiles(m_recentmanager->saveRecent());
        set.save();
        event->accept();
    } else {
        event->ignore();
    }
}
