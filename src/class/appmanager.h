#ifndef APPMANAGER_H
#define APPMANAGER_H

#include "dialog/mainwindow.h"

#include <QApplication>

class AppManager : public QApplication {
    Q_OBJECT
public:
    explicit AppManager(int &argc, char *argv[]);
    virtual ~AppManager();

    static AppManager *instance();

    MainWindow *mainWindow() const;

public slots:
    void openFile(const QString &file);

private:
    MainWindow *_w = nullptr;
    static AppManager *_instance;
};

#endif // APPMANAGER_H
