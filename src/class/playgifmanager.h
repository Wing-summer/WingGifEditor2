#ifndef PLAYGIFMANAGER_H
#define PLAYGIFMANAGER_H

#include <QObject>
#include <QTimer>
#include <QVector>

class PlayGifManager : public QObject {
    Q_OBJECT
public:
    explicit PlayGifManager(QObject *parent = nullptr);
    void setTickIntervals(const QVector<int> &intervals);
    void play(int index);
    void stop();

    bool isPlaying() const;

signals:
    void tick(int index);

    void playStateChanged();

private:
    void tick_p();

private:
    int curpos = 0;
    bool _isContinue = false;
    QVector<int> m_intervals;
};

#endif // PLAYGIFMANAGER_H
