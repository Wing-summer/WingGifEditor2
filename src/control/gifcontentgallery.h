#ifndef GIFCONTENTGALLERY_H
#define GIFCONTENTGALLERY_H

#include <QListView>

class GifContentGallery : public QListView {
    Q_OBJECT
public:
    explicit GifContentGallery(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *e) override;
};

#endif // GIFCONTENTGALLERY_H
