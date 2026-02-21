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

#ifndef GIFCONTENTMODEL_H
#define GIFCONTENTMODEL_H

#include "giffile.h"

#include <QAbstractListModel>
#include <QCache>
#include <QListView>
#include <QStyledItemDelegate>
#include <QTemporaryDir>

#include "control/gifeditor.h"

class GifContentModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum class MoveFrameDirection { Left, Right };

    struct Result {
        using R = QPair<QSize, QVector<QSharedPointer<GifFrame>>>;

        R olddata;
        R newdata;

        inline bool isValid() const {
            return !olddata.first.isEmpty() && !newdata.first.isEmpty();
        }
    };

public:
    explicit GifContentModel(QObject *parent = nullptr);

    void setLinkedListView(QListView *view);

    void setLinkedEditor(GifEditor *editor);

    QListView *linkedListView() const;

    GifEditor *linkedGifEditor() const;

    QSharedPointer<GifFrame> frame(qsizetype index) const;

    QImage image(qsizetype index) const;

    int delay(qsizetype index) const;

    QString comment() const;

    int width() const;

    int height() const;

    QSize frameSize() const;

    qsizetype frameCount() const;

public:
    GifFile::ErrorCode readGifFile(const QString &gif);

    void clearData();

public:
    QSharedPointer<GifFrame> generateFrame(const QImage &image, int delay);

    void insertFrame(qsizetype index, const QSharedPointer<GifFrame> &frame);

    void insertFrames(qsizetype index,
                      const QVector<QSharedPointer<GifFrame>> &frames);

    QVector<QSharedPointer<GifFrame>> removeFrames(qsizetype index,
                                                   qsizetype count);

    QSharedPointer<GifFrame> removeFrame(qsizetype index);

    QPair<QSize, QVector<QSharedPointer<GifFrame>>>
    replaceFrames(const QSize &frameSize,
                  const QVector<QSharedPointer<GifFrame>> &frames);

    void replaceFrame(qsizetype index, const QSharedPointer<GifFrame> frame);

    QMap<int, std::variant<QPair<int, int>, QSharedPointer<GifFrame>>>
    reduceFrameDryRun(qsizetype begin, qsizetype end, qsizetype step);

    void applyReduceFrame(
        const QMap<int, std::variant<QPair<int, int>, QSharedPointer<GifFrame>>>
            &data);

    void undoReduceFrame(
        const QMap<int, std::variant<QPair<int, int>, QSharedPointer<GifFrame>>>
            &data);

public:
    Result cropFrames(const QRect &rect);

    Result rotateFrames(bool clockwise);

    QMap<int, QSharedPointer<GifFrame>> flipFrames(const QVector<int> &indices,
                                                   Qt::Orientation dir);

    Result scaleFrames(int width, int height);

    void setFrameDelay(qsizetype index, int delay);

    void reverseFrames(qsizetype begin, qsizetype end);

    Result setFrameImage(qsizetype index, const QImage &img);

public:
    void moveFrames(qsizetype index, GifContentModel::MoveFrameDirection dir,
                    qsizetype count = 1);

signals:
    void sigUpdateUIProcess();

private:
    void updateLinkedListViewCurrent() const;

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;

private:
    GifFile _file;

    QListView *_view = nullptr;
    GifEditor *_editor = nullptr;
};

#endif // GIFCONTENTMODEL_H
