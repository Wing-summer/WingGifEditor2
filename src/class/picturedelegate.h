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
