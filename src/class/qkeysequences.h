#ifndef QKEYSEQUENCES_H
#define QKEYSEQUENCES_H

#include <QKeySequence>
#include <QMap>

class QKeySequences {
public:
    enum class Key {
        REDO,
        SAVE_AS,
        EXPORT,
        SEL_REV,
        GOTO,
        REDUCE_FRAME,
        DEL_BEFORE,
        DEL_AFTER,
        MOV_LEFT,
        MOV_RIGHT,
        REV_ALL,
        SET_DELAY,
        SCALE_PIC,
        SCALE_DELAY,
        CUT_PIC,
    };

public:
    QKeySequence keySequence(Key key) const;

    static const QKeySequences &instance();

private:
    QKeySequences();

private:
    QMap<Key, QKeySequence> _kseqs;
};

#endif // QKEYSEQUENCES_H
