#ifndef RECENTFILEMANAGER_H
#define RECENTFILEMANAGER_H

#include <QAction>
#include <QList>
#include <QMenu>
#include <QObject>

class RecentFileManager : public QObject {
    Q_OBJECT
public:
    explicit RecentFileManager(QMenu *menu);
    ~RecentFileManager();
    void addRecentFile(const QString &info);
    void clearFile();
    void apply(QWidget *parent, const QStringList &files);

    const QStringList &saveRecent() const;

signals:
    void triggered(const QString &rinfo);

private:
    bool existsPath(const QString &info);
    QString getDisplayFileName(const QString &info);

private:
    QMenu *m_menu;
    QWidget *m_parent;
    QStringList m_recents;
    QList<QAction *> hitems;
};

#endif // RECENTFILEMANAGER_H
