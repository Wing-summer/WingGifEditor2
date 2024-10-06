#include "qkeysequences.h"

QKeySequence QKeySequences::keySequence(Key key) const {
    return _kseqs.value(key, QKeySequence());
}

const QKeySequences &QKeySequences::instance() {
    static QKeySequences instance;
    return instance;
}

QKeySequences::QKeySequences() {
    _kseqs = {
        {Key::REDO,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_Y)},
        {Key::SAVE_AS,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::ShiftModifier | Qt::Key_S)},
        {Key::EXPORT,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::ShiftModifier | Qt::Key_E)},
        {Key::SEL_REV,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_I)},
        {Key::GOTO,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_G)},
        {Key::REDUCE_FRAME,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_D)},
        {Key::DEL_BEFORE,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_J)},
        {Key::DEL_AFTER,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_K)},
        {Key::MOV_LEFT, QKeySequence(Qt::KeyboardModifier::ControlModifier |
                                     Qt::Key_BracketLeft)},
        {Key::MOV_RIGHT, QKeySequence(Qt::KeyboardModifier::ControlModifier |
                                      Qt::Key_BracketRight)},
        {Key::REV_ALL,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::ShiftModifier | Qt::Key_I)},
        {Key::SET_DELAY,
         QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key_T)},
        {Key::SCALE_PIC,
         QKeySequence(Qt::KeyboardModifier::ShiftModifier | Qt::Key_T)},
        {Key::CUT_PIC,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::ShiftModifier | Qt::Key_X)},
        {Key::SCALE_DELAY,
         QKeySequence(Qt::KeyboardModifier::ControlModifier |
                      Qt::KeyboardModifier::ShiftModifier | Qt::Key_T)}};
}
