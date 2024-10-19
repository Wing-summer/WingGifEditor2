#ifndef GIFEDITORSCENE_H
#define GIFEDITORSCENE_H

#include "imagecropper/imagecropper.h"
#include <QGraphicsScene>

class GifEditorScene : public QGraphicsScene {
    Q_OBJECT
public:
    GifEditorScene(const QImage &img, QObject *parent = nullptr);
    virtual ~GifEditorScene();

    bool isCuttingMode();
    void setCuttingMode(bool value);

    void setFrameImg(const QImage &img);

    QRectF contentBounding() const;

    QSize frameImageSize() const;

    QRectF selRect() const;

public slots:
    void setSelRect(int x, int y, int w, int h);

signals:
    void selRectChanged(const QRectF &rect);

private:
    ImageCropper *sel;
    QPointF oldpos;
    QGraphicsProxyWidget *gw;
};

#endif // GIFEDITORSCENE_H
