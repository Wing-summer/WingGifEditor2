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

#include "picturedelegate.h"

#include <QPainter>

const unsigned int BANNER_HEIGHT = 20;
const unsigned int HIGHLIGHT_ALPHA = 100;

PictureDelegate::PictureDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

void PictureDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const {
    painter->save();

    auto img = index.model()->data(index, Qt::DecorationRole).value<QImage>();
    auto size = img.size().scaled(option.rect.width(),
                                  option.rect.height() - BANNER_HEIGHT * 2,
                                  Qt::KeepAspectRatio);

    painter->drawImage(
        QPoint(option.rect.left() + (option.rect.width() - size.width()) / 2,
               option.rect.top() + (option.rect.height() - size.height()) / 2),
        img.scaled(size));

    QRect bannerRectTop = QRect(option.rect.x(), option.rect.y(),
                                option.rect.width(), BANNER_HEIGHT);
    QRect bannerRectBottom = bannerRectTop;
    bannerRectBottom.moveBottom(option.rect.bottom());

    QColor bannerColor = this->bannerColor;
    painter->fillRect(bannerRectTop, bannerColor);
    painter->fillRect(bannerRectBottom, bannerColor);

    QString filename = index.model()->data(index, Qt::DisplayRole).toString();
    painter->setPen(this->bannerTextColor);
    painter->drawText(bannerRectTop, Qt::AlignCenter,
                      QString::number(index.row() + 1));
    painter->drawText(bannerRectBottom, Qt::AlignCenter, filename);

    if (option.state.testFlag(QStyle::State_Selected)) {
        QColor selectedColor = option.palette.highlight().color();
        selectedColor.setAlpha(HIGHLIGHT_ALPHA);
        painter->fillRect(option.rect, selectedColor);
    }

    painter->restore();
}

QSize PictureDelegate::sizeHint(const QStyleOptionViewItem &option,
                                const QModelIndex &index) const {
    auto s = option.widget->height() - 40;
    return QSize(s * 5 / 4, s);
}

QColor PictureDelegate::getBannerTextColor() const { return bannerTextColor; }

void PictureDelegate::setBannerTextColor(const QColor &newBannerTextColor) {
    if (bannerTextColor == newBannerTextColor)
        return;
    bannerTextColor = newBannerTextColor;
    emit bannerTextColorChanged();
}

QColor PictureDelegate::getBannerColor() const { return bannerColor; }

void PictureDelegate::setBannerColor(const QColor &newBannerColor) {
    if (bannerColor == newBannerColor)
        return;
    bannerColor = newBannerColor;
    emit bannerColorChanged();
}
