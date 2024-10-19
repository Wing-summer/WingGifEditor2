#include "aboutsoftwaredialog.h"
#include "ui_aboutsoftwaredialog.h"

#include "utilities.h"

AboutSoftwareDialog::AboutSoftwareDialog(QWidget *parent)
    : QWidget(parent), ui(new Ui::AboutSoftwareDialog) {
    ui->setupUi(this);

    _dialog = new FramelessDialogBase(parent);
    _dialog->buildUpContent(this);
    _dialog->setWindowTitle(this->windowTitle());

    ui->lblVersion->setText(WINGGIF_VERSION);

    _dialog->setWindowTitle(tr("About"));

    Utilities::moveToCenter(this);
}

AboutSoftwareDialog::~AboutSoftwareDialog() {
    delete ui;
    _dialog->deleteLater();
}

int AboutSoftwareDialog::exec() { return _dialog->exec(); }
