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
