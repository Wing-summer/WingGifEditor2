#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include "framelessdialogbase.h"

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QObject>
#include <QVBoxLayout>

enum class NewType { FromPics, FromGifs };

class NewDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    NewDialog(NewType type, QWidget *parent = nullptr);
    QStringList getResult();

private:
    void on_accept();
    void on_reject();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QStringList filenames;

    QListWidget *imgslist;
    QLabel *imgview;
    QLabel *imgsize;
};

#endif // NEWDIALOG_H
