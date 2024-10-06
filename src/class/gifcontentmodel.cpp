#include "gifcontentmodel.h"

GifContentModel::GifContentModel(QObject *parent)
    : QAbstractListModel(parent) {}

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

QListView *GifContentModel::linkedListView() const { return _view; }

void GifContentModel::updateLinkedListViewCurrent() const {
    if (_view) {
        auto i = _view->currentIndex();
        emit _view->selectionModel()->currentRowChanged(i, i);
    }
}

void GifContentModel::readGifReader(GifReader *reader) {
    if (reader) {
        _data = reader->images();
        _delays = reader->delays();
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

    beginInsertRows(QModelIndex(), index, index);
    if (_data.isEmpty()) {
        _data.insert(index, image);
    } else {
        _data.insert(index,
                     image.scaled(_data.first().size(), Qt::IgnoreAspectRatio,
                                  Qt::SmoothTransformation));
    }
    _delays.insert(index, delay);
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

        if (_data.isEmpty()) {
            imgs.append(f.image);
        } else {
            imgs.append(f.image.scaled(_data.first().size(),
                                       Qt::IgnoreAspectRatio,
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

    if (index < 0) {
        beginInsertRows(QModelIndex(), this->frameCount(),
                        this->frameCount() + images.size());
    } else {
        beginInsertRows(QModelIndex(), index, index + images.size());
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

    if (index < 0) {
        _data.append(n_imgs);
        _delays.append(delays);
    } else {
        auto total = n_imgs.size();
        _data.insert(index, total, QImage());
        _delays.insert(index, total, 0);

        for (int i = 0; i < total; ++i) {
            _data.replace(i + index, n_imgs.at(i));
            _delays.replace(i + index, _delays.at(i));
        }
    }

    endInsertRows();
}

bool GifContentModel::isValidGifFrame(const QImage &image, int delay) {
    if (image.isNull()) {
        return false;
    }
    if (delay <= 0 && delay > delayLimitMax()) {
        return false;
    }
    return true;
}

int GifContentModel::delayLimitMin() { return 10; }

int GifContentModel::delayLimitMax() { return 65530; }

void GifContentModel::removeFrames(int index, qsizetype count) {
    if (count < 0) {
        beginRemoveRows(QModelIndex(), index, this->frameCount() - 1);
        _data.erase(_data.cbegin() + index, _data.cend());
        _delays.erase(_delays.cbegin() + index, _delays.cend());
    } else {
        beginRemoveRows(QModelIndex(), index, index + count - 1);
        _data.remove(index, count);
        _delays.remove(index, count);
    }

    endRemoveRows();
}

void GifContentModel::clearData() {
    _data.clear();
    _delays.clear();
    emit layoutChanged();
}

QImage GifContentModel::image(qsizetype index) const { return _data.at(index); }

int GifContentModel::delay(qsizetype index) const { return _delays.at(index); }

int GifContentModel::width() const {
    if (_data.isEmpty()) {
        return -1;
    }
    return _data.first().width();
}

int GifContentModel::height() const {
    if (_data.isEmpty()) {
        return -1;
    }
    return _data.first().height();
}

QSize GifContentModel::frameSize() const { return QSize(width(), height()); }

QVector<int> GifContentModel::delays() const { return _delays; }

QVector<QImage> GifContentModel::images() const { return _data; }

qsizetype GifContentModel::frameCount() const { return _data.size(); }

void GifContentModel::swapFrames(const QVector<QImage> &imgs) {
    Q_ASSERT(imgs.size() == _data.size());
    _data = imgs;
    emit dataChanged(this->index(0), this->index(_data.size() - 1));
    updateLinkedListViewCurrent();
}

void GifContentModel::cropFrames(const QRect &rect) {
    Q_ASSERT(rect.isValid());
    if (_data.isEmpty()) {
        return;
    }
    Q_ASSERT(_data.first().rect().contains(rect));

#pragma omp parallel for
    for (auto &img : _data) {
        img = img.copy(rect);
    }

    emit dataChanged(this->index(0), this->index(_data.size() - 1));
    updateLinkedListViewCurrent();
}

void GifContentModel::setFrameDelay(qsizetype index, int delay,
                                    qsizetype count) {
    Q_ASSERT(count != 0);
    if (count < 0) {
        for (qsizetype i = index; i < _delays.size(); ++i) {
            _delays.replace(i, delay);
        }
        emit dataChanged(this->index(index), this->index(_delays.size() - 1));
    } else {
        auto total = qMin(index + count, _delays.size());
        for (qsizetype i = index; i < total; ++i) {
            _delays.replace(i, delay);
        }
        emit dataChanged(this->index(index), this->index(total - 1));
    }
}

bool GifContentModel::setFrameImage(qsizetype index, const QImage &img) {
    if (index < 0 || index > _data.size()) {
        return false;
    }
    if (img.isNull()) {
        return false;
    }
    _data.replace(index, img.scaled(frameSize(), Qt::IgnoreAspectRatio,
                                    Qt::SmoothTransformation));
    updateLinkedListViewCurrent();

    return true;
}

void GifContentModel::scaleFrames(int width, int height) {
    Q_ASSERT(width > 0 && height > 0);
    if (_data.isEmpty()) {
        return;
    }

#pragma omp parallel for
    for (auto &img : _data) {
        img = img.scaled(width, height, Qt::IgnoreAspectRatio,
                         Qt::SmoothTransformation);
    }

    emit dataChanged(this->index(0), this->index(_data.size() - 1));
    updateLinkedListViewCurrent();
}

void GifContentModel::flipFrames(Qt::Orientation dir, int index,
                                 qsizetype count) {
    switch (dir) {
    case Qt::Horizontal: {
        for (auto &img : _data) {
            img = img.mirrored(true, false);
        }
        break;
    }
    case Qt::Vertical: {
        for (auto &img : _data) {
            img = img.mirrored();
        }
        break;
    }
    }
    emit dataChanged(this->index(0), this->index(_data.size() - 1));
}

void GifContentModel::reverseFrames(qsizetype begin, qsizetype end) {
    if ((end > 0 && end <= begin) || begin < 0 || end >= _data.size()) {
        return;
    }
    if (end < 0) {
        std::reverse(std::next(_data.begin(), begin), _data.end());
        std::reverse(std::next(_delays.begin(), begin), _delays.end());
        emit dataChanged(this->index(begin), this->index(_data.size() - 1));
    } else {
        std::reverse(std::next(_data.begin(), begin),
                     std::next(_data.begin(), end));
        std::reverse(std::next(_delays.begin(), begin),
                     std::next(_delays.begin(), end));
        emit dataChanged(this->index(begin), this->index(end));
    }
    updateLinkedListViewCurrent();
}

void GifContentModel::rotateFrames(bool clockwise) {
    QTransform trans;
    trans.rotate(clockwise ? -90 : 90);
    for (auto &img : _data) {
        img = img.transformed(trans, Qt::SmoothTransformation);
    }
    emit dataChanged(this->index(0), this->index(_data.size() - 1));
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
        moveRange(_data, index, index - 1, count);
        moveRange(_delays, index, index - 1, count);
    } break;
    case MoveFrameDirection::Right: {
        if (index + count == frameCount() - 1) {
            return;
        }
        auto b = beginMoveRows(QModelIndex(), index, index + count - 1,
                               QModelIndex(), index + 2);
        Q_ASSERT(b);
        moveRange(_data, index, index + 1, count);
        moveRange(_delays, index, index + 1, count);
    } break;
    }

    endMoveRows();
}

int GifContentModel::rowCount(const QModelIndex &parent) const {
    return _data.size();
}

QVariant GifContentModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole: {
        auto i = index.row();
        return QString::number(_delays.at(i)) + QStringLiteral(" ms");
    }
    case Qt::DecorationRole: {
        return _data.at(index.row());
    }
    case Qt::ToolTipRole: {
        break;
    }
    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
    }
    return QVariant();
}
