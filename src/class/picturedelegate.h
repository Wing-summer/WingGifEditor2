#ifndef PICTUREDELEGATE_H
#define PICTUREDELEGATE_H

#include <QStyledItemDelegate>

class PictureDelegate : public QStyledItemDelegate {
    Q_OBJECT

    Q_PROPERTY(QColor bannerTextColor READ getBannerTextColor WRITE
                   setBannerTextColor NOTIFY bannerTextColorChanged FINAL)
    Q_PROPERTY(QColor bannerColor READ getBannerColor WRITE setBannerColor
                   NOTIFY bannerColorChanged FINAL)
public:
    PictureDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    QColor getBannerTextColor() const;
    void setBannerTextColor(const QColor &newBannerTextColor);

    QColor getBannerColor() const;
    void setBannerColor(const QColor &newBannerColor);

signals:
    void bannerTextColorChanged();

    void bannerColorChanged();

private:
    QColor bannerTextColor = 0xffffff;
    QColor bannerColor = 0x303030;
};

#endif // PICTUREDELEGATE_H
