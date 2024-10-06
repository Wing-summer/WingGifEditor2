#ifndef GIFCONTENTMODEL_H
#define GIFCONTENTMODEL_H

#include "gifreader.h"

#include <QAbstractListModel>
#include <QListView>
#include <QStyledItemDelegate>

#include "utilities.h"

class GifContentModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum class MoveFrameDirection { Left, Right };

public:
    explicit GifContentModel(QObject *parent = nullptr);

    void setLinkedListView(QListView *view);

    QListView *linkedListView() const;

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
    QVector<QImage> _data;
    QVector<int> _delays;

    QListView *_view = nullptr;
};

#endif // GIFCONTENTMODEL_H
