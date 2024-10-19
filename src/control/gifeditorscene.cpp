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
    gw = addWidget(sel);
    connect(sel, &ImageCropper::sizeChanged, gw, [=] {
        // call the internal updateProxyGeometryFromWidget
        gw->resetTransform();
        gw->setWidget(nullptr);
        gw->setWidget(sel);
        // center the image
        setSceneRect(QRectF(QPointF(), gw->size()));
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

QRectF GifEditorScene::contentBounding() const { return gw->boundingRect(); }

QSize GifEditorScene::frameImageSize() const { return sel->imageSize(); }

QRectF GifEditorScene::selRect() const { return sel->selRect(); }

void GifEditorScene::setSelRect(int x, int y, int w, int h) {
    sel->setSelRect(x, y, w, h);
}
