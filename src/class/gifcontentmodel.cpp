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

#include "gifcontentmodel.h"

#include <QDir>
#include <QtConcurrent>

GifContentModel::GifContentModel(QObject *parent)
    : QAbstractListModel(parent), _frameCache(64) {}

void GifContentModel::setLinkedListView(QListView *view) {
    if (view) {
        view->setModel(this);
    } else {
        if (_view) {
            _view->setModel(nullptr);
        }
    }
    _view = view;
}

void GifContentModel::setLinkedEditor(GifEditor *editor) { _editor = editor; }

QListView *GifContentModel::linkedListView() const { return _view; }

GifEditor *GifContentModel::linkedGifEditor() const { return _editor; }

void GifContentModel::updateLinkedListViewCurrent() const {
    if (_view) {
        auto i = _view->currentIndex();
        emit _view->selectionModel()->currentRowChanged(i, i);
    }
}

bool GifContentModel::ensureStoreDir() {
    if (_storeDir && _storeDir->isValid()) {
        return true;
    }
    _storeDir = std::make_unique<QTemporaryDir>();
    return _storeDir->isValid();
}

QString GifContentModel::framePathForIndex(qsizetype index) const {
    if (!_storeDir || !_storeDir->isValid()) {
        return {};
    }
    return _storeDir->path() + QDir::separator() +
           QStringLiteral("model_%1.png").arg(index, 6, 10, QLatin1Char('0'));
}

void GifContentModel::persistFrame(qsizetype index, const QImage &image) {
    if (!ensureStoreDir()) {
        return;
    }

    if (_frameFiles.size() <= index) {
        _frameFiles.resize(index + 1);
    }

    const auto framePath = framePathForIndex(index);
    image.save(framePath, "PNG");
    _frameFiles[index] = framePath;
    _frameCache.insert(index, new QImage(image));
}

QImage GifContentModel::loadFrame(qsizetype index) const {
    if (index < 0 || index >= _frameFiles.size()) {
        return {};
    }
    if (auto cached = _frameCache.object(index)) {
        return *cached;
    }
    QImage img;
    img.load(_frameFiles.at(index));
    if (!img.isNull()) {
        _frameCache.insert(index, new QImage(img));
    }
    return img;
}

void GifContentModel::readGifReader(GifReader *reader) {
    if (reader) {
        clearData();
        _frameSize = QSize(reader->width(), reader->height());
        _delays = reader->delays();
        _frameFiles.resize(reader->imageCount());
        if (!ensureStoreDir()) {
            return;
        }

        for (qsizetype i = 0; i < reader->imageCount(); ++i) {
            auto img = reader->image(i);
            persistFrame(i, img);
        }
        emit layoutChanged();
    }
}

bool GifContentModel::insertFrame(const GifData &frame, int index) {
    return insertFrame(frame.image, frame.delay, index);
}

bool GifContentModel::insertFrame(const QImage &image, int delay, int index) {
    if (!isValidGifFrame(image, delay)) {
        return false;
    }

    auto allImages = images();
    auto img = image;
    if (!_frameSize.isEmpty()) {
        img = image.scaled(_frameSize, Qt::IgnoreAspectRatio,
                           Qt::SmoothTransformation);
    } else {
        _frameSize = image.size();
    }

    allImages.insert(index, img);

    beginInsertRows(QModelIndex(), index, index);
    _delays.insert(index, delay);
    _frameFiles.insert(index, QString());
    for (qsizetype i = 0; i < allImages.size(); ++i) {
        persistFrame(i, allImages.at(i));
    }
    endInsertRows();
    return true;
}

void GifContentModel::insertFrames(const QVector<GifData> &frames, int index) {
    QVector<QImage> imgs;
    QVector<int> delays;

    for (auto &f : frames) {
        if (!isValidGifFrame(f.image, f.delay)) {
            continue;
        }

        if (_frameSize.isEmpty()) {
            imgs.append(f.image);
        } else {
            imgs.append(f.image.scaled(_frameSize, Qt::IgnoreAspectRatio,
                                       Qt::SmoothTransformation));
        }
        delays.append(f.delay);
    }

    insertFrames(imgs, delays, index, true);
}

void GifContentModel::insertFrames(const QVector<QImage> &images,
                                   const QVector<int> &delays, int index) {
    insertFrames(images, delays, index, false);
}

void GifContentModel::insertFrames(const QVector<QImage> &images,
                                   const QVector<int> &delays, int index,
                                   bool processed) {
    if (images.size() != delays.size()) {
        return;
    }

    QVector<QImage> n_imgs;
    QVector<int> n_delays;

    if (!processed) {
        for (qsizetype i = 0; i < images.size(); ++i) {
            auto img = images.at(i);
            auto delay = delays.at(i);
            if (!isValidGifFrame(img, delay)) {
                continue;
            }

            n_imgs.append(img);
            n_delays.append(delay);
        }
    } else {
        n_imgs = images;
        n_delays = delays;
    }

    if (n_imgs.isEmpty()) {
        return;
    }

    if (_frameSize.isEmpty()) {
        _frameSize = n_imgs.first().size();
    }

    for (auto &img : n_imgs) {
        if (img.size() != _frameSize) {
            img = img.scaled(_frameSize, Qt::IgnoreAspectRatio,
                             Qt::SmoothTransformation);
        }
    }

    auto allImages = this->images();
    auto insertPos = index < 0 ? frameCount() : index;
    for (int i = 0; i < n_imgs.size(); ++i) {
        allImages.insert(insertPos + i, n_imgs.at(i));
    }

    beginInsertRows(QModelIndex(), insertPos, insertPos + n_imgs.size() - 1);
    _frameFiles.insert(insertPos, n_imgs.size(), QString());
    _delays.insert(insertPos, n_delays.size(), 0);
    for (int i = 0; i < n_delays.size(); ++i) {
        _delays[insertPos + i] = n_delays.at(i);
    }
    for (qsizetype i = 0; i < allImages.size(); ++i) {
        persistFrame(i, allImages.at(i));
    }
    endInsertRows();
}

bool GifContentModel::isValidGifFrame(const QImage &image, int delay) {
    if (image.isNull()) {
        return false;
    }
    if (delay <= 0 || delay > delayLimitMax()) {
        return false;
    }
    return true;
}

int GifContentModel::delayLimitMin() { return 10; }

int GifContentModel::delayLimitMax() { return 65530; }

void GifContentModel::removeFrames(int index, qsizetype count) {
    auto allImages = images();
    if (count < 0) {
        beginRemoveRows(QModelIndex(), index, this->frameCount() - 1);
        allImages.erase(allImages.begin() + index, allImages.end());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        _frameFiles.erase(_frameFiles.cbegin() + index, _frameFiles.cend());
        _delays.erase(_delays.cbegin() + index, _delays.cend());
#else
        _frameFiles.erase(_frameFiles.begin() + index, _frameFiles.end());
        _delays.erase(_delays.begin() + index, _delays.end());
#endif
    } else {
        beginRemoveRows(QModelIndex(), index, index + count - 1);
        allImages.remove(index, count);
        _frameFiles.remove(index, count);
        _delays.remove(index, count);
    }

    _frameCache.clear();
    for (qsizetype i = 0; i < allImages.size(); ++i) {
        persistFrame(i, allImages.at(i));
    }

    if (_frameFiles.isEmpty()) {
        _frameSize = QSize();
    }

    endRemoveRows();
}

void GifContentModel::clearData() {
    _frameFiles.clear();
    _delays.clear();
    _frameSize = QSize();
    _frameCache.clear();
    _storeDir.reset();
    emit layoutChanged();
}

QImage GifContentModel::image(qsizetype index) const { return loadFrame(index); }

int GifContentModel::delay(qsizetype index) const { return _delays.at(index); }

int GifContentModel::width() const {
    if (_frameSize.isEmpty()) {
        return -1;
    }
    return _frameSize.width();
}

int GifContentModel::height() const {
    if (_frameSize.isEmpty()) {
        return -1;
    }
    return _frameSize.height();
}

QSize GifContentModel::frameSize() const { return QSize(width(), height()); }

QVector<int> GifContentModel::delays() const { return _delays; }

QVector<QImage> GifContentModel::images() const {
    QVector<QImage> imgs;
    imgs.reserve(_frameFiles.size());
    for (qsizetype i = 0; i < _frameFiles.size(); ++i) {
        imgs.append(loadFrame(i));
    }
    return imgs;
}

qsizetype GifContentModel::frameCount() const { return _frameFiles.size(); }

void GifContentModel::swapFrames(const QVector<QImage> &imgs) {
    Q_ASSERT(imgs.size() == _frameFiles.size());
    for (qsizetype i = 0; i < imgs.size(); ++i) {
        persistFrame(i, imgs.at(i));
    }
    emit dataChanged(this->index(0), this->index(_frameFiles.size() - 1));
    updateLinkedListViewCurrent();
}

void GifContentModel::cropFrames(const QRect &rect) {
    Q_ASSERT(rect.isValid());
    if (_frameFiles.isEmpty()) {
        return;
    }

    auto imgs = images();
    QtConcurrent::blockingMap(imgs, [rect](QImage &img) { img = img.copy(rect); });

    _frameSize = rect.size();
    for (qsizetype i = 0; i < imgs.size(); ++i) {
        persistFrame(i, imgs.at(i));
    }

    emit dataChanged(this->index(0), this->index(_frameFiles.size() - 1));
    updateLinkedListViewCurrent();
}

void GifContentModel::setFrameDelay(qsizetype index, int delay, qsizetype count) {
    Q_ASSERT(count != 0);
    if (count < 0) {
        for (qsizetype i = index; i < _delays.size(); ++i) {
            _delays.replace(i, delay);
        }

        emit dataChanged(this->index(index), this->index(_delays.size() - 1));
    } else {
        auto total = qMin(index + count, qsizetype(_delays.size()));

        for (qsizetype i = index; i < total; ++i) {
            _delays.replace(i, delay);
        }

        emit dataChanged(this->index(index), this->index(total - 1));
    }
}

bool GifContentModel::setFrameImage(qsizetype index, const QImage &img) {
    if (index < 0 || index > _frameFiles.size()) {
        return false;
    }
    if (img.isNull()) {
        return false;
    }
    auto scaled = _frameSize.isEmpty()
                      ? img
                      : img.scaled(frameSize(), Qt::IgnoreAspectRatio,
                                   Qt::SmoothTransformation);
    if (_frameSize.isEmpty()) {
        _frameSize = scaled.size();
    }
    persistFrame(index, scaled);
    updateLinkedListViewCurrent();

    return true;
}

void GifContentModel::scaleFrames(int width, int height) {
    Q_ASSERT(width > 0 && height > 0);
    if (_frameFiles.isEmpty()) {
        return;
    }

    auto imgs = images();
    QtConcurrent::blockingMap(imgs, [width, height](QImage &img) {
        img = img.scaled(width, height, Qt::IgnoreAspectRatio,
                         Qt::SmoothTransformation);
    });

    _frameSize = QSize(width, height);
    for (qsizetype i = 0; i < imgs.size(); ++i) {
        persistFrame(i, imgs.at(i));
    }

    emit dataChanged(this->index(0), this->index(_frameFiles.size() - 1));
    updateLinkedListViewCurrent();
}

void GifContentModel::flipFrames(Qt::Orientation dir, int index, qsizetype count) {
    Q_UNUSED(index);
    Q_UNUSED(count);

    auto imgs = images();
    switch (dir) {
    case Qt::Horizontal:
        QtConcurrent::blockingMap(imgs,
                                  [](QImage &img) { img = img.mirrored(true, false); });
        break;
    case Qt::Vertical:
        QtConcurrent::blockingMap(imgs,
                                  [](QImage &img) { img = img.mirrored(); });
        break;
    }

    for (qsizetype i = 0; i < imgs.size(); ++i) {
        persistFrame(i, imgs.at(i));
    }

    emit dataChanged(this->index(0), this->index(_frameFiles.size() - 1));
}

void GifContentModel::reverseFrames(qsizetype begin, qsizetype end) {
    if ((end > 0 && end <= begin) || begin < 0 || end >= _frameFiles.size()) {
        return;
    }
    if (end < 0) {
        std::reverse(std::next(_frameFiles.begin(), begin), _frameFiles.end());
        std::reverse(std::next(_delays.begin(), begin), _delays.end());
        emit dataChanged(this->index(begin), this->index(_frameFiles.size() - 1));
    } else {
        std::reverse(std::next(_frameFiles.begin(), begin),
                     std::next(_frameFiles.begin(), end));
        std::reverse(std::next(_delays.begin(), begin),
                     std::next(_delays.begin(), end));
        emit dataChanged(this->index(begin), this->index(end));
    }
    _frameCache.clear();
    updateLinkedListViewCurrent();
}

void GifContentModel::rotateFrames(bool clockwise) {
    QTransform trans;
    trans.rotate(clockwise ? -90 : 90);

    auto imgs = images();
    QtConcurrent::blockingMap(imgs, [&trans](QImage &img) {
        img = img.transformed(trans, Qt::SmoothTransformation);
    });

    if (!imgs.isEmpty()) {
        _frameSize = imgs.first().size();
    }

    for (qsizetype i = 0; i < imgs.size(); ++i) {
        persistFrame(i, imgs.at(i));
    }

    emit dataChanged(this->index(0), this->index(_frameFiles.size() - 1));
    updateLinkedListViewCurrent();
}

void GifContentModel::moveFrames(qsizetype index, MoveFrameDirection dir,
                                 qsizetype count) {
    if (count <= 0 || index < 0) {
        return;
    }
    switch (dir) {
    case MoveFrameDirection::Left: {
        if (index == 0) {
            return;
        }
        auto b = beginMoveRows(QModelIndex(), index, index + count - 1,
                               QModelIndex(), index - 1);
        Q_ASSERT(b);
        moveRange(_frameFiles, index, index - 1, count);
        moveRange(_delays, index, index - 1, count);
    } break;
    case MoveFrameDirection::Right: {
        if (index + count == frameCount() - 1) {
            return;
        }
        auto b = beginMoveRows(QModelIndex(), index, index + count - 1,
                               QModelIndex(), index + 2);
        Q_ASSERT(b);
        moveRange(_frameFiles, index, index + 1, count);
        moveRange(_delays, index, index + 1, count);
    } break;
    }

    _frameCache.clear();
    endMoveRows();
}

int GifContentModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return _frameFiles.size();
}

QVariant GifContentModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole: {
        auto i = index.row();
        return QString::number(_delays.at(i)) + QStringLiteral(" ms");
    }
    case Qt::DecorationRole: {
        return loadFrame(index.row());
    }
    case Qt::ToolTipRole: {
        break;
    }
    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
    }
    return QVariant();
}
