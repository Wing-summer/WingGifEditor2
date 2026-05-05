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

#include "scriptmachine.h"

#include "angelscript/sdk/add_on/scriptbuilder/scriptbuilder.h"
#include "angelscript/sdk/add_on/scriptgrid/scriptgrid.h"
#include "angelscript/sdk/add_on/scripthandle/scripthandle.h"
#include "angelscript/sdk/add_on/scriptmath/scriptmath.h"
#include "angelscript/sdk/add_on/scriptmath/scriptmathcomplex.h"
#include "angelscript/sdk/add_on/weakref/weakref.h"
#include "angelscript/sdk/angelscript/source/as_scriptengine.h"

#include "class/angelscripthelper.h"
#include "class/appmanager.h"
#include "control/toast.h"
#include "define.h"
#include "dialog/colorpickerdialog.h"
#include "wingfiledialog.h"
#include "winginputdialog.h"
#include "wingmessagebox.h"

#include "scriptaddon/scriptany.h"
#include "scriptaddon/scriptcolor.h"
#include "scriptaddon/scriptjson.h"
#include "scriptaddon/scriptoptbox.h"
#include "scriptaddon/scriptqdictionary.h"
#include "scriptaddon/scriptqstring.h"
#include "scriptaddon/scriptregex.h"
#include "scriptaddon/scriptrunable.h"

#include <QScopeGuard>
#include <QThread>

inline QString escapeNonPrintable(const QChar &ch) {
    const ushort code = ch.unicode();

    switch (code) {
    case '\\':
        return QStringLiteral("\\\\");
    case '\"':
        return QStringLiteral("\\\"");
    case '\'':
        return QStringLiteral("\\'");
    case '\0':
        return QStringLiteral("\\0");
    case '\n':
        return QStringLiteral("\\n");
    case '\r':
        return QStringLiteral("\\r");
    case '\t':
        return QStringLiteral("\\t");
    case '\b':
        return QStringLiteral("\\b");
    case '\f':
        return QStringLiteral("\\f");
    case '\v':
        return QStringLiteral("\\v");
    case '\a':
        return QStringLiteral("\\a");
    }

    if (ch.isPrint()) {
        return QString(ch);
    }

    if (code <= 0xFF) {
        return QStringLiteral("\\x%1").arg(code, 2, 16, QLatin1Char('0'));
    } else {
        return QStringLiteral("\\u%1").arg(code, 4, 16, QLatin1Char('0'));
    }
}

inline QString escapeNonPrintable(const QString &input) {
    QString out;
    out.reserve(input.size() * 2);
    for (const auto &ch : input) {
        out.append(escapeNonPrintable(ch));
    }
    return out;
}

bool ScriptMachine::init() {
    if (isInited()) {
        return true;
    }

    qRegisterMetaType<MessageInfo>();

    _engine = asCreateScriptEngine();
    if (!ScriptMachine::configureEngine()) {
        _engine->ShutDownAndRelease();
        _engine = nullptr;
        return false;
    }

    return true;
}

bool ScriptMachine::isInited() const { return _engine != nullptr; }

bool ScriptMachine::isRunning() const { return _ctx != nullptr; }

bool ScriptMachine::checkEngineConfigError() const {
    if (_engine) {
        auto e = static_cast<asCScriptEngine *>(_engine);
        if (e) {
            e->PrepareEngine();
            return e->configFailed;
        }
    }
    return true;
}

bool ScriptMachine::configureEngine() {
    if (_engine == nullptr) {
        return false;
    }

    // we need utf8, the default is what we want
    _engine->SetEngineProperty(asEP_EXPAND_DEF_ARRAY_TO_TMPL, true);
    _engine->SetEngineProperty(asEP_DISALLOW_EMPTY_LIST_ELEMENTS, true);
    _engine->SetEngineProperty(asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE, false);
    _engine->SetEngineProperty(asEP_ALLOW_MULTILINE_STRINGS, false);
    _engine->SetEngineProperty(asEP_USE_CHARACTER_LITERALS, false);
    _engine->SetEngineProperty(asEP_DISABLE_INTEGER_DIVISION, false);
    _engine->SetEngineProperty(asEP_PRIVATE_PROP_AS_PROTECTED, false);
    _engine->SetEngineProperty(asEP_ALTER_SYNTAX_NAMED_ARGS, 0);
    _engine->SetEngineProperty(asEP_ALLOW_UNICODE_IDENTIFIERS, true);
    _engine->SetEngineProperty(asEP_REQUIRE_ENUM_SCOPE,
                               true); // enum class like

    // The script compiler will send any compiler messages to the callback
    auto r = _engine->SetMessageCallback(asFUNCTION(messageCallback), this,
                                         asCALL_CDECL);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    _engine->SetTranslateAppExceptionCallback(asFUNCTION(translateAppException),
                                              this, asCALL_CDECL);

    _engine->SetFunctionUserDataCleanupCallback(
        &ScriptMachine::cleanUpPluginSysIDFunction,
        AsUserDataType::UserData_PluginFn);

    registerEngineAddon(_engine);

    _engine->SetDefaultAccessMask(0x1);

    // Register a couple of extra functions for the scripts
    r = _engine->RegisterGlobalFunction(
        "void print(const ? &in obj, const ? &in ...)", asFUNCTION(print),
        asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }
    r = _engine->RegisterGlobalFunction(
        "void printf(const string &in fmt, const ? &in ...)",
        asFUNCTION(printf), asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }
    r = _engine->RegisterGlobalFunction(
        "void println(const ? &in obj, const ? &in ...)", asFUNCTION(println),
        asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction(
        "void warnprint(const ? &in obj, const ? &in ...)",
        asFUNCTION(warnprint), asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }
    r = _engine->RegisterGlobalFunction(
        "void warnprintln(const ? &in obj, const ? &in ...)",
        asFUNCTION(warnprintln), asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction(
        "void infoprint(const ? &in obj, const ? &in ...)",
        asFUNCTION(infoprint), asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }
    r = _engine->RegisterGlobalFunction(
        "void infoprintln(const ? &in obj, const ? &in ...)",
        asFUNCTION(infoprintln), asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction(
        "void errprint(const ? &in obj, const ? &in ...)", asFUNCTION(errprint),
        asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }
    r = _engine->RegisterGlobalFunction(
        "void errprintln(const ? &in obj, const ? &in ...)",
        asFUNCTION(errprintln), asCALL_GENERIC);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction("string input()",
                                        asMETHOD(ScriptMachine, input),
                                        asCALL_THISCALL_ASGLOBAL, this);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    r = _engine->RegisterGlobalFunction("string stringify(? &in obj)",
                                        asMETHOD(ScriptMachine, stringify),
                                        asCALL_THISCALL_ASGLOBAL, this);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    CContextMgr::RegisterCoRoutineSupport(_engine);
    CContextMgr::SetGetTimeCallback([]() -> asUINT {
        return AppManager::instance()->currentMSecsSinceEpoch();
    });
    CContextMgr::RegisterThreadSupport(_engine);

    // Tell the engine to use our context pool. This will also
    // allow us to debug internal script calls made by the engine
    r = _engine->SetContextCallbacks(requestContextCallback,
                                     returnContextCallback, this);
    ASSERT(r >= 0);
    if (r < 0) {
        return false;
    }

    installAPI(this);

    return true;
}

QString ScriptMachine::getCallStack(asIScriptContext *context) {
    QString str = QStringLiteral("AngelScript callstack:\n");

    // Append the call stack
    for (asUINT i = 0; i < context->GetCallstackSize(); i++) {
        asIScriptFunction *func;
        const char *scriptSection;
        int line, column;
        func = context->GetFunction(i);
        line = context->GetLineNumber(i, &column, &scriptSection);
        str +=
            (QStringLiteral("\t") + QString::fromUtf8(scriptSection) +
             QStringLiteral(":") + QString::fromUtf8(func->GetDeclaration()) +
             QStringLiteral(":") + QString::number(line) + QStringLiteral(",") +
             QString::number(column) + QStringLiteral("\n"));
    }

    return str;
}

void ScriptMachine::destoryMachine() {
    if (_engine) {
        _engine->SetContextCallbacks(nullptr, nullptr);

        _ctxMgr->AbortAll();
        delete _ctxMgr;

        _engine->ShutDownAndRelease();
        _engine = nullptr;
        _ctxPool.clear();
    }
}

void ScriptMachine::exceptionCallback(asIScriptContext *context) {
    if (context) {
        QString message =
            QStringLiteral("- Exception '%1' in '%2'\n")
                .arg(context->GetExceptionString(),
                     context->GetExceptionFunction()->GetDeclaration()) +
            getCallStack(context);

        const char *section;
        int col;
        MessageInfo msg;
        msg.row = context->GetExceptionLineNumber(&col, &section);
        msg.col = col;
        msg.section = QString::fromUtf8(section);
        msg.type = MessageType::Error;
        msg.message = message;

        outputMessage(msg);
    }
}

void ScriptMachine::attachDebugBreak(asIScriptContext *ctx) {
    if (!ctx)
        ctx = asGetActiveContext();

    ctx->SetLineCallback(asFUNCTION(ScriptMachine::lineCallback), nullptr,
                         asCALL_CDECL);
}

void ScriptMachine::__output(MessageType type, asIScriptGeneric *args) {
    auto context = asGetActiveContext();
    if (context) {
        auto &m = ScriptMachine::instance();

        MessageInfo info;
        info.type = type;

        for (int i = 0; i < args->GetArgCount(); ++i) {
            void *ref = args->GetArgAddress(i);
            int typeId = args->GetArgTypeId(i);
            info.message.append(m.stringify(ref, typeId));
        }

        m.outputMessage(info);
    }
}

void ScriptMachine::print(asIScriptGeneric *args) {
    __output(MessageType::Print, args);
}

void ScriptMachine::__outputln(MessageType type, asIScriptGeneric *args) {
    auto context = asGetActiveContext();
    if (context) {
        auto &m = ScriptMachine::instance();

        MessageInfo info;
        info.type = type;

        for (int i = 0; i < args->GetArgCount(); ++i) {
            void *ref = args->GetArgAddress(i);
            int typeId = args->GetArgTypeId(i);

            if (typeId) {
                info.message = (m.stringify(ref, typeId).append('\n'));
                m.outputMessage(info);
            }
        }
    }
}

void ScriptMachine::println(asIScriptGeneric *args) {
    __outputln(MessageType::Print, args);
}

void ScriptMachine::warnprint(asIScriptGeneric *args) {
    __output(MessageType::Warn, args);
}

void ScriptMachine::warnprintln(asIScriptGeneric *args) {
    __outputln(MessageType::Warn, args);
}

void ScriptMachine::errprint(asIScriptGeneric *args) {
    __output(MessageType::Error, args);
}

void ScriptMachine::errprintln(asIScriptGeneric *args) {
    __outputln(MessageType::Error, args);
}

void ScriptMachine::infoprint(asIScriptGeneric *args) {
    __output(MessageType::Info, args);
}

void ScriptMachine::infoprintln(asIScriptGeneric *args) {
    __outputln(MessageType::Info, args);
}

QString ScriptMachine::input() {
    auto context = asGetActiveContext();
    if (context) {
        if (_regcalls.getInputFn) {
            return _regcalls.getInputFn();
        }
    }
    return {};
}

void ScriptMachine::outputMessage(const MessageInfo &info) {
    if (_regcalls.printMsgFn) {
        _regcalls.printMsgFn(info);
    }
}

QString ScriptMachine::stringify(void *ref, int typeId) {
    return stringify_helper(ref, typeId);
}

void ScriptMachine::_UI_Toast(const QString &message, const QString &icon) {
    Toast::toast(nullptr, QPixmap(icon), message);
}

QColor ScriptMachine::_Color_get(const QString &caption) {
    ColorPickerDialog d;
    d.setWindowTitle(caption);
    if (d.exec()) {
        return d.color();
    }
    return {};
}

void ScriptMachine::_MSG_AboutQt(const QString &title) {
    WingMessageBox::aboutQt(nullptr, title);
}

QMessageBox::StandardButton
ScriptMachine::_MSG_Information(const QString &title, const QString &text,
                                int buttons,
                                QMessageBox::StandardButton defaultButton) {
    return WingMessageBox::information(nullptr, title, text,
                                       QMessageBox::StandardButtons(buttons),
                                       defaultButton);
}

QMessageBox::StandardButton
ScriptMachine::_MSG_Question(const QString &title, const QString &text,
                             int buttons,
                             QMessageBox::StandardButton defaultButton) {
    return WingMessageBox::question(nullptr, title, text,
                                    QMessageBox::StandardButtons(buttons),
                                    defaultButton);
}

QMessageBox::StandardButton
ScriptMachine::_MSG_Warning(const QString &title, const QString &text,
                            int buttons,
                            QMessageBox::StandardButton defaultButton) {
    return WingMessageBox::warning(nullptr, title, text,
                                   QMessageBox::StandardButtons(buttons),
                                   defaultButton);
}

QMessageBox::StandardButton
ScriptMachine::_MSG_Critical(const QString &title, const QString &text,
                             int buttons,
                             QMessageBox::StandardButton defaultButton) {
    return WingMessageBox::critical(nullptr, title, text,
                                    QMessageBox::StandardButtons(buttons),
                                    defaultButton);
}

void ScriptMachine::_MSG_About(const QString &title, const QString &text) {
    return WingMessageBox::about(nullptr, title, text);
}

QMessageBox::StandardButton
ScriptMachine::_MSG_msgbox(QMessageBox::Icon icon, const QString &title,
                           const QString &text, int buttons,
                           QMessageBox::StandardButton defaultButton) {
    return WingMessageBox::msgbox(nullptr, icon, title, text,
                                  QMessageBox::StandardButtons(buttons),
                                  defaultButton);
}

QString ScriptMachine::_InputBox_GetText(const QString &title,
                                         const QString &label,
                                         QLineEdit::EchoMode echo,
                                         const QString &text, bool *ok,
                                         int inputMethodHints) {
    return WingInputDialog::getText(nullptr, title, label, echo, text, ok,
                                    Qt::InputMethodHints(inputMethodHints));
}

QString ScriptMachine::_InputBox_GetMultiLineText(const QString &title,
                                                  const QString &label,
                                                  const QString &text, bool *ok,
                                                  int inputMethodHints) {
    return WingInputDialog::getMultiLineText(
        nullptr, title, label, text, ok,
        Qt::InputMethodHints(inputMethodHints));
}

QString ScriptMachine::_InputBox_getItem(const QString &title,
                                         const QString &label,
                                         const CScriptArray &items, int current,
                                         bool editable, bool *ok,
                                         int inputMethodHints) {
    bool o = false;
    auto ret = cArray2QStringList(items, &o);
    if (o) {
        return WingInputDialog::getItem(nullptr, title, label, ret, current,
                                        editable, ok,
                                        Qt::InputMethodHints(inputMethodHints));
    } else {
        if (ok) {
            *ok = false;
        }
        return {};
    }
}

int ScriptMachine::_InputBox_GetInt(const QString &title, const QString &label,
                                    int value, int minValue, int maxValue,
                                    int step, bool *ok) {
    return WingInputDialog::getInt(nullptr, title, label, value, minValue,
                                   maxValue, step, ok);
}

double ScriptMachine::_InputBox_GetDouble(const QString &title,
                                          const QString &label, double value,
                                          double minValue, double maxValue,
                                          int decimals, bool *ok, double step) {
    return WingInputDialog::getDouble(nullptr, title, label, value, minValue,
                                      maxValue, decimals, ok, step);
}

QString ScriptMachine::_FileDialog_GetExistingDirectory(const QString &caption,
                                                        const QString &dir,
                                                        int options) {
    return WingFileDialog::getExistingDirectory(nullptr, caption, dir,
                                                QFileDialog::Options(options));
}

QString ScriptMachine::_FileDialog_GetOpenFileName(const QString &caption,
                                                   const QString &dir,
                                                   const QString &filter,
                                                   QString *selectedFilter,
                                                   int options) {
    return WingFileDialog::getOpenFileName(nullptr, caption, dir, filter,
                                           selectedFilter,
                                           QFileDialog::Options(options));
}

CScriptArray *ScriptMachine::_FileDialog_getOpenFileNames(
    const QString &caption, const QString &dir, const QString &filter,
    QString *selectedFilter, int options) {
    auto ctx = asGetActiveContext();
    if (ctx) {
        return retArrayWrapperFunction(
            [&]() -> QStringList {
                return WingFileDialog::getOpenFileNames(
                    nullptr, caption, dir, filter, selectedFilter,
                    QFileDialog::Options(options));
            },
            static_cast<asITypeInfo *>(ctx->GetEngine()->GetUserData(
                AsUserDataType::UserData_StringListTypeInfo)));
    } else {
        return nullptr;
    }
}

QString ScriptMachine::_FileDialog_GetSaveFileName(const QString &caption,
                                                   const QString &dir,
                                                   const QString &filter,
                                                   QString *selectedFilter,
                                                   int options) {
    return WingFileDialog::getSaveFileName(nullptr, caption, dir, filter,
                                           selectedFilter,
                                           QFileDialog::Options(options));
}

QString ScriptMachine::getAsTypeName(int typeId) {
    return getAsTypeNameById(_engine, typeId);
}

QString ScriptMachine::stringify_helper(const void *ref, int typeId) {
    ASSERT(ref && typeId);
    if (ref == nullptr) {
        return {};
    }

    switch (typeId & asTYPEID_MASK_SEQNBR) {
    case asTYPEID_BOOL:
        return *static_cast<const bool *>(ref) ? QStringLiteral("true")
                                               : QStringLiteral("false");
    case asTYPEID_INT8:
        return QString::number(*static_cast<const qint8 *>(ref));
    case asTYPEID_INT16:
        return QString::number(*static_cast<const qint16 *>(ref));
    case asTYPEID_INT32:
        return QString::number(*static_cast<const qint32 *>(ref));
    case asTYPEID_INT64:
        return QString::number(*static_cast<const qint64 *>(ref));
    case asTYPEID_UINT8:
        return QString::number(*static_cast<const quint8 *>(ref));
    case asTYPEID_UINT16:
        return QString::number(*static_cast<const quint16 *>(ref));
    case asTYPEID_UINT32:
        return QString::number(*static_cast<const quint32 *>(ref));
    case asTYPEID_UINT64:
        return QString::number(*static_cast<const quint64 *>(ref));
    case asTYPEID_FLOAT:
        return QString::number(*static_cast<const float *>(ref));
    case asTYPEID_DOUBLE:
        return QString::number(*static_cast<const double *>(ref));
    }

    if (isAngelChar(typeId)) {
        // char
        auto pch = resolveObjAs<QChar>(ref, typeId);
        return *pch;
    } else if (isAngelString(typeId)) {
        // string
        return *resolveObjAs<QString>(ref, typeId);
    } else if (isAngelArray(typeId)) {
        // array<?>
        auto arr = resolveObjAs<CScriptArray>(ref, typeId);
        auto buffer = QStringLiteral("[");
        if (!arr->IsEmpty()) {
            auto type = arr->GetElementTypeId();
            auto data = arr->At(0);
            buffer.append(stringify_helper(data, type));

            auto total = arr->GetSize();
            for (asUINT i = 1; i < total; ++i) {
                data = arr->At(i);
                buffer.append(QLatin1String(", "))
                    .append(stringify_helper(data, type));
            }
        }
        buffer.append(']');
        return buffer;
    } else if (isAngelDictionary(typeId)) {
        // dictionary
        auto dic = resolveObjAs<CScriptDictionary>(ref, typeId);
        auto buffer = QStringLiteral("{");

        if (!dic->IsEmpty()) {
            for (auto p = dic->begin(); p != dic->end(); p++) {
                if (p != dic->begin()) {
                    buffer.append(QLatin1String(", "));
                }

                auto key = p.GetKey();
                auto type = p.GetTypeId();
                auto data = p.GetAddressOfValue();

                buffer.append('{').append('"').append(key).append('"').append(
                    '=');
                if (isAngelChar(type)) {
                    buffer.append('\'')
                        .append(escapeNonPrintable(
                            *resolveObjAs<QChar>(ref, typeId)))
                        .append('\'');
                } else if (isAngelString(type)) {
                    buffer.append('"')
                        .append(escapeNonPrintable(
                            *resolveObjAs<QString>(ref, typeId)))
                        .append('"');
                } else {
                    buffer.append(stringify_helper(ref, typeId));
                }
                buffer.append('}');
            }
        }

        buffer.append('}');
        return buffer;
    } else if (isAngelDicValue(typeId)) {
        // dictonaryValue
        auto dicv = resolveObjAs<CScriptDictValue>(ref, typeId);
        return stringify_helper(const_cast<void *>(dicv->GetAddressOfValue()),
                                dicv->GetTypeId());
    } else if (isAngelAny(typeId)) {
        // any
        auto obj = resolveObjAs<CScriptAny>(ref, typeId);
        return stringify_helper(const_cast<void *>(obj->GetAddressOfValue()),
                                obj->GetTypeId());
    }

    // if type has toString() function
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        auto info = _engine->GetTypeInfoById(typeId);
        if (info) {
            auto func = info->GetMethodByDecl("string toString() const");
            if (func) {
                ctx->PushState();
                ctx->Prepare(func);
                ctx->SetObject(
                    const_cast<void *>(resolveObjAs<void>(ref, typeId)));
                ctx->Execute();
                auto rstr = *static_cast<QString *>(ctx->GetReturnObject());
                ctx->PopState();
                return rstr;
            }
        }
    }

    return QStringLiteral("%1<#%2>")
        .arg(getAsTypeName(typeId))
        .arg(quintptr(ref), 0, 16);
}

bool ScriptMachine::executeScript(const QString &script,
                                  const std::function<void(bool)> &onFinished) {
    Q_ASSERT(onFinished);
    if (_engine == nullptr) {
        MessageInfo info;
        info.section = script;
        info.message = QStringLiteral("The script engine is not initialized");
        info.type = MessageType::Error;
        outputMessage(info);
        onFinished(true);
        return false;
    }

    if (QThread::currentThread() != qApp->thread()) {
        MessageInfo info;
        info.section = script;
        info.message = QStringLiteral("Code must be exec in the main thread");
        info.type = MessageType::Warn;
        outputMessage(info);
        onFinished(true);
        return false;
    }

    if (script.isEmpty()) {
        onFinished(true);
        return true;
    }

    if (isRunning()) {
        onFinished(false);
        return false;
    }

    // TODO include and pragma callback set
    CScriptBuilder builder;
    auto r = builder.StartNewModule(_engine, "WINGSCRIPT");
    if (r < 0) {
        onFinished(true);
        return false;
    }

    // Compile the script
    r = builder.AddSectionFromFile(script.toUtf8());
    if (r < 0) {
        MessageInfo info;
        info.section = script;
        info.message = QStringLiteral("Script failed to pre-processed");
        info.type = MessageType::Error;
        outputMessage(info);
        onFinished(true);
        return false;
    }

    r = builder.BuildModule();
    if (r < 0) {
        MessageInfo info;
        info.section = script;
        info.message = QStringLiteral("Script failed to build");
        info.type = MessageType::Error;
        outputMessage(info);
        onFinished(true);
        return false;
    }

    auto mod = builder.GetModule();

    // Find the main function
    asIScriptFunction *func = mod->GetFunctionByDecl("int main()");
    if (func == nullptr) {
        // Try again with "void main()"
        func = mod->GetFunctionByDecl("void main()");
    }

    if (func == nullptr) {
        MessageInfo info;
        info.section = script;
        info.message =
            QStringLiteral("Cannot find 'int main()' or 'void main()'");
        info.type = MessageType::Error;
        outputMessage(info);
        onFinished(true);
        return false;
    }

    // Set up a context to execute the script
    // The context manager will request the context from the
    // pool, which will automatically attach the debugger
    auto ctxMgr = new CContextMgr;
    asIScriptContext *ctx = ctxMgr->SetMainFunction(_engine, func, true);
    if (ctx == nullptr) {
        MessageInfo info;
        info.section = script;
        info.message = QStringLiteral("Cannot prepare context for execution.");
        info.type = MessageType::Error;
        outputMessage(info);
        delete ctxMgr;
        onFinished(true);
        return false;
    }

    ctxMgr->setScriptName(script);
    ctx->SetUserData(reinterpret_cast<void *>(
                         AppManager::instance()->currentMSecsSinceEpoch()),
                     AsUserDataType::UserData_Timer);

    ctx->SetExceptionCallback(asMETHOD(ScriptMachine, exceptionCallback), this,
                              asCALL_THISCALL);

    _ctx = ctx;
    _ctxMgr = ctxMgr;

    auto runner = new ScriptRunable(ctxMgr);
    QObject::connect(
        runner, &QObject::destroyed, runner,
        [this, ctx, script, func, mod, onFinished]() {
            _ctx = nullptr;

            // Check if the main script finished normally
            int r = ctx->GetState();
            if (r != asEXECUTION_FINISHED) {
                if (r == asEXECUTION_EXCEPTION) {
                    r = -1;
                } else if (r == asEXECUTION_ABORTED) {
                    MessageInfo info;
                    info.section = script;
                    info.message = QStringLiteral("The script was aborted");
                    info.type = MessageType::Error;
                    outputMessage(info);
                    r = -1;
                } else {
                    auto e = QMetaEnum::fromType<asEContextState>();
                    MessageInfo info;
                    info.section = script;
                    info.message =
                        QStringLiteral("The script terminated unexpectedly (") +
                        QString::fromLatin1(e.valueToKey(r)) +
                        QStringLiteral(")");
                    info.type = MessageType::Error;
                    outputMessage(info);
                    r = -1;
                }
            } else {
                // Get the return value from the script
                if (func->GetReturnTypeId() == asTYPEID_INT32) {
                    r = *(int *)ctx->GetAddressOfReturnValue();
                } else
                    r = 0;
            }

            MessageInfo info;
            info.section = script;
            info.message =
                QStringLiteral("The script exited with ") + QString::number(r);
            info.type = MessageType::ExecInfo;
            outputMessage(info);

            // Return the context after retrieving the return value
            _ctxMgr->DoneWithContext(ctx);
            _ctxMgr = nullptr;

            // Before leaving, allow the engine to clean up remaining objects by
            // discarding the module and doing a full garbage collection so that
            // this can also be debugged if desired
            mod->Discard();

            onFinished(true);
        });
    runner->start();
    return true;
}

void ScriptMachine::abortScript() {
    if (_ctxMgr) {
        _ctxMgr->AbortAll();
    }
}

void ScriptMachine::messageCallback(const asSMessageInfo *msg, void *param) {
    auto ins = static_cast<ScriptMachine *>(param);

    MessageType t = MessageType::Print;
    switch (msg->type) {
    case asMSGTYPE_ERROR:
        t = MessageType::Error;
        break;
    case asMSGTYPE_WARNING:
        t = MessageType::Warn;
        break;
    case asMSGTYPE_INFORMATION:
        t = MessageType::Info;
        break;
    }

    MessageInfo info;
    info.row = msg->row;
    info.col = msg->col;
    info.section = QString::fromUtf8(msg->section);
    info.message = QString::fromUtf8(msg->message);
    info.type = t;
    ins->outputMessage(info);
}

void ScriptMachine::translateAppException(asIScriptContext *ctx,
                                          void *userParam) {
    Q_UNUSED(userParam);
    try {
        // Retrow the original exception so we can catch it again
        throw;
    } catch (const std::exception &e) {
        // Tell the VM the type of exception that occurred
        ctx->SetException(e.what());
    } catch (...) {
        // The callback must not allow any exception to be thrown, but it is not
        // necessary to explicitly set an exception string if the default
        // exception string is sufficient
    }
}

void ScriptMachine::cleanUpPluginSysIDFunction(asIScriptFunction *) {
    // do nothing
    // UserData_API is readonly and it will delete later by its allocator
    // UserData_PluginFn is just an id, not a valid pointer to data
}

ScriptMachine &ScriptMachine::instance() {
    static ScriptMachine ins;
    return ins;
}

ScriptMachine::ScriptMachine() {}

asIScriptContext *ScriptMachine::requestContextCallback(asIScriptEngine *engine,
                                                        void *param) {
    asIScriptContext *ctx = nullptr;
    auto p = reinterpret_cast<ScriptMachine *>(param);
    ASSERT(p);

    // Check if there is a free context available in the pool
    if (p->_ctxPool.isEmpty()) {
        // No free context was available so we'll have to create a new one
        ctx = engine->CreateContext();
    } else {
        ctx = p->_ctxPool.dequeue();
    }

    // Attach the debugger
    ctx->SetLineCallback(asFUNCTION(ScriptMachine::lineCallback), nullptr,
                         asCALL_CDECL);

    return ctx;
}

void ScriptMachine::lineCallback(asIScriptContext *ctx, void *) {
    if (ctx->GetUserData(AsUserDataType::UserData_NeedYeild)) {
        // return the control
        ctx->Suspend();
    }
}

void ScriptMachine::returnContextCallback(asIScriptEngine *engine,
                                          asIScriptContext *ctx, void *param) {
    Q_UNUSED(engine);

    if (ctx) {
        if (ctx->GetState() == asEXECUTION_SUSPENDED) {
            ctx->Abort();
        }

        // Unprepare the context to free any objects it may still hold (e.g.
        // return value) This must be done before making the context available
        // for re-use, as the clean up may trigger other script executions, e.g.
        // if a destructor needs to call a function.
        if (ctx->Unprepare() < 0) {
            ctx->Release();
            return;
        }

        // reset userdata
        for (int i = AsUserDataType::UserData_CopyAttr_Begin;
             i < AsUserDataType::UserData_CopyAttr_End; ++i) {
            ctx->SetUserData(0, i);
        }

        auto p = reinterpret_cast<ScriptMachine *>(param);
        ASSERT(p);

        // Place the context into the pool for when it will be needed again
        p->_ctxPool.enqueue(ctx);
    }
}

quint64 ScriptMachine::debug_elapsedTime() {
    auto ctx = asGetActiveContext();
    if (ctx) {
        return AppManager::instance()->currentMSecsSinceEpoch() -
               reinterpret_cast<quint64>(
                   ctx->GetUserData(AsUserDataType::UserData_Timer));
    }
    return 0;
}

QString ScriptMachine::debug_backtrace() {
    auto ctx = asGetActiveContext();
    if (ctx) {
        auto &m = ScriptMachine::instance();
        auto cs = ctx->GetCallstackSize();

        QString ret;
        for (asUINT i = 0; i < cs; i++) {
            auto f = ctx->GetFunction(i);
            int col;
            const char *section;
            int row = ctx->GetLineNumber(i, &col, &section);
            ret.append(QStringLiteral("%1 %2[%3:%4]\n")
                           .arg(QString::fromUtf8(
                                    f->GetDeclaration(true, false, true)),
                                QString::fromUtf8(section))
                           .arg(row)
                           .arg(col));
        }

        return ret;
    }

    return {};
}

void ScriptMachine::registerEngineAddon(asIScriptEngine *engine) {
    // all modules can access
    engine->SetDefaultAccessMask(0x3);

    auto r = engine->RegisterTypedef("byte", "uint8"); // register alias
    ASSERT(r >= 0);

    RegisterScriptArray(engine, true);
    RegisterQString(engine);
    RegisterScriptRegex(engine);
    RegisterQStringUtils(engine);
    RegisterQStringRegExSupport(engine);

    r = engine->SetDefaultNamespace("math");
    ASSERT(r >= 0);

    RegisterScriptMath(engine);
    RegisterScriptMathComplex(engine);
    engine->SetDefaultNamespace("");

    RegisterScriptWeakRef(engine);
    RegisterScriptAny(engine);
    RegisterScriptDictionary(engine);
    RegisterScriptGrid(engine);
    RegisterScriptHandle(engine);
    RegisterColor(engine);
    RegisterQJson(engine);
    RegisterOptBox(engine);

    engine->SetDefaultAccessMask(0x1);
    registerExceptionRoutines(engine);
    registerEngineAssert(engine);
    registerEngineDebug(engine);

    // cache typeids and typeinfos we all must use
    auto type = engine->GetTypeInfoByName("char");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_CharTypeInfo);

    type = engine->GetTypeInfoByName("string");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_StringTypeInfo);

    type = engine->GetTypeInfoByName("array");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_ArrayTypeInfo);

    type = engine->GetTypeInfoByName("dictionary");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_DictionaryTypeInfo);

    type = engine->GetTypeInfoByName("dictionaryValue");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_DictionaryValueTypeInfo);

    type = engine->GetTypeInfoByName("any");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_AnyTypeInfo);

    type = engine->GetTypeInfoByName("json::value");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_JsonValueTypeInfo);

    type = engine->GetTypeInfoByDecl("array<byte>");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_ByteArrayTypeInfo);

    type = engine->GetTypeInfoByDecl("array<string>");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_StringListTypeInfo);

    type = engine->GetTypeInfoByDecl("array<char>");
    ASSERT(type);
    engine->SetUserData(type, AsUserDataType::UserData_CharArrayTypeInfo);
}

void ScriptMachine::registerEngineAssert(asIScriptEngine *engine) {
    int r;

    // The string type must be available
    ASSERT(engine->GetTypeInfoByDecl("string"));

    r = engine->RegisterGlobalFunction("void assert(bool expression)",
                                       asFUNCTION(scriptAssert), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "void assert_x(bool expression, const string &in msg)",
        asFUNCTION(scriptAssert_X), asCALL_CDECL);
    ASSERT(r >= 0);
}

void ScriptMachine::registerEngineDebug(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("debug");
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "uint64 elapsedTime()", asFUNCTION(debug_elapsedTime), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "string backtrace()", asFUNCTION(debug_backtrace), asCALL_CDECL);
    ASSERT(r >= 0);

    engine->SetDefaultNamespace("");
}

void ScriptMachine::installAPI(ScriptMachine *machine) {
    ASSERT(machine);
    auto engine = machine->engine();

    installBasicTypes(engine);
    installExtAPI(engine);
    installMsgboxAPI(engine);
    installInputboxAPI(engine);
    installFileDialogAPI(engine);
    installColorDialogAPI(engine);

    installGifEditAPI(engine);
}

void ScriptMachine::installBasicTypes(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("msgbox");
    ASSERT(r >= 0);

    registerAngelType<QMessageBox::StandardButtons>(engine, "buttons");
    registerAngelType<QMessageBox::Icon>(engine, "icon");

    r = engine->SetDefaultNamespace("inputbox");
    ASSERT(r >= 0);

    registerAngelType<QLineEdit::EchoMode>(engine, "EchoMode");
    registerAngelType<Qt::InputMethodHints>(engine, "InputMethodHints");

    r = engine->SetDefaultNamespace("filedlg");
    ASSERT(r >= 0);

    registerAngelType<QFileDialog::Options>(engine, "options");

    engine->SetDefaultNamespace("");
}

void ScriptMachine::installExtAPI(asIScriptEngine *engine) {
    // toast(message, iconPath)
    int r = engine->RegisterGlobalFunction(
        "void toast(const string &in message, const string &in "
        "icon =\"\")",
        asFUNCTION(_UI_Toast), asCALL_CDECL);
    ASSERT(r >= 0);
}

void ScriptMachine::installMsgboxAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("msgbox");
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "void aboutQt(const string &in title =\"\")", asFUNCTION(_MSG_AboutQt),
        asCALL_CDECL);
    ASSERT(r >= 0);
    r = engine->RegisterGlobalFunction(
        "void information(const string &in title, const string &in text, "
        "int buttons = msgbox::buttons::Ok, "
        "msgbox::buttons defaultButton = msgbox::buttons::NoButton)",
        asFUNCTION(_MSG_Information), asCALL_CDECL);
    ASSERT(r >= 0);
    r = engine->RegisterGlobalFunction(
        "void question(const string &in title, const string &in text, "
        "int buttons = msgbox::buttons::Yes | msgbox::buttons::No, "
        "msgbox::buttons defaultButton = msgbox::buttons::NoButton)",
        asFUNCTION(_MSG_Question), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "void warning(const string &in title, const string &in text, "
        "int buttons = msgbox::buttons::Ok, "
        "msgbox::buttons defaultButton = msgbox::buttons::NoButton)",
        asFUNCTION(_MSG_Warning), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "void critical(const string &in title, const string &in text, "
        "int buttons = msgbox::buttons::Ok, "
        "msgbox::buttons defaultButton = msgbox::buttons::NoButton)",
        asFUNCTION(_MSG_Critical), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "void msgbox(msgbox::icon icon, const string &in title, "
        "const string &in text, "
        "int buttons = msgbox::buttons::NoButton, "
        "msgbox::buttons defaultButton = msgbox::buttons::NoButton)",
        asFUNCTION(_MSG_msgbox), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "void about(const string &in title, const string &in text)",
        asFUNCTION(_MSG_About), asCALL_CDECL);
    ASSERT(r >= 0);

    engine->SetDefaultNamespace("");
}

void ScriptMachine::installInputboxAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("inputbox");
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "string getText(const string &in title, const string &in label, "
        "inputbox::EchoMode echo = inputbox::EchoMode::Normal, "
        "const string &in text = \"\", bool &out ok = void, "
        "int inputMethodHints = inputbox::InputMethodHints::ImhNone)",
        asFUNCTION(_InputBox_GetText), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "string getMultiLineText(const string &in title, "
        "const string &in label, "
        "const string &in text = \"\", bool &out ok = void, "
        "int inputMethodHints = inputbox::InputMethodHints::ImhNone)",
        asFUNCTION(_InputBox_GetMultiLineText), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "int getInt(const string &in title, const string &in label, "
        "int &in value = 0, int &in minValue = -2147483647, "
        "int &in maxValue = 2147483647, "
        "int &in step = 1, bool &out ok = void)",
        asFUNCTION(_InputBox_GetInt), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "double getDouble(const string &in title, const string &in label, "
        "double &in value = 0, double &in minValue = -2147483647, "
        "double &in maxValue = 2147483647, int &in decimals = 1, "
        "bool &out ok = void, double &in step = 1)",
        asFUNCTION(_InputBox_GetDouble), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "string getItem(const string &in title, const string &in label, "
        "const string[] &in items, int current = 0, "
        "bool editable = true, bool &out ok = void, "
        "int inputMethodHints = inputbox::InputMethodHints::ImhNone)",
        asFUNCTION(_InputBox_getItem), asCALL_CDECL);
    ASSERT(r >= 0);

    engine->SetDefaultNamespace("");
}

void ScriptMachine::installFileDialogAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("filedlg");
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "string getExistingDirectory(const string &in caption = \"\", "
        "const string &in dir = \"\", "
        "int options = filedlg::options::ShowDirsOnly)",
        asFUNCTION(_FileDialog_GetExistingDirectory), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "string getOpenFileName(const string &in caption = \"\", "
        "const string &in dir = \"\", const string &in filter = \"\", "
        "string &out selectedFilter = void, int options = 0)",
        asFUNCTION(_FileDialog_GetOpenFileName), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "string getSaveFileName(const string &in caption = \"\", "
        "const string &in dir = \"\", const string &in filter = \"\", "
        "string &out selectedFilter = void, int options = 0)",
        asFUNCTION(_FileDialog_GetSaveFileName), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "string[]@ getOpenFileNames(const string &in caption = \"\", "
        "const string &in dir = \"\", const string &in filter = \"\", "
        "string &out selectedFilter = void, int options = 0)",
        asFUNCTION(_FileDialog_getOpenFileNames), asCALL_CDECL);
    ASSERT(r >= 0);

    engine->SetDefaultNamespace("");
}

void ScriptMachine::installColorDialogAPI(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("colordlg");
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "color getColor(const string &in caption)", asFUNCTION(_Color_get),
        asCALL_CDECL);
    ASSERT(r >= 0);

    engine->SetDefaultNamespace("");
}

void ScriptMachine::installGifEditAPI(asIScriptEngine *engine) {
    // TODO

    // gifframe object

    // imgshow function
}

void ScriptMachine::registerCallBack(const RegCallBacks &callbacks) {
    _regcalls = callbacks;
}

bool ScriptMachine::isAngelChar(int typeID) const {
    auto type = static_cast<asITypeInfo *>(
        _engine->GetUserData(AsUserDataType::UserData_CharTypeInfo));
    return typeID == type->GetTypeId();
}

bool ScriptMachine::isAngelString(int typeID) const {
    auto type = static_cast<asITypeInfo *>(
        _engine->GetUserData(AsUserDataType::UserData_StringTypeInfo));
    return typeID == type->GetTypeId();
}

bool ScriptMachine::isAngelArray(int typeID) const {
    if (typeID & asTYPEID_TEMPLATE) {
        auto t = _engine->GetTypeInfoById(typeID);
        t = _engine->GetTypeInfoByName(t->GetName());
        ASSERT(t);
        typeID = t->GetTypeId();
    }
    auto type = static_cast<asITypeInfo *>(
        _engine->GetUserData(AsUserDataType::UserData_ArrayTypeInfo));
    return typeID == type->GetTypeId();
}

bool ScriptMachine::isAngelDictionary(int typeID) const {
    auto type = static_cast<asITypeInfo *>(
        _engine->GetUserData(AsUserDataType::UserData_DictionaryTypeInfo));
    return typeID == type->GetTypeId();
}

bool ScriptMachine::isAngelDicValue(int typeID) const {
    auto type = static_cast<asITypeInfo *>(
        _engine->GetUserData(AsUserDataType::UserData_DictionaryValueTypeInfo));
    return typeID == type->GetTypeId();
}

bool ScriptMachine::isAngelAny(int typeID) const {
    auto type = static_cast<asITypeInfo *>(
        _engine->GetUserData(AsUserDataType::UserData_AnyTypeInfo));
    return typeID == type->GetTypeId();
}

void ScriptMachine::scriptAssert(bool b) {
    auto ctx = asGetActiveContext();
    if (ctx) {
        if (!b) {
            ctx->SetException("Assert failed", false);
        }
    }
}

void ScriptMachine::scriptAssert_X(bool b, const QString &msg) {
    auto ctx = asGetActiveContext();
    if (ctx) {
        if (!b) {
            auto m = msg;
            if (m.isEmpty()) {
                m = QStringLiteral("Assert failed");
            }
            ctx->SetException(m.toUtf8(), false);
        }
    }
}

void ScriptMachine::scriptThrow(const QString &msg) {
    asIScriptContext *ctx = asGetActiveContext();
    if (ctx) {
        ctx->SetException(msg.toUtf8());
    }
}

asIScriptEngine *ScriptMachine::engine() const { return _engine; }

QString ScriptMachine::scriptGetExceptionInfo() {
    asIScriptContext *ctx = asGetActiveContext();
    if (!ctx)
        return {};

    const char *msg = ctx->GetExceptionString();
    if (msg == 0)
        return {};

    return QString::fromUtf8(msg);
}

void ScriptMachine::registerExceptionRoutines(asIScriptEngine *engine) {
    int r;

    // The string type must be available
    ASSERT(engine->GetTypeInfoByDecl("string"));

    r = engine->RegisterGlobalFunction("void throw(const string &in)",
                                       asFUNCTION(scriptThrow), asCALL_CDECL);
    ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction("string getExceptionInfo()",
                                       asFUNCTION(scriptGetExceptionInfo),
                                       asCALL_CDECL);
    ASSERT(r >= 0);
}
