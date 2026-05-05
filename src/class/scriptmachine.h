/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#ifndef SCRIPTMACHINE_H
#define SCRIPTMACHINE_H

#include "angelscript.h"
#include "scriptaddon/contextmgr.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QObject>
#include <QQueue>

class CScriptArray;

class ScriptMachine {
    Q_GADGET
public:
    enum class MessageType { Unknown, Info, Warn, Error, Print, ExecInfo };

    struct MessageInfo {
        QString section;
        qint64 row = -1;
        qint64 col = -1;
        MessageType type = MessageType::Info;
        QString message;
    };

public:
    // only for refection
    enum class asEContextState {
        asEXECUTION_FINISHED = ::asEContextState::asEXECUTION_FINISHED,
        asEXECUTION_SUSPENDED = ::asEContextState::asEXECUTION_SUSPENDED,
        asEXECUTION_ABORTED = ::asEContextState::asEXECUTION_ABORTED,
        asEXECUTION_EXCEPTION = ::asEContextState::asEXECUTION_EXCEPTION,
        asEXECUTION_PREPARED = ::asEContextState::asEXECUTION_PREPARED,
        asEXECUTION_UNINITIALIZED =
            ::asEContextState::asEXECUTION_UNINITIALIZED,
        asEXECUTION_ACTIVE = ::asEContextState::asEXECUTION_ACTIVE,
        asEXECUTION_ERROR = ::asEContextState::asEXECUTION_ERROR,
        asEXECUTION_DESERIALIZATION =
            ::asEContextState::asEXECUTION_DESERIALIZATION
    };
    Q_ENUM(asEContextState)

public:
    struct RegCallBacks {
        std::function<QString()> getInputFn;
        std::function<void()> clearFn;
        std::function<void(const ScriptMachine::MessageInfo &)> printMsgFn;
    };

private:
    explicit ScriptMachine();
    Q_DISABLE_COPY_MOVE(ScriptMachine)

public:
    static ScriptMachine &instance();
    void destoryMachine();

public:
    bool init();
    bool isInited() const;
    bool isRunning() const;
    bool checkEngineConfigError() const;

    static void registerEngineAddon(asIScriptEngine *engine);
    static void registerEngineAssert(asIScriptEngine *engine);
    static void registerEngineDebug(asIScriptEngine *engine);

    static void installAPI(ScriptMachine *machine);
    static void installBasicTypes(asIScriptEngine *engine);
    static void installExtAPI(asIScriptEngine *engine);
    static void installMsgboxAPI(asIScriptEngine *engine);
    static void installInputboxAPI(asIScriptEngine *engine);
    static void installFileDialogAPI(asIScriptEngine *engine);
    static void installColorDialogAPI(asIScriptEngine *engine);
    static void installGifEditAPI(asIScriptEngine *engine);

    void registerCallBack(const RegCallBacks &callbacks);

public:
    bool isAngelChar(int typeID) const;
    bool isAngelString(int typeID) const;
    bool isAngelArray(int typeID) const;
    bool isAngelDictionary(int typeID) const;
    bool isAngelDicValue(int typeID) const;
    bool isAngelAny(int typeID) const;

public:
    asIScriptEngine *engine() const;

    void outputMessage(const MessageInfo &info);

public:
    static void scriptAssert(bool b);
    static void scriptAssert_X(bool b, const QString &msg);

    static void scriptThrow(const QString &msg);

    static QString scriptGetExceptionInfo();

    static void registerExceptionRoutines(asIScriptEngine *engine);

public:
    bool executeScript(const QString &script,
                       const std::function<void(bool)> &onFinished);

    void abortScript();

    QString getAsTypeName(int typeId);

protected:
    bool configureEngine();

    QString getCallStack(asIScriptContext *context);

private:
    static void __output(MessageType type, asIScriptGeneric *args);
    static void __outputln(MessageType type, asIScriptGeneric *args);

    static void print(asIScriptGeneric *args);
    static void println(asIScriptGeneric *args);

    static void warnprint(asIScriptGeneric *args);
    static void warnprintln(asIScriptGeneric *args);

    static void errprint(asIScriptGeneric *args);
    static void errprintln(asIScriptGeneric *args);

    static void infoprint(asIScriptGeneric *args);
    static void infoprintln(asIScriptGeneric *args);

    QString input();

    QString stringify(void *ref, int typeId);

    template <typename T>
    static inline const T *resolveObjAs(const void *address, int typeId) {
        if (!address)
            return nullptr;
        else if (typeId & (asTYPEID_HANDLETOCONST | asTYPEID_OBJHANDLE)) {
            return *reinterpret_cast<const T *const *>(address);
        }
        return reinterpret_cast<const T *>(address);
    }

private:
    static void _UI_Toast(const QString &message, const QString &icon);

    static QColor _Color_get(const QString &caption);

    static void _MSG_AboutQt(const QString &title);

    static QMessageBox::StandardButton
    _MSG_Information(const QString &title, const QString &text, int buttons,
                     QMessageBox::StandardButton defaultButton);

    static QMessageBox::StandardButton
    _MSG_Question(const QString &title, const QString &text, int buttons,
                  QMessageBox::StandardButton defaultButton);

    static QMessageBox::StandardButton
    _MSG_Warning(const QString &title, const QString &text, int buttons,
                 QMessageBox::StandardButton defaultButton);

    static QMessageBox::StandardButton
    _MSG_Critical(const QString &title, const QString &text, int buttons,
                  QMessageBox::StandardButton defaultButton);

    static void _MSG_About(const QString &title, const QString &text);

    static QMessageBox::StandardButton
    _MSG_msgbox(QMessageBox::Icon icon, const QString &title,
                const QString &text, int buttons,
                QMessageBox::StandardButton defaultButton);

private:
    static QString _InputBox_GetText(const QString &title, const QString &label,
                                     QLineEdit::EchoMode echo,
                                     const QString &text, bool *ok,
                                     int inputMethodHints);

    static QString _InputBox_GetMultiLineText(const QString &title,
                                              const QString &label,
                                              const QString &text, bool *ok,
                                              int inputMethodHints);

    static QString _InputBox_getItem(const QString &title, const QString &label,
                                     const CScriptArray &items, int current,
                                     bool editable, bool *ok,
                                     int inputMethodHints);

    static int _InputBox_GetInt(const QString &title, const QString &label,
                                int value, int minValue, int maxValue, int step,
                                bool *ok);

    static double _InputBox_GetDouble(const QString &title,
                                      const QString &label, double value,
                                      double minValue, double maxValue,
                                      int decimals, bool *ok, double step);

private:
    static QString _FileDialog_GetExistingDirectory(const QString &caption,
                                                    const QString &dir,
                                                    int options);

    static QString _FileDialog_GetOpenFileName(const QString &caption,
                                               const QString &dir,
                                               const QString &filter,
                                               QString *selectedFilter,
                                               int options);

    static CScriptArray *_FileDialog_getOpenFileNames(const QString &caption,
                                                      const QString &dir,
                                                      const QString &filter,
                                                      QString *selectedFilter,
                                                      int options);

    static QString _FileDialog_GetSaveFileName(const QString &caption,
                                               const QString &dir,
                                               const QString &filter,
                                               QString *selectedFilter,
                                               int options);

public:
    QString stringify_helper(const void *ref, int typeId);

    void exceptionCallback(asIScriptContext *context);

private:
    static void messageCallback(const asSMessageInfo *msg, void *param);

    static void translateAppException(asIScriptContext *ctx, void *userParam);

    static void cleanUpPluginSysIDFunction(asIScriptFunction *fn);

    static asIScriptContext *requestContextCallback(asIScriptEngine *engine,
                                                    void *param);
    static void lineCallback(asIScriptContext *ctx, void *param);
    static void returnContextCallback(asIScriptEngine *engine,
                                      asIScriptContext *ctx, void *param);

    static quint64 debug_elapsedTime();
    static QString debug_backtrace();

private:
    void attachDebugBreak(asIScriptContext *ctx);

private:
    asIScriptEngine *_engine = nullptr;
    asIScriptModule *_eMod = nullptr;

    QQueue<asIScriptContext *> _ctxPool;
    RegCallBacks _regcalls;
    asIScriptContext *_ctx;
    CContextMgr *_ctxMgr;
};

Q_DECLARE_METATYPE(ScriptMachine::MessageInfo)

#endif // SCRIPTMACHINE_H
