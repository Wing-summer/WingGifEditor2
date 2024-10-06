#include "gifcontentgallery.h"

#include "class/picturedelegate.h"

GifContentGallery::GifContentGallery(QWidget *parent) : QListView(parent) {
    this->setWrapping(false);
    this->setFlow(QListView::LeftToRight);
    this->setViewMode(QListView::IconMode);
    this->setResizeMode(QListView::Adjust);
    this->setUniformItemSizes(true);
    this->setSpacing(10);
    this->setItemDelegate(new PictureDelegate(this));
}

void GifContentGallery::resizeEvent(QResizeEvent *e) {
    QListView::resizeEvent(e);
    if (this->model()) {
        emit this->model()->layoutChanged();
    }
}
