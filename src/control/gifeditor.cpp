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

#include "gifeditor.h"
#include <QGuiApplication>
#include <QWheelEvent>

GifEditor::GifEditor(const QImage &img, QWidget *parent)
    : QGraphicsView(parent) {
    scene = new GifEditorScene(img, this);
    connect(scene, &GifEditorScene::selRectChanged, this,
            &GifEditor::selRectChanged);
    setScene(scene);
    rubber = new QRubberBand(QRubberBand::Line, this);
}

void GifEditor::setImage(const QImage &img) { scene->setFrameImg(img); }

void GifEditor::fitOpenSize() {
    auto imgSize = scene->frameImageSize();
    auto wSize = this->size();
    if (imgSize.width() > wSize.width() || imgSize.height() > wSize.height()) {
        fitInEditorView();
    } else {
        auto r = scene->contentBounding();
        setSceneRect(r);
        resetTransform();
    }
}

void GifEditor::fitInEditorView() {
    auto r = scene->contentBounding();
    setSceneRect(r);
    fitInView(r, Qt::KeepAspectRatio);
}

void GifEditor::setCropMode(bool b) { scene->setCuttingMode(b); }

void GifEditor::zoomIn() { scale(1.1, 1.1); }

void GifEditor::zoomOut() { scale(0.9, 0.9); }

void GifEditor::setZoom(int value) {
    resetTransform();
    qreal v = value / 100.0;
    scale(v, v);
}

QRectF GifEditor::selRect() const { return scene->selRect(); }

void GifEditor::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MouseButton::LeftButton &&
        !scene->isCuttingMode()) {
        tmppos = event->pos();

        rubber->setGeometry(QRect(tmppos, QSize()));
        rubber->show();
    }
    QGraphicsView::mousePressEvent(event);
}

void GifEditor::mouseMoveEvent(QMouseEvent *event) {
    rubber->setGeometry(QRect(tmppos, event->pos()).normalized());
    QGraphicsView::mouseMoveEvent(event);
}

void GifEditor::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    rubber->hide();
    QGraphicsView::mouseReleaseEvent(event);
}

void GifEditor::wheelEvent(QWheelEvent *event) {
    auto mod = QGuiApplication::keyboardModifiers();
    if (mod != Qt::KeyboardModifier::ControlModifier)
        return;

    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta();

    if (!numPixels.isNull()) {
        if (numPixels.y() < 0) {
            zoomOut();
        } else {
            zoomIn();
        }
    } else if (!numDegrees.isNull()) {
        if (numDegrees.y() < 0) {
            zoomOut();
        } else {
            zoomIn();
        }
    }

    event->accept();
}

void GifEditor::setSelRect(int x, int y, int w, int h) {
    scene->setSelRect(x, y, w, h);
}

void GifEditor::leaveEvent(QEvent *event) { qApp->restoreOverrideCursor(); }
