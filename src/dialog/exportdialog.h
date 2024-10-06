#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include "QPathEdit/qpathedit.h"
#include "framelessdialogbase.h"

#include <QButtonGroup>
#include <QDialog>
#include <QMainWindow>
#include <QObject>

enum class ExportImageType { PNG, JPG, TIFF, WEBP };

struct ExportResult {
    QString path;
    ExportImageType type;
};

class ExportDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    ExportDialog(QWidget *parent = nullptr);
    ExportResult getResult();

private:
    void on_accept();
    void on_reject();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QPathEdit *folder;
    QButtonGroup *btnbox;

    ExportResult res;
};

#endif // EXPORTDIALOG_H
