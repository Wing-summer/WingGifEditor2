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

#include "mainwindow.h"

#include <QtConcurrent/QtConcurrentMap>

#include <QDesktopServices>
#include <QStatusBar>

#include "QWingRibbon/ribbontabcontent.h"
#include "class/appmanager.h"
#include "class/clipboardhelper.h"
#include "class/gifcontentmodel.h"
#include "class/giffile.h"
#include "class/gifwriter.h"
#include "class/languagemanager.h"
#include "class/qkeysequences.h"
#include "class/settingmanager.h"
#include "class/waitingloop.h"
#include "class/wingfiledialog.h"
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
#include "command/reverseframecommand.h"
#include "command/rotateframecommand.h"
#include "command/scaleframecommand.h"
#include "control/gifcontentgallery.h"
#include "control/toast.h"
#include "define.h"
#include "dialog/aboutsoftwaredialog.h"
#include "dialog/createreversedialog.h"
#include "dialog/exportdialog.h"
#include "dialog/fileinfodialog.h"
#include "dialog/gifsavedialog.h"
#include "dialog/newdialog.h"
#include "dialog/reduceframedialog.h"
#include "dialog/scalegifdialog.h"
#include "settings/editorsettingdialog.h"

#include <QCloseEvent>
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

    _splitter = new QSplitter(this);
    _splitter->setOrientation(Qt::Vertical);

    _editor = new GifEditor(QImage(NAMEICONRES("icon")), this);
    _splitter->addWidget(_editor);
    _gallery = new GifContentGallery(this);
    _splitter->addWidget(_gallery);

    _model = new GifContentModel(_gallery);
    _model->setLinkedListView(_gallery);
    _model->setLinkedEditor(_editor);
    connect(_model, &GifContentModel::sigUpdateUIProcess, this,
            [] { qApp->processEvents(); });
    _gallery->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _gallery->setMaximumHeight(300);
    connect(_gallery->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this,
            [=](const QModelIndex &current, const QModelIndex & /*previous*/) {
                auto i = current.row();
                auto img = _model->image(i);
                auto delay = _model->delay(i);
                _editor->setImage(img);
                updateGifMessage();
            });
    connect(_model, &QAbstractListModel::rowsInserted, this,
            &MainWindow::updateGifMessage);
    connect(_model, &QAbstractListModel::rowsRemoved, this,
            &MainWindow::updateGifMessage);

    auto ldata = SettingManager::instance().editorLayout();
    if (!ldata.isEmpty()) {
        _splitter->restoreState(ldata);
    }
    layout->addWidget(_splitter, 1);

    m_status = new QStatusBar(this);

    _status = new QLabel(this);
    m_status->addPermanentWidget(_status);

    auto separator = new QFrame(this);
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    m_status->addPermanentWidget(separator);

    // init statusbar
    loadCacheIcon();
    auto disableStyle = QStringLiteral("border:none;background:transparent;");

    m_iBatch = new QToolButton(m_status);
    m_iBatch->setIcon(ICONRES("batch"));
    m_iBatch->setStyleSheet(disableStyle);
    m_iBatch->setEnabled(false);
    m_status->addPermanentWidget(m_iBatch);

    m_iSaved = new QToolButton(m_status);
    m_iSaved->setIcon(_infoSaved);
    m_iSaved->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_iSaved->setToolTip(tr("InfoSave"));
    m_iSaved->setStyleSheet(disableStyle);

    m_status->addPermanentWidget(m_iSaved);

    layout->addWidget(m_status);

    buildUpContent(cw);

    _player = new PlayGifManager(this);
    _player->setTickIntervals([this](qsizetype index) -> int {
        if (index < 0 || index >= _model->frameCount()) {
            return -1;
        }
        return _model->delay(index);
    });
    connect(_player, &PlayGifManager::tick, this, [=](int index) {
        auto i = _model->index(index);
        _gallery->setCurrentIndex(i);
    });
    connect(_player, &PlayGifManager::playStateChanged, this,
            &MainWindow::updatePlayState);

    auto psc = new QShortcut(QKeySequence(Qt::Key_Space), this);
    psc->setContext(Qt::ShortcutContext::WindowShortcut);
    connect(psc, &QShortcut::activated, this, [=] {
        if (_curfilename.isEmpty()) {
            return;
        }
        if (_player->isPlaying()) {
            on_stop();
        } else {
            on_play();
        }
    });

    connect(&undo, &QUndoStack::canUndoChanged, _btnUndoAction,
            &QToolButton::setEnabled);
    connect(&undo, &QUndoStack::canRedoChanged, _btnRedoAction,
            &QToolButton::setEnabled);
    connect(&undo, &QUndoStack::cleanChanged, this, [=](bool clean) {
        this->setSaved(clean && _curfilename != QStringLiteral(":"));
    });

    connect(_editor, &GifEditor::selRectChanged, this, [=](const QRectF &rect) {
        auto r = rect.toRect();
        if (_editor->isCuttingMode()) {
            updateTipMessage(QStringLiteral("X: %1, Y: %2, W: %3, H: %4")
                                 .arg(r.x())
                                 .arg(r.y())
                                 .arg(r.width())
                                 .arg(r.height()));
        }
    });
    connect(_editor, &GifEditor::cropFinished, this, [=](bool finished) {
        _editor->setCropMode(false);
        this->setEditModeEnabled(true);
        clearTipMessage();
        if (finished) {
            auto sel = _editor->selRect();
            if (sel.size() == _model->frameSize() || sel.x() < 0 ||
                sel.y() < 0 || sel.width() < 0 || sel.height() < 0) {
                return;
            }
            undo.push(new CropImageCommand(_model, sel.toRect()));
        }
    });

    // note: this signal is customed and the QPoint is global
    connect(_editor, &GifEditor::customContextMenuRequested, this,
            [=](const QPoint &gp) {
                if (_model->frameCount() > 0 && !_player->isPlaying()) {
                    auto shortcuts = QKeySequences::instance();

                    QMenu menu;
                    menu.addAction(ICONRES(QStringLiteral("copy")), tr("Copy"),
                                   QKeySequence::Copy, this,
                                   &MainWindow::on_copy);
                    menu.addAction(ICONRES(QStringLiteral("paste")),
                                   tr("Paste"), QKeySequence::Paste, this,
                                   &MainWindow::on_paste);
                    menu.addSeparator();
                    menu.addAction(
                        ICONRES(QStringLiteral("rml")), tr("DeleteBefore"),
                        shortcuts.keySequence(QKeySequences::Key::DEL_BEFORE),
                        this, &MainWindow::on_delbefore);
                    menu.addAction(
                        ICONRES(QStringLiteral("rmr")), tr("DeleteAfter"),
                        shortcuts.keySequence(QKeySequences::Key::DEL_AFTER),
                        this, &MainWindow::on_delafter);
                    menu.addAction(
                        ICONRES(QStringLiteral("reverse")), tr("Reverse"),
                        shortcuts.keySequence(QKeySequences::Key::REV_ALL),
                        this, &MainWindow::on_reverse);
                    menu.addAction(
                        ICONRES(QStringLiteral("setdelay")), tr("SetDelay"),
                        shortcuts.keySequence(QKeySequences::Key::SET_DELAY),
                        this, &MainWindow::on_setdelay);
                    menu.addAction(
                        ICONRES(QStringLiteral("scale")), tr("ScaleGif"),
                        shortcuts.keySequence(QKeySequences::Key::SCALE_PIC),
                        this, &MainWindow::on_scalepic);
                    menu.addAction(
                        ICONRES(QStringLiteral("cutpic")), tr("CutGif"),
                        shortcuts.keySequence(QKeySequences::Key::CUT_PIC),
                        this, &MainWindow::on_crop);
                    menu.addSeparator();
                    menu.addAction(ICONRES(QStringLiteral("fliph")),
                                   tr("FilpH"), this, &MainWindow::on_fliph);
                    menu.addAction(ICONRES(QStringLiteral("flipv")),
                                   tr("FlipV"), this, &MainWindow::on_flipv);
                    menu.addAction(ICONRES(QStringLiteral("rotatel")),
                                   tr("RotateLeft"), this,
                                   &MainWindow::on_clockwise);
                    menu.addAction(ICONRES(QStringLiteral("rotater")),
                                   tr("RotateR"), this,
                                   &MainWindow::on_anticlockwise);
                    menu.exec(gp);
                }
            });

    setEditModeEnabled(false);

    setWindowTitle(tr("WingGifEditor2"));
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
    _playDisWidgets << buildSettingPage(m_ribbon->addTab(tr("Setting")));
    _playDisWidgets << buildAboutPage(m_ribbon->addTab(tr("About")));
    connect(m_ribbon, &Ribbon::onDragDropFiles, this,
            [=](const QStringList &files) {
                QString f;

                if (files.size() > 1) {
                    bool ok;
                    f = WingInputDialog::getItem(this, tr("ChooseFile"),
                                                 tr("Choose to open"), files, 0,
                                                 false, &ok);
                    if (!ok) {
                        return;
                    }
                }

                openGif(f);
            });
}

void MainWindow::updateTipMessage(const QString &msg) { _status->setText(msg); }

void MainWindow::clearTipMessage() { _status->clear(); }

void MainWindow::on_new() {
    _player->stop();
    if (ensureSafeClose()) {
        updateTipMessage(tr("Newing..."));
        NewDialog d(this);
        if (d.exec()) {
            WaitingLoop dw(tr("NewFromPicsGif"));
            if (loadfromImages(d.getFilenames(), getNewFrameInterval())) {
                _curfilename = QStringLiteral(":");
                setSaved(false);
                _gallery->setCurrentIndex(_model->index(0));
                setEditModeEnabled(true);
            }
        }
        clearTipMessage();
    }
}

void MainWindow::on_open() {
    _player->stop();
    if (ensureSafeClose()) {
        updateTipMessage(tr("Opening..."));
        auto filename = WingFileDialog::getOpenFileName(
            this, tr("ChooseFile"), _lastusedpath, "gif (*.gif)");
        if (filename.isEmpty())
            return;

        _lastusedpath = Utilities::getAbsoluteDirPath(filename);

        openGif(filename);
        clearTipMessage();
    }
}

void MainWindow::on_save() {
    _player->stop();
    if (_curfilename == QStringLiteral(":")) {
        on_saveas();
        return;
    }

    updateTipMessage(tr("Saving..."));

    GifSaveDialog d(_model->comment());
    GifSaveResult r;
    if (d.exec()) {
        r = d.getResult();
    } else {
        clearTipMessage();
        return;
    }

    WingProgressDialog dw;
    dw.dialog()->setCancelButton(nullptr);
    dw.dialog()->setLabelText(tr("SaveGif"));
    dw.dialog()->setRange(0, 0);

    if (writeGif(_curfilename, r.comment, r.loop)) {
        undo.setClean();
        _model->setComment(r.comment);
        Toast::toast(this, NAMEICONRES("save"), tr("SaveSuccess"));
    } else {
        WingMessageBox::critical(this, qAppName(), tr("SaveFail"));
    }

    clearTipMessage();
}

void MainWindow::on_saveas() {
    _player->stop();

    updateTipMessage(tr("SavingAs..."));
    GifSaveDialog d(_model->comment());
    GifSaveResult r;
    if (d.exec()) {
        r = d.getResult();
    } else {
        clearTipMessage();
        return;
    }

    auto filename = WingFileDialog::getSaveFileName(
        this, tr("ChooseSaveFile"), _lastusedpath,
        QStringLiteral("gif (*.gif)"));
    if (filename.isEmpty()) {
        clearTipMessage();
        return;
    }

    _lastusedpath = Utilities::getAbsoluteDirPath(filename);

    WaitingLoop wl(tr("SaveAsGif"));

    if (writeGif(filename, r.comment, r.loop)) {
        undo.setClean();
        _model->setComment(r.comment);
        _curfilename = filename;
        m_recentmanager->addRecentFile(filename);
        Toast::toast(this, NAMEICONRES("saveas"), tr("SaveAsSuccess"));
    } else {
        Toast::toast(this, NAMEICONRES("saveas"), tr("SaveAsFail"));
    }

    clearTipMessage();
}

void MainWindow::on_export() {
    _player->stop();

    updateTipMessage(tr("Exporting..."));
    bool isGlobal = isGLobalOp();
    ExportDialog d(isGlobal, this);
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

        if (exportGifFrames(res.path, ext, isGlobal)) {
            Toast::toast(this, NAMEICONRES("saveas"), tr("ExportSuccess"));
        } else {
            Toast::toast(this, NAMEICONRES("saveas"), tr("ExportFail"));
        }
    }
    clearTipMessage();
}

void MainWindow::on_close() {
    _player->stop();
    if (ensureSafeClose()) {
        _editor->setImage(QImage(NAMEICONRES(QStringLiteral("icon"))));
        _model->clearData();
        _curfilename.clear();
        undo.clear();
        _editor->fitOpenSize();
        setEditModeEnabled(false);
    }
}

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
    QVector<QSharedPointer<GifFrame>> sel;
    for (auto &i : sels) {
        auto index = i.row();
        sel.append(_model->frame(index));
    }
    if (ClipBoardHelper::setImageFrames(sel)) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("copy")),
                     tr("CopyToClipBoard"));
    } else {
        Toast::toast(this, NAMEICONRES(QStringLiteral("copy")),
                     tr("UnCopyToClipBoard"));
    }
}

void MainWindow::on_cut() {
    _player->stop();
    auto sels = getSelectedIndices();
    if (sels.size() == _model->frameCount()) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("cut")),
                     tr("MustKeepOneFrame"));
        return;
    }
    QVector<int> indices;
    QVector<QSharedPointer<GifFrame>> sel;
    for (auto &index : sels) {
        indices.append(index);
        sel.append(_model->frame(index));
    }
    if (!ClipBoardHelper::setImageFrames(sel)) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("cut")),
                     tr("UnCutToClipBoard"));
        return;
    }
    undo.push(new RemoveFrameCommand(_model, indices));
    Toast::toast(this, NAMEICONRES(QStringLiteral("cut")),
                 tr("CutToClipBoard"));
}

void MainWindow::on_paste() {
    _player->stop();
    auto pos = _gallery->currentIndex().row() + 1;
    QVector<QSharedPointer<GifFrame>> imgs;
    ClipBoardHelper::getImageFrames(imgs);
    if (imgs.count()) {
        undo.push(new InsertFrameCommand(_model, pos, imgs));
    }
}

void MainWindow::on_del() {
    _player->stop();
    auto sels = getSelectedIndices();
    if (sels.size() == _model->frameCount()) {
        Toast::toast(this, NAMEICONRES(QStringLiteral("cut")),
                     tr("MustKeepOneFrame"));
        return;
    }
    undo.push(new RemoveFrameCommand(_model, sels));
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
        _gallery->clearSelection();
        _gallery->setCurrentIndex(_model->index(index - 1));
    }
}

void MainWindow::on_beginframe() {
    _player->stop();
    _gallery->clearSelection();
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
    _gallery->clearSelection();
    _gallery->setCurrentIndex(index);
}

void MainWindow::on_play() { _player->play(_gallery->currentIndex().row()); }

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
        undo.push(
            new ReduceFrameCommand(_model, res.start, res.end, res.stepcount));
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

    int start, end;
    if (isGLobalOp()) {
        start = 0;
        end = -1;
    } else {
        auto rs = getSelectedIndices();
        if (rs.isEmpty()) {
            return;
        }
        std::sort(rs.begin(), rs.end());

        start = rs.first();
        end = start;
        if (rs.size() == 1) {
            start = 0;
            end = -1;
        } else {
            auto total = rs.size();
            for (qsizetype i = 1; i < total; ++i) {
                auto l = rs.at(i - 1);
                auto r = rs.at(i);
                if (l + 1 != r) {
                    return;
                }
                end = r;
            }
            end++;
        }
    }

    undo.push(new ReverseFrameCommand(_model, start, end));
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
        QVector<QSharedPointer<GifFrame>> datas;
        datas.reserve(res.end - res.start + 1);
        for (int i = res.end; i >= res.start; --i) {
            auto image = _model->image(i);
            auto delay = _model->delay(i);
            datas.append(_model->generateFrame(image, delay));
        }
        undo.push(new InsertFrameCommand(_model, res.end, datas));
    }
}

void MainWindow::on_setdelay() {
    if (_model->frameCount() == 0) {
        return;
    }

    _player->stop();

    bool ok;
    bool isGlobal = isGLobalOp();
    auto time = WingInputDialog::getInt(
        this, isGlobal ? tr("DelayTime") + tr("(Global)") : tr("DelayTime"),
        tr("Inputms"), _model->delay(0), 1, INT_MAX, 1, &ok);
    if (ok) {
        QVector<int> is;
        if (!isGlobal) {
            is = getSelectedIndices();
        }
        undo.push(new DelayFrameCommand(_model, is, time));
    }
}

void MainWindow::on_scaledelay() {
    _player->stop();

    bool ok;
    bool isGlobal = isGLobalOp();
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
        WingFileDialog::getOpenFileNames(this, tr("ChooseFile"), _lastusedpath,
                                         tr("Images (*.jpg *.tiff *.png)"));
    if (filenames.isEmpty())
        return;
    _lastusedpath = QFileInfo(filenames.first()).absoluteDir().absolutePath();

    WaitingLoop wl(tr("InsertPics"));

    auto pos = _gallery->currentIndex().row() + 1;
    if (loadfromImages(filenames, pos, getNewFrameInterval(),
                       _model->frameSize())) {
        Toast::toast(this, NAMEICONRES("pics"), tr("InsertPicsSuccess"));
    } else {
        Toast::toast(this, NAMEICONRES("pics"), tr("InsertPicsFailed"));
    }
}

void MainWindow::on_merge() {
    _player->stop();

    auto filenames = WingFileDialog::getOpenFileNames(
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
    _editor->setSelRect(0, 0, s.width(), s.height());
    _editor->setCropMode(true);
}

void MainWindow::on_fliph() {
    _player->stop();
    undo.push(new FlipFrameCommand(
        _model, isGLobalOp() ? getAllFrameIndices() : getSelectedIndices(),
        Qt::Horizontal));
}

void MainWindow::on_flipv() {
    _player->stop();
    undo.push(new FlipFrameCommand(
        _model, isGLobalOp() ? getAllFrameIndices() : getSelectedIndices(),
        Qt::Vertical));
}

void MainWindow::on_clockwise() {
    _player->stop();
    undo.push(new RotateFrameCommand(_model, true));
}

void MainWindow::on_anticlockwise() {
    _player->stop();
    undo.push(new RotateFrameCommand(_model, false));
}

void MainWindow::on_fullscreen() { this->showFullScreen(); }

void MainWindow::on_about() { AboutSoftwareDialog().exec(); }

void MainWindow::on_sponsor() {
    // Github is not easy to access for Chinese people,
    // Gitee mirror instead
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    if (LanguageManager::instance().defaultLocale().territory() ==
#else
    if (LanguageManager::instance().defaultLocale().country() ==
#endif
        QLocale::China) {
        QDesktopServices::openUrl(QUrl(QStringLiteral(
            "https://gitee.com/wing-cloud/WingGifEditor2#%E6%8D%90%E5%8A%A9")));
    } else {
        QDesktopServices::openUrl(
            QUrl(QStringLiteral("https://github.com/Wing-summer/"
                                "WingGifEditor2#%E6%8D%90%E5%8A%A9")));
    }
}

void MainWindow::on_wiki() {
    QDesktopServices::openUrl(QUrl(
        QStringLiteral("https://github.com/Wing-summer/WingGifEditor2/wiki")));
}

RibbonTabContent *MainWindow::buildFilePage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();

    {
        auto pannel = tab->addGroup(tr("Basic"));
        addPannelAction(pannel, QStringLiteral("new"), tr("New"),
                        &MainWindow::on_new);

        addPannelAction(pannel, QStringLiteral("open"), tr("Open"),
                        &MainWindow::on_open, QKeySequence::Open);

        addPannelAction(pannel, QStringLiteral("recent"), tr("RecentFiles"),
                        EMPTY_FUNC, {}, m_recentMenu);
    }

    {
        auto pannel = tab->addGroup(tr("Save"));

        m_editStateWidgets << addPannelAction(pannel, QStringLiteral("save"),
                                              tr("Save"), &MainWindow::on_save,
                                              QKeySequence::Save);
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("saveas"), tr("SaveAs"),
            &MainWindow::on_saveas,
            shortcuts.keySequence(QKeySequences::Key::SAVE_AS));

        auto a =
            addPannelAction(pannel, QStringLiteral("export"), tr("Export"),
                            &MainWindow::on_export,
                            shortcuts.keySequence(QKeySequences::Key::EXPORT));
        m_editStateWidgets.append(a);
        _gTools.append(a);
        m_editStateWidgets << addPannelAction(
            pannel, QStringLiteral("info"), tr("FileInfo"), [=] {
                FileInfoDialog(_curfilename, _model->frameSize(),
                               _model->comment())
                    .exec();
            });
        m_editStateWidgets << addPannelAction(pannel, QStringLiteral("close"),
                                              tr("Close"),
                                              &MainWindow::on_close);
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
        _btnUndoAction = a;

        a = addPannelAction(pannel, QStringLiteral("redo"), tr("Redo"),
                            &MainWindow::on_redo,
                            shortcuts.keySequence(QKeySequences::Key::REDO));
        a->setEnabled(false);
        _btnRedoAction = a;

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
        _gTools << addPannelAction(
            pannel, QStringLiteral("reverse"), tr("Reverse"),
            &MainWindow::on_reverse,
            shortcuts.keySequence(QKeySequences::Key::REV_ALL));
        _gTools << addPannelAction(
            pannel, QStringLiteral("setdelay"), tr("SetDelay"),
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
        _gTools << addPannelAction(pannel, QStringLiteral("fliph"), tr("FilpH"),
                                   &MainWindow::on_fliph);
        _gTools << addPannelAction(pannel, QStringLiteral("flipv"), tr("FlipV"),
                                   &MainWindow::on_flipv);
        addPannelAction(pannel, QStringLiteral("rotatel"), tr("RotateLeft"),
                        &MainWindow::on_clockwise);
        addPannelAction(pannel, QStringLiteral("rotater"), tr("RotateR"),
                        &MainWindow::on_anticlockwise);
    }

    {
        auto pannel = tab->addGroup(tr("Effect"));
        addPannelAction(pannel, QStringLiteral("reverseplus"),
                        tr("CreateReverse"), &MainWindow::on_createreverse);
        addPannelAction(pannel, QStringLiteral("scaledelay"), tr("ScaleDelay"),
                        &MainWindow::on_scaledelay,
                        shortcuts.keySequence(QKeySequences::Key::SCALE_DELAY));
    }

    {
        auto pannel = tab->addGroup(tr("Merge"));
        addPannelAction(pannel, QStringLiteral("picture"), tr("InsertPics"),
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
                                           tr("Play"), &MainWindow::on_play);
        _btnPlayerStop = addPannelAction(pannel, QStringLiteral("pause"),
                                         tr("Stop"), &MainWindow::on_stop);

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
            newAction(tr("FitInView"), [this] { _editor->fitInEditorView(); }));
        menu->addAction(newAction(QStringLiteral("100%"),
                                  [this] { _editor->setZoom(100); }));
        menu->addSeparator();
        menu->addAction(
            newAction(QStringLiteral("80%"), [this] { _editor->setZoom(80); }));
        menu->addAction(
            newAction(QStringLiteral("90%"), [this] { _editor->setZoom(90); }));
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

    return tab;
}

void MainWindow::openGif(const QString &filename) {
    if (filename == _curfilename) {
        Toast::toast(this, NAMEICONRES("file"), tr("OpenedGif"));
        return;
    }

    _model->clearData();
    undo.clear();

    if (checkIsGif(filename)) {
        auto r = readGif(filename);
        if (r == GifFile::ErrorCode::SUCCEEDED) {
            _curfilename = filename;
            setSaved(true);
            setEditModeEnabled(true);
            m_recentmanager->addRecentFile(filename);
        } else {
            QString err;
            switch (r) {
            case GifFile::ErrorCode::ERR_OPEN_FAILED:
                err = tr("Failed to open given file");
                break;
            case GifFile::ErrorCode::ERR_NO_SCRN_DSCR:
                err = tr("Screen descriptor has already been set");
                break;
            case GifFile::ErrorCode::ERR_NO_COLOR_MAP:
                err = tr("Neither global nor local color map");
                break;
            case GifFile::ErrorCode::ERR_DATA_TOO_BIG:
                err = tr("Number of pixels bigger than width * height");
                break;
            case GifFile::ErrorCode::ERR_NOT_ENOUGH_MEM:
                err = tr("Failed to allocate required memory");
                break;
            case GifFile::ErrorCode::ERR_CLOSE_FAILED:
                err = tr("Failed to close given file");
                break;
            case GifFile::ErrorCode::ERR_READ_FAILED:
                err = tr("Failed to read from given file");
                break;
            case GifFile::ErrorCode::ERR_NOT_GIF_FILE:
                err = tr("Data is not in GIF format");
                break;
            case GifFile::ErrorCode::ERR_NO_IMAG_DSCR:
                err = tr("No Image Descriptor detected");
                break;
            case GifFile::ErrorCode::ERR_WRONG_RECORD:
                err = tr("Wrong record type detected");
                break;
            case GifFile::ErrorCode::ERR_NOT_READABLE:
                err = tr("Given file was not opened for read");
                break;
            case GifFile::ErrorCode::ERR_IMAGE_DEFECT:
                err = tr("Image is defective, decoding aborted");
                break;
            case GifFile::ErrorCode::ERR_EOF_TOO_SOON:
                err = tr("Image EOF detected before image complete");
                break;
            case GifFile::ErrorCode::SUCCEEDED:
                break;
            }
            WingMessageBox::critical(this, qAppName(),
                                     tr("OpenFailed - ") + err);
        }
    } else {
        Toast::toast(this, QStringLiteral("open"), tr("InvalidGif"));
    }
}

GifFile::ErrorCode MainWindow::readGif(const QString &gif) {
    WaitingLoop dw(tr("ReadingGif"));
    auto r = _model->readGifFile(gif);

    switch (r) {
    case GifFile::ErrorCode::SUCCEEDED:
        break;
    default:
        return r;
    }

    if (_gallery->currentIndex().row() == 0) {
        auto index = _model->index(0);
        emit _gallery->selectionModel()->currentRowChanged(index, index);
    } else {
        _gallery->setCurrentIndex(_model->index(0));
    }
    _editor->fitOpenSize();
    return r;
}

bool MainWindow::writeGif(const QString &gif, QString &comment,
                          unsigned int loopCount) {
    GifWriter writer;
    connect(&writer, &GifWriter::sigUpdateUIProcess, this,
            [] { qApp->processEvents(); });

    WaitingLoop dw(tr("WritingGif"));
    writer.setExtString(comment);
    comment = writer.extString();
    auto ret = writer.save(gif, loopCount, _model->frameCount(),
                           [this](qsizetype i) -> QPair<int, QImage> {
                               auto delay = _model->delay(i);
                               auto image = _model->image(i);
                               return qMakePair(delay, image);
                           });
    return ret;
}

bool MainWindow::exportGifFrames(const QString &dirPath, const char *ext,
                                 bool isGlobal) {
    if (_model->frameCount() == 0) {
        return false;
    }

    QDir dir(dirPath);
    if (!dir.exists()) {
        return false;
    }

    WaitingLoop dw(tr("ExportingGif"));
    QtConcurrent::blockingMap(
        isGlobal ? getAllFrameIndices() : getSelectedIndices(),
        [this, &dir, ext](int i) {
            _model->image(i).save(dir.absoluteFilePath(QString::number(i)),
                                  ext);
        });

    return true;
}

bool MainWindow::loadfromImages(const QStringList &filenames, int newInterval,
                                qsizetype index, const QSize &size) {
    if (_model->frameCount() + filenames.size() > FRAME_COUNT_LIMIT) {
        return false;
    }

    if (index < 0) {
        index = qMax(_model->frameCount() - 1, qsizetype(0));
    }
    Q_ASSERT(index < _model->frameCount());
    Q_ASSERT(_model->frameCount() > 0);

    int total = filenames.size();
    undo.push(new InsertFrameCommand(
        _model, index,
        [this, filenames](int index) -> QPair<int, QImage> {
            QImage img;
            auto f = filenames.at(index);
            if (!img.load(f)) {
                return {-1, {}};
            }

            auto osize = _model->frameSize();
            if (img.size() != osize) {
                osize.scale(osize, Qt::KeepAspectRatio);
            }
            return qMakePair(FRAME_DEFAULT_DELAY, img);
        },
        total));

    return true;
}

bool MainWindow::loadfromGifs(const QStringList &gifs, qsizetype index,
                              const QSize &size) {
    auto curCount = _model->frameCount();
    if (curCount + gifs.size() > FRAME_COUNT_LIMIT) {
        return false;
    }

    if (index < 0) {
        index = qMax(curCount - 1, qsizetype(0));
    }
    Q_ASSERT(index < curCount);
    Q_ASSERT(curCount > 0);

    QVector<QSharedPointer<GifFrame>> datas;
    for (auto &gif : gifs) {
        GifFile reader;
        if (reader.load(gif) == GifFile::ErrorCode::SUCCEEDED) {
            if (curCount + reader.frameCount() > FRAME_COUNT_LIMIT) {
                return false;
            }
            auto osize = _model->frameSize();
            if (osize != reader.size()) {
                datas << QtConcurrent::blockingMapped<GifFile::data_type>(
                    reader.begin(), reader.end(),
                    [this, osize](const QSharedPointer<GifFrame> &frame)
                        -> QSharedPointer<GifFrame> {
                        auto delay = frame->delay();
                        auto image = frame->image();
                        return _model->generateFrame(
                            image.scaled(osize, Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation),
                            delay);
                    });
            } else {
                datas.append(reader.begin(), reader.end());
            }
        }
    }

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
    m_iBatch->setEnabled(isGLobalOp());
    updateGlobalMark();
    m_iSaved->setEnabled(b);
}

void MainWindow::updatePlayState() {
    auto b = _player->isPlaying();
    _btnPlayerStop->setEnabled(b);
    for (auto &item : _playDisWidgets) {
        item->setEnabled(!b);
    }
}

QVector<int> MainWindow::getSelectedIndices() const {
    QVector<int> indices;
    for (auto &item : _gallery->selectionModel()->selectedIndexes()) {
        indices.append(item.row());
    }
    return indices;
}

QVector<int> MainWindow::getAllFrameIndices() const {
    QVector<int> indices;
    indices.resize(_model->frameCount());
    std::iota(indices.begin(), indices.end(), 0);
    return indices;
}

bool MainWindow::isGLobalOp() const { return m_iBatch->isEnabled(); }

void MainWindow::updateGlobalMark() {
    static bool gcache = false;
    auto n = isGLobalOp();
    if (gcache == n) {
        return;
    }
    gcache = n;
    if (n) {
        for (auto &w : _gTools) {
            auto font = w->font();
            font.setUnderline(true);
            font.setBold(true);
            w->setFont(font);
        }
    } else {
        for (auto &w : _gTools) {
            auto font = w->font();
            font.setUnderline(false);
            font.setBold(false);
            w->setFont(font);
        }
    }
}

void MainWindow::loadCacheIcon() {
    _infoSaved = ICONRES(QStringLiteral("saved"));
    _infoUnsaved = ICONRES(QStringLiteral("unsaved"));
}

void MainWindow::updateGifMessage() {
    if (_curfilename.isEmpty()) {
        m_status->showMessage({});
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
        _model->clearData();
        auto &set = SettingManager::instance();
        set.setRecentFiles(m_recentmanager->saveRecent());
        set.setEditorLayout(_splitter->saveState());
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (_model->frameCount() > 0) {
        m_iBatch->setEnabled(event->modifiers() == Qt::ControlModifier);
        updateGlobalMark();
    }
    FramelessMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (_model->frameCount() > 0) {
        m_iBatch->setEnabled(false);
        updateGlobalMark();
    }
    FramelessMainWindow::keyReleaseEvent(event);
}

void MainWindow::focusInEvent(QFocusEvent *event) {
    m_iBatch->setEnabled(QGuiApplication::keyboardModifiers() ==
                         Qt::ControlModifier);
    updateGlobalMark();
    FramelessMainWindow::focusInEvent(event);
}

void MainWindow::enterEvent(QEnterEvent *event) {
    m_iBatch->setEnabled(event->modifiers() == Qt::ControlModifier);
    updateGlobalMark();
    FramelessMainWindow::enterEvent(event);
}
