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

#ifndef GIFCONTENTMODEL_H
#define GIFCONTENTMODEL_H

#include "gifreader.h"

#include <QAbstractListModel>
#include <QCache>
#include <QListView>
#include <QStyledItemDelegate>
#include <QTemporaryDir>

#include "control/gifeditor.h"
#include "utilities.h"

class GifContentModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum class MoveFrameDirection { Left, Right };

public:
    explicit GifContentModel(QObject *parent = nullptr);

    void setLinkedListView(QListView *view);

    void setLinkedEditor(GifEditor *editor);

    QListView *linkedListView() const;

    GifEditor *linkedGifEditor() const;

    QImage image(qsizetype index) const;

    int delay(qsizetype index) const;

    int width() const;

    int height() const;

    QSize frameSize() const;

    QVector<int> delays() const;

    QVector<QImage> images() const;

    qsizetype frameCount() const;

    static bool isValidGifFrame(const QImage &image, int delay);

    static int delayLimitMin();

    static int delayLimitMax();

public slots:
    void readGifReader(GifReader *reader);

    bool insertFrame(const GifData &frame, int index);

    bool insertFrame(const QImage &image, int delay, int index);

    void insertFrames(const QVector<GifData> &frames, int index);

    void insertFrames(const QVector<QImage> &images, const QVector<int> &delays,
                      int index);

    void removeFrames(int index, qsizetype count = 1);

    void swapFrames(const QVector<QImage> &imgs);

    void cropFrames(const QRect &rect);

    void setFrameDelay(qsizetype index, int delay, qsizetype count = 1);

    bool setFrameImage(qsizetype index, const QImage &img);

    void scaleFrames(int width, int height);

    void flipFrames(Qt::Orientation dir, int index, qsizetype count = 1);

    void reverseFrames(qsizetype begin = 0, qsizetype end = -1);

    void rotateFrames(bool clockwise);

    void moveFrames(qsizetype index, MoveFrameDirection dir,
                    qsizetype count = 1);

    void clearData();

private:
    void updateLinkedListViewCurrent() const;

    void insertFrames(const QVector<QImage> &images, const QVector<int> &delays,
                      int index, bool processed);

    bool ensureStoreDir();
    QString framePathForIndex(qsizetype index) const;
    void persistFrame(qsizetype index, const QImage &image);
    QImage loadFrame(qsizetype index) const;

    template <typename T>
    void moveRange(QVector<T> &data, qsizetype pos, qsizetype dest,
                   qsizetype count = -1) {
        auto src = data.mid(pos, count);
        data.remove(pos, count);
        data.insert(dest, count, T());
        for (int i = 0; i < count; ++i) {
            data.replace(dest + i, src.at(i));
        }
    }

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;

private:
    QSize _frameSize;
    QVector<QString> _frameFiles;
    mutable QCache<qsizetype, QImage> _frameCache;
    std::unique_ptr<QTemporaryDir> _storeDir;
    QVector<int> _delays;

    QListView *_view = nullptr;
    GifEditor *_editor = nullptr;
};

#endif // GIFCONTENTMODEL_H
