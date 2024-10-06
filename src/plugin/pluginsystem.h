#ifndef PLUGINSYSTEM_H
#define PLUGINSYSTEM_H

#include "iwingplugin.h"

#include <QObject>

using namespace WingGif;

class MainWindow;

class PluginSystem : public QObject {
    Q_OBJECT
public:
    static PluginSystem &instance();

    void setMainWindow(MainWindow *win);
    QWidget *mainWindow() const;

    const QList<IWingPlugin *> &plugins() const;

    const IWingPlugin *plugin(qsizetype index) const;

signals:

private:
    PluginSystem(QObject *parent = nullptr);
    ~PluginSystem();

private:
    bool checkThreadAff();
    void connectUIInterface(IWingPlugin *plg);

private:
    bool loadPlugin(IWingPlugin *p);
    void connectBaseInterface(IWingPlugin *plg);
    void connectInterface(IWingPlugin *plg);

    QString packLogMessage(const char *header, const QString &msg);

private:
    MainWindow *_win = nullptr;

    QStringList loadedpuid;
    QList<IWingPlugin *> loadedplgs;
};

#endif // PLUGINSYSTEM_H
