#ifndef GIFSAVEDIALOG_H
#define GIFSAVEDIALOG_H

#include "framelessdialogbase.h"

#include <QLineEdit>
#include <QSpinBox>

struct GifSaveResult {
    unsigned int loop = 0;
    QString comment;
};

class GifSaveDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    explicit GifSaveDialog(QWidget *parent = nullptr);
    virtual ~GifSaveDialog();

    GifSaveResult getResult() const;

private slots:
    void on_accept();
    void on_reject();

private:
    GifSaveResult _res;

    QSpinBox *_loop;
    QLineEdit *_comment;
};

#endif // GIFSAVEDIALOG_H
