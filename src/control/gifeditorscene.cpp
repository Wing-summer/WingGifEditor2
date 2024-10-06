#include "gifeditorscene.h"
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>

GifEditorScene::GifEditorScene(const QImage &img, QObject *parent)
    : QGraphicsScene(parent) {
    sel = new ImageCropper();
    sel->setAttribute(Qt::WA_TranslucentBackground);
    sel->setBackgroundColor(Qt::transparent);
    sel->setCropperVisible(false);
    sel->setImage(img);
    auto w = addWidget(sel);
    connect(sel, &ImageCropper::sizeChanged, w, [=] {
        // call the internal updateProxyGeometryFromWidget
        w->resetTransform();
        w->setWidget(nullptr);
        w->setWidget(sel);
        // center the image
        setSceneRect(QRectF(QPointF(), w->size()));
    });
    connect(sel, &ImageCropper::selRectChanged, this,
            &GifEditorScene::selRectChanged);
}

GifEditorScene::~GifEditorScene() { sel->deleteLater(); }

bool GifEditorScene::isCuttingMode() { return sel->cropperVisible(); }

void GifEditorScene::setCuttingMode(bool value) {
    sel->setCropperVisible(value);
}

void GifEditorScene::setFrameImg(const QImage &img) { sel->setImage(img); }

QRectF GifEditorScene::selRect() const { return sel->selRect(); }

void GifEditorScene::setSelRect(int x, int y, int w, int h) {
    sel->setSelRect(x, y, w, h);
}
