/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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
#include <QtConcurrent/QtConcurrentMap>

GifContentModel::GifContentModel(QObject *parent) : QAbstractListModel(parent) {
    connect(&_file, &GifFile::sigUpdateUIProcess, this,
            &GifContentModel::sigUpdateUIProcess);
}

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

QSharedPointer<GifFrame> GifContentModel::frame(qsizetype index) const {
    return _file.frame(index);
}

void GifContentModel::updateLinkedListViewCurrent() const {
    if (_view) {
        auto i = _view->currentIndex();
        emit _view->selectionModel()->currentRowChanged(i, i);
    }
}

GifFile::ErrorCode GifContentModel::readGifFile(const QString &gif) {
    auto r = _file.load(gif);
    emit layoutChanged();
    return r;
}

void GifContentModel::clearData() {
    _file.clear();
    emit layoutChanged();
}

QSharedPointer<GifFrame> GifContentModel::generateFrame(const QImage &image,
                                                        int delay) {
    return _file.generateFrame(image, delay);
}

void GifContentModel::insertFrame(qsizetype index,
                                  const QSharedPointer<GifFrame> &frame) {
    beginInsertRows(QModelIndex(), index, index);
    _file.insertFrame(index, frame);
    endInsertRows();
}

void GifContentModel::insertFrames(
    qsizetype index, const QVector<QSharedPointer<GifFrame>> &frames) {
    beginInsertRows(QModelIndex(), index, index + frames.size() - 1);
    _file.insertFrames(index, frames);
    endInsertRows();
}

QVector<QSharedPointer<GifFrame>>
GifContentModel::removeFrames(qsizetype index, qsizetype count) {
    if (count < 0) {
        beginRemoveRows(QModelIndex(), index, this->frameCount() - 1);
    } else {
        beginRemoveRows(QModelIndex(), index, index + count - 1);
    }

    auto ret = _file.removeFrames(index, count);
    endRemoveRows();
    return ret;
}

QSharedPointer<GifFrame> GifContentModel::removeFrame(qsizetype index) {
    beginRemoveRows(QModelIndex(), index, index);
    auto ret = _file.removeFrame(index);
    endRemoveRows();
    return ret;
}

QPair<QSize, QVector<QSharedPointer<GifFrame>>> GifContentModel::replaceFrames(
    const QSize &frameSize, const QVector<QSharedPointer<GifFrame>> &frames) {
    Q_ASSERT(frames.size() == frameCount());
    auto r = _file.replaceFrames(frameSize, frames);
    emit dataChanged(this->index(0), this->index(frameCount() - 1));
    updateLinkedListViewCurrent();
    return r;
}

void GifContentModel::replaceFrame(qsizetype index,
                                   const QSharedPointer<GifFrame> frame) {
    Q_ASSERT(frame);
    _file.replaceFrame(index, frame);
    updateLinkedListViewCurrent();
    emit dataChanged(this->index(index), this->index(index));
}

QMap<int, std::variant<QPair<int, int>, QSharedPointer<GifFrame>>>
GifContentModel::reduceFrameDryRun(qsizetype begin, qsizetype end,
                                   qsizetype step) {
    QMap<int, std::variant<QPair<int, int>, QSharedPointer<GifFrame>>> ret;
    auto ii = begin;
    auto q = step + 1;
    for (auto i = ii; i <= end; i++) {
        if (i == ii + step) {
            ii += q;
            ret.insert(i, frame(i));
        } else {
            auto d = delay(i);
            ret.insert(i, qMakePair(d, int(d * (q + 1) / q)));
        }
    }
    return ret;
}

void GifContentModel::applyReduceFrame(
    const QMap<int, std::variant<QPair<int, int>, QSharedPointer<GifFrame>>>
        &data) {

    for (auto &&[idx, value] : data.asKeyValueRange()) {
        if (std::holds_alternative<QPair<int, int>>(value)) {
            auto delayPair = std::get<QPair<int, int>>(value);
            auto frame = this->frame(idx);
            frame->setDelay(delayPair.second);
        }
    }

    _file.removeFrameIf(
        [data](qsizetype index, QSharedPointer<GifFrame>) -> bool {
            auto r = data.value(index);
            return std::holds_alternative<QSharedPointer<GifFrame>>(r);
        });
    updateLinkedListViewCurrent();
    emit layoutChanged();
}

void GifContentModel::undoReduceFrame(
    const QMap<int, std::variant<QPair<int, int>, QSharedPointer<GifFrame>>>
        &data) {
    for (auto &&[idx, value] : data.asKeyValueRange()) {
        if (std::holds_alternative<QPair<int, int>>(value)) {
            auto delayPair = std::get<QPair<int, int>>(value);
            auto frame = this->frame(idx);
            frame->setDelay(delayPair.first);
        } else {
            auto frame = std::get<QSharedPointer<GifFrame>>(value);
            _file.insertFrame(idx, frame);
        }
    }
    updateLinkedListViewCurrent();
    emit layoutChanged();
}

QImage GifContentModel::image(qsizetype index) const {
    return _file.image(index);
}

int GifContentModel::delay(qsizetype index) const { return _file.delay(index); }

QString GifContentModel::comment() const { return _file.comment(); }

int GifContentModel::width() const { return _file.width(); }

int GifContentModel::height() const { return _file.height(); }

QSize GifContentModel::frameSize() const { return _file.size(); }

qsizetype GifContentModel::frameCount() const { return _file.frameCount(); }

GifContentModel::Result GifContentModel::cropFrames(const QRect &rect) {
    Q_ASSERT(rect.isValid());
    auto r = QtConcurrent::blockingMapped<GifFile::data_type>(
        _file.begin(), _file.end(),
        [this, rect](
            const QSharedPointer<GifFrame> &frame) -> QSharedPointer<GifFrame> {
            auto delay = frame->delay();
            auto image = frame->image();
            return _file.generateFrame(image.copy(rect), delay);
        });
    auto nsize = rect.size();
    auto old = replaceFrames(nsize, r);
    emit dataChanged(this->index(0), this->index(frameCount() - 1));
    updateLinkedListViewCurrent();
    return {old, {nsize, r}};
}

void GifContentModel::setFrameDelay(qsizetype index, int delay) {
    auto frame = _file.frame(index);
    frame->setDelay(delay);
    emit dataChanged(this->index(index), this->index(index));
}

GifContentModel::Result GifContentModel::setFrameImage(qsizetype index,
                                                       const QImage &img) {
    if (index < 0 || index >= frameCount()) {
        return {};
    }
    if (img.size() != frameSize()) {
        return {};
    }

    auto size = frameSize();
    auto frame = _file.frame(index);
    auto nframe = _file.generateFrame(img, frame->delay());
    _file.replaceFrame(index, nframe);
    updateLinkedListViewCurrent();

    return {{size, {frame}}, {size, {nframe}}};
}

GifContentModel::Result GifContentModel::scaleFrames(int width, int height) {
    Q_ASSERT(width > 0 && height > 0);
    auto r = QtConcurrent::blockingMapped<GifFile::data_type>(
        _file.begin(), _file.end(),
        [this, width, height](
            const QSharedPointer<GifFrame> &frame) -> QSharedPointer<GifFrame> {
            auto delay = frame->delay();
            auto image = frame->image();
            return _file.generateFrame(image.scaled(width, height,
                                                    Qt::IgnoreAspectRatio,
                                                    Qt::SmoothTransformation),
                                       delay);
        });

    auto nsize = QSize(width, height);
    auto old = replaceFrames(nsize, r);
    emit dataChanged(this->index(0), this->index(frameCount() - 1));
    updateLinkedListViewCurrent();
    return {old, {nsize, r}};
}

QMap<int, QSharedPointer<GifFrame>>
GifContentModel::flipFrames(const QVector<int> &indices, Qt::Orientation dir) {
    QMap<int, QSharedPointer<GifFrame>> result;
    QtConcurrent::blockingMappedReduced(
        indices,
        [this, dir](int index) -> QPair<int, QSharedPointer<GifFrame>> {
            auto frame = _file.frame(index);
            auto delay = frame->delay();
            auto image = frame->image();

            return qMakePair(index,
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
                             _file.generateFrame(image.flipped(dir), delay)
#else
                             _file.generateFrame(
                                 image.mirrored(dir == Qt::Horizontal,
                                                dir == Qt::Vertical),
                                 delay)
#endif
            );
        },
        [this](QMap<int, QSharedPointer<GifFrame>> &result,
               const QPair<int, QSharedPointer<GifFrame>> &data) {
            auto index = data.first;
            auto frame = data.second;
            result.insert(index, frame);
            replaceFrame(index, frame);
            emit dataChanged(this->index(index), this->index(index));
        },
        std::ref(result));
    return result;
}

void GifContentModel::reverseFrames(qsizetype begin, qsizetype end) {
    if ((end > 0 && end <= begin) || begin < 0 || end >= frameCount()) {
        return;
    }
    if (end < 0) {
        std::reverse(std::next(_file.begin(), begin), _file.end());
        emit dataChanged(this->index(begin), this->index(frameCount() - 1));
    } else {
        std::reverse(std::next(_file.begin(), begin),
                     std::next(_file.begin(), end));
        emit dataChanged(this->index(begin), this->index(end));
    }
    updateLinkedListViewCurrent();
}

GifContentModel::Result GifContentModel::rotateFrames(bool clockwise) {
    QTransform trans;
    trans.rotate(clockwise ? -90 : 90);
    auto rect = frameSize();
    auto r = QtConcurrent::blockingMapped<GifFile::data_type>(
        _file.begin(), _file.end(),
        [this, trans](
            const QSharedPointer<GifFrame> &frame) -> QSharedPointer<GifFrame> {
            auto delay = frame->delay();
            auto image = frame->image();
            return _file.generateFrame(
                image.transformed(trans, Qt::SmoothTransformation), delay);
        });
    auto nsize = rect.transposed();
    auto old = replaceFrames(nsize, r);
    emit dataChanged(this->index(0), this->index(frameCount() - 1));
    updateLinkedListViewCurrent();
    return {old, {nsize, r}};
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
        auto src_begin = std::next(_file.begin(), index);
        auto src_end = std::next(src_begin, count);
        auto dest = std::prev(src_begin);
        std::rotate(dest, src_begin, src_end);
    } break;
    case MoveFrameDirection::Right: {
        if (index + count == frameCount() - 1) {
            return;
        }
        auto b = beginMoveRows(QModelIndex(), index, index + count - 1,
                               QModelIndex(), index + 2);
        Q_ASSERT(b);
        auto src_begin = std::next(_file.begin(), index);
        auto src_end = std::next(src_begin, count);
        auto dest = std::next(src_begin);
        std::rotate(dest, src_begin, src_end);
    } break;
    }

    endMoveRows();
}

int GifContentModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return frameCount();
}

QVariant GifContentModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case Qt::DisplayRole: {
        auto i = index.row();
        return QString::number(delay(i)) + QStringLiteral(" ms");
    }
    case Qt::DecorationRole: {
        return image(index.row());
    }
    case Qt::ToolTipRole: {
        break;
    }
    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
    }
    return {};
}
