#ifndef GIFEDITOR_H
#define GIFEDITOR_H

#include "gifeditorscene.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QRubberBand>

class GifEditor : public QGraphicsView {
    Q_OBJECT
public:
    GifEditor(const QImage &img, QWidget *parent = nullptr);
    void setImage(const QImage &img);

    QRectF selRect() const;

public slots:
    void setSelRect(int x, int y, int w, int h);
    void fitOpenSize();

    void fitInEditorView();

    void setCropMode(bool b);

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
    bool iscut = true;
};

#endif // GIFEDITOR_H
