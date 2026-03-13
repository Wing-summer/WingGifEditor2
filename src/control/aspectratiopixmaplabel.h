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
#ifndef ASPECTRATIOPIXMAPLABEL_H
#define ASPECTRATIOPIXMAPLABEL_H

#include <QLabel>

class AspectRatioPixmapLabel : public QLabel {
    Q_OBJECT
public:
    explicit AspectRatioPixmapLabel(QWidget *parent = nullptr);

public slots:
    void setPixmap(const QPixmap &pm);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateMargins();

    int pixmapWidth = 0;
    int pixmapHeight = 0;
};

#endif // ASPECTRATIOPIXMAPLABEL_H
