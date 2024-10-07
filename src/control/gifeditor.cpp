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
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void GifEditor::setImage(const QImage &img) {
    scene->setFrameImg(img);
    resetTransform();
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
