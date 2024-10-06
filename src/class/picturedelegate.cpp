#include "picturedelegate.h"

#include <QPainter>

const unsigned int BANNER_HEIGHT = 20;
const unsigned int BANNER_COLOR = 0x303030;
const unsigned int BANNER_ALPHA = 200;
const unsigned int BANNER_TEXT_COLOR = 0xffffff;
const unsigned int HIGHLIGHT_ALPHA = 100;

PictureDelegate::PictureDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

void PictureDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const {
    painter->save();

    auto img = index.model()->data(index, Qt::DecorationRole).value<QImage>();
    auto size = img.size().scaled(option.rect.size(), Qt::KeepAspectRatio);

    painter->drawImage(
        QPoint(option.rect.left() + (option.rect.width() - size.width()) / 2,
               option.rect.top() + (option.rect.height() - size.height()) / 2),
        img.scaled(size));

    QRect bannerRectTop = QRect(option.rect.x(), option.rect.y(),
                                option.rect.width(), BANNER_HEIGHT);
    QRect bannerRectBottom = bannerRectTop;
    bannerRectBottom.moveBottom(option.rect.bottom());

    QColor bannerColor = QColor(BANNER_COLOR);
    bannerColor.setAlpha(BANNER_ALPHA);
    painter->fillRect(bannerRectTop, bannerColor);
    painter->fillRect(bannerRectBottom, bannerColor);

    QString filename = index.model()->data(index, Qt::DisplayRole).toString();
    painter->setPen(BANNER_TEXT_COLOR);
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
