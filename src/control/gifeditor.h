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

#ifndef GIFEDITOR_H
#define GIFEDITOR_H

#include "gifeditorscene.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QRubberBand>

class GifEditor : public QGraphicsView {
    Q_OBJECT
public:
    enum class DrawTool { None, Rectangle, Ellipse };

    GifEditor(const QImage &img, QWidget *parent = nullptr);
    void setImage(const QImage &img);

    QRectF selRect() const;

    DrawTool drawTool() const;


public slots:
    void setSelRect(int x, int y, int w, int h);
    void fitOpenSize();

    void fitInEditorView();

    void setCropMode(bool b);

    void setDrawTool(DrawTool tool);

    void zoomIn();
    void zoomOut();
    void setZoom(int value);

signals:
    void selRectChanged(QRectF rect);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;

private:
    GifEditorScene *scene;
    QRubberBand *rubber;

    QPoint tmppos;
    QPointF drawStartPos;
    QGraphicsItem *activeShape = nullptr;
    DrawTool activeTool = DrawTool::None;
    bool iscut = true;
};

#endif // GIFEDITOR_H
