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

#include "qtgeometrybinding.h"
#include "class/angelscripthelper.h"
#include "define.h"

#include <QGlyphRun>
#include <QLine>
#include <QPainterPath>
#include <QPoint>
#include <QPointF>
#include <QPolygon>
#include <QRect>
#include <QRectF>
#include <QtGlobal>

// -----------------------------
// value type ctor / dtor helpers
// -----------------------------
#define DECLARE_VALUE_TYPE_WRAPPERS(Type)                                      \
    static void Type##_Ctor(void *mem) { new (mem) Type(); }                   \
    static void Type##_CopyCtor(const Type &other, void *mem) {                \
        new (mem) Type(other);                                                 \
    }                                                                          \
    static void Type##_Dtor(void *mem) {                                       \
        reinterpret_cast<Type *>(mem)->~Type();                                \
    }                                                                          \
    static bool Type##_equals(const Type &a, const Type &b) { return a == b; }

DECLARE_VALUE_TYPE_WRAPPERS(QPoint)
DECLARE_VALUE_TYPE_WRAPPERS(QMargins)
DECLARE_VALUE_TYPE_WRAPPERS(QSize)
DECLARE_VALUE_TYPE_WRAPPERS(QRect)
DECLARE_VALUE_TYPE_WRAPPERS(QLine)
DECLARE_VALUE_TYPE_WRAPPERS(QPolygon)
DECLARE_VALUE_TYPE_WRAPPERS(QGlyphRun)
DECLARE_VALUE_TYPE_WRAPPERS(QPainterPath)

// -----------------------------
// small wrapper helpers
// -----------------------------

// QPoint arithmetic helpers
static QPoint QPoint_add(const QPoint &a, const QPoint &b) { return a + b; }
static QPoint QPoint_sub(const QPoint &a, const QPoint &b) { return a - b; }

static QPoint QPoint_mul_int(const QPoint &p, int factor) { return p * factor; }
static QPoint QPoint_mul_int_rev(int factor, const QPoint &p) {
    return factor * p;
}

static QPoint QPoint_mul_double(const QPoint &p, double factor) {
    return p * factor;
}
static QPoint QPoint_mul_double_rev(double factor, const QPoint &p) {
    return factor * p;
}

static QPoint QPoint_mul_float(const QPoint &p, float factor) {
    return p * factor;
}
static QPoint QPoint_mul_float_rev(float factor, const QPoint &p) {
    return factor * p;
}

static QPoint QPoint_div(const QPoint &p, double divisor) {
    return p / divisor;
}

static QPoint QPoint_neg(const QPoint &p) { return -p; }

static QPoint &QPoint_addAssign(QPoint &self, const QPoint &p) {
    return self += p;
}

static QPoint &QPoint_subAssign(QPoint &self, const QPoint &p) {
    return self -= p;
}

static QPoint &QPoint_mulAssignInt(QPoint &self, int factor) {
    return self *= factor;
}

static QPoint &QPoint_mulAssignFloat(QPoint &self, float factor) {
    return self *= factor;
}

static QPoint &QPoint_mulAssignDouble(QPoint &self, double factor) {
    return self *= factor;
}

static QPoint &QPoint_divAssign(QPoint &self, double divisor) {
    return self /= divisor;
}

static int QPoint_dotProduct(const QPoint &p1, const QPoint &p2) {
    return QPoint::dotProduct(p1, p2);
}

static void QMargins_Ctor4(int left, int top, int right, int bottom,
                           QMargins *self) {
    new (self) QMargins(left, top, right, bottom);
}

static bool QMargins_Equal(const QMargins &lhs, const QMargins &rhs) {
    return lhs == rhs;
}

static QMargins QMargins_AddMM(const QMargins &lhs, const QMargins &rhs) {
    return lhs + rhs;
}

static QMargins QMargins_AddMI(const QMargins &lhs, int rhs) {
    return lhs + rhs;
}

static QMargins QMargins_AddIM(int lhs, const QMargins &rhs) {
    return lhs + rhs;
}

static QMargins QMargins_SubMM(const QMargins &lhs, const QMargins &rhs) {
    return lhs - rhs;
}

static QMargins QMargins_SubMI(const QMargins &lhs, int rhs) {
    return lhs - rhs;
}

static QMargins QMargins_MulMI(const QMargins &lhs, int factor) {
    return lhs * factor;
}

static QMargins QMargins_MulIM(int factor, const QMargins &rhs) {
    return factor * rhs;
}

static QMargins QMargins_MulMD(const QMargins &lhs, double factor) {
    return lhs * factor;
}

static QMargins QMargins_MulDM(double factor, const QMargins &rhs) {
    return factor * rhs;
}

static QMargins QMargins_DivMI(const QMargins &lhs, int divisor) {
    return lhs / divisor;
}

static QMargins QMargins_DivMD(const QMargins &lhs, double divisor) {
    return lhs / divisor;
}

static QMargins QMargins_Neg(const QMargins &m) { return -m; }

static QMargins &QMargins_AddAssignMM(const QMargins &rhs, QMargins *self) {
    *self += rhs;
    return *self;
}

static QMargins &QMargins_AddAssignMI(int rhs, QMargins *self) {
    *self += rhs;
    return *self;
}

static QMargins &QMargins_SubAssignMM(const QMargins &rhs, QMargins *self) {
    *self -= rhs;
    return *self;
}

static QMargins &QMargins_SubAssignMI(int rhs, QMargins *self) {
    *self -= rhs;
    return *self;
}

static QMargins &QMargins_MulAssignMI(int rhs, QMargins *self) {
    *self *= rhs;
    return *self;
}

static QMargins &QMargins_MulAssignMD(double rhs, QMargins *self) {
    *self *= rhs;
    return *self;
}

static QMargins &QMargins_DivAssignMI(int rhs, QMargins *self) {
    *self /= rhs;
    return *self;
}

static QMargins &QMargins_DivAssignMD(double rhs, QMargins *self) {
    *self /= rhs;
    return *self;
}

static QMargins QMargins_Or(const QMargins &lhs, const QMargins &rhs) {
    return lhs | rhs;
}

static void QPolygon_CtorFromArray(const CScriptArray *arr, QPolygon *self) {
    const auto n = arr->GetSize();
    new (self) QPolygon(n);
    for (asUINT i = 0; i < n; ++i) {
        const QPoint *p = static_cast<const QPoint *>(arr->At(i));
        self->setPoint(i, *p);
    }
}

static void QPolygon_insertPoint(QPolygon &poly, int index, const QPoint &pt) {
    poly.insert(index, pt);
}
static void QPolygon_removePointAt(QPolygon &poly, int index) {
    poly.removeAt(index);
}

static void QPolygon_AddPoints(const CScriptArray *arr, QPolygon *self) {
    const asUINT n = arr->GetSize();
    self->reserve(self->size() + int(n));

    for (asUINT i = 0; i < n; ++i) {
        const QPoint *p = static_cast<const QPoint *>(arr->At(i));
        self->append(p ? *p : QPoint());
    }
}

static void QPolygon_InsertPoint(int index, const QPoint &pt, QPolygon *self) {
    if (index < 0)
        index = 0;
    if (index > self->size())
        index = self->size();

    self->insert(index, pt);
}

static void QPolygon_InsertPoints(int index, const CScriptArray *arr,
                                  QPolygon *self) {
    if (!arr)
        return;

    if (index < 0)
        index = 0;
    if (index > self->size())
        index = self->size();

    const asUINT n = arr->GetSize();
    self->reserve(self->size() + int(n));

    for (asUINT i = 0; i < n; ++i) {
        const QPoint *p = static_cast<const QPoint *>(arr->At(i));
        self->insert(index + int(i), p ? *p : QPoint());
    }
}

// ---- QSize ----
static QSize QSize_Add(const QSize &lhs, const QSize &rhs) { return lhs + rhs; }

static QSize QSize_Sub(const QSize &lhs, const QSize &rhs) { return lhs - rhs; }

static QSize QSize_Mul(const QSize &lhs, qreal factor) { return lhs * factor; }

static QSize QSize_Mul_r(qreal factor, const QSize &rhs) {
    return factor * rhs;
}
static QSize QSize_Div(const QSize &lhs, qreal divisor) {
    return lhs / divisor;
}

static void QPainterPath_clear(QPainterPath &p) { p.clear(); }
static bool QPainterPath_isEmpty(const QPainterPath &p) { return p.isEmpty(); }
static QPoint QPainterPath_currentPosition(const QPainterPath &p) {
    return p.currentPosition().toPoint();
}
static QRect QPainterPath_boundingRect(const QPainterPath &p) {
    return p.boundingRect().toRect();
}
static QRect QPainterPath_controlPointRect(const QPainterPath &p) {
    return p.controlPointRect().toRect();
}
static Qt::FillRule QPainterPath_fillRule(const QPainterPath &p) {
    return p.fillRule();
}
static void QPainterPath_setFillRule(QPainterPath &p, Qt::FillRule rule) {
    p.setFillRule(rule);
}
static void QPainterPath_moveTo1(QPainterPath &p, int x, int y) {
    p.moveTo(x, y);
}
static void QPainterPath_moveTo2(QPainterPath &p, const QPoint &pt) {
    p.moveTo(pt);
}
static void QPainterPath_lineTo1(QPainterPath &p, int x, int y) {
    p.lineTo(x, y);
}
static void QPainterPath_lineTo2(QPainterPath &p, const QPoint &pt) {
    p.lineTo(pt);
}
static void QPainterPath_quadTo1(QPainterPath &p, int cx, int cy, int ex,
                                 int ey) {
    p.quadTo(cx, cy, ex, ey);
}
static void QPainterPath_quadTo2(QPainterPath &p, const QPoint &c,
                                 const QPoint &e) {
    p.quadTo(c, e);
}
static void QPainterPath_cubicTo1(QPainterPath &p, int c1x, int c1y, int c2x,
                                  int c2y, int ex, int ey) {
    p.cubicTo(c1x, c1y, c2x, c2y, ex, ey);
}
static void QPainterPath_cubicTo2(QPainterPath &p, const QPoint &c1,
                                  const QPoint &c2, const QPoint &e) {
    p.cubicTo(c1, c2, e);
}
static void QPainterPath_closeSubpath(QPainterPath &p) { p.closeSubpath(); }
static void QPainterPath_addPath(QPainterPath &p, const QPainterPath &other) {
    p.addPath(other);
}
static void QPainterPath_connectPath(QPainterPath &p,
                                     const QPainterPath &other) {
    p.connectPath(other);
}
static void QPainterPath_addRect1(QPainterPath &p, int x, int y, int w, int h) {
    p.addRect(x, y, w, h);
}
static void QPainterPath_addRect2(QPainterPath &p, const QRect &r) {
    p.addRect(r);
}
static void QPainterPath_addEllipse1(QPainterPath &p, int x, int y, int w,
                                     int h) {
    p.addEllipse(x, y, w, h);
}
static void QPainterPath_addEllipse2(QPainterPath &p, const QPoint &c, int rx,
                                     int ry) {
    p.addEllipse(c, rx, ry);
}
static void QPainterPath_translate1(QPainterPath &p, int dx, int dy) {
    p.translate(dx, dy);
}
static void QPainterPath_translate2(QPainterPath &p, const QPoint &pt) {
    p.translate(pt);
}
static QPainterPath QPainterPath_translated1(const QPainterPath &p, int dx,
                                             int dy) {
    return p.translated(dx, dy);
}
static QPainterPath QPainterPath_translated2(const QPainterPath &p,
                                             const QPoint &pt) {
    return p.translated(pt);
}
static QPainterPath QPainterPath_simplified(const QPainterPath &p) {
    return p.simplified();
}
static bool QPainterPath_containsPoint(const QPainterPath &p,
                                       const QPoint &pt) {
    return p.contains(pt);
}
static bool QPainterPath_containsRect(const QPainterPath &p, const QRect &r) {
    return p.contains(r);
}
static bool QPainterPath_intersectsPath(const QPainterPath &p,
                                        const QPainterPath &other) {
    return p.intersects(other);
}
static bool QPainterPath_intersectsRect(const QPainterPath &p, const QRect &r) {
    return p.intersects(r);
}

#define REGISTER_VALUE_TYPE_COMMON(Type, ScriptName)                           \
    do {                                                                       \
        auto r = engine->RegisterObjectType(                                   \
            ScriptName, sizeof(Type), asOBJ_VALUE | asGetTypeTraits<Type>());  \
        ASSERT(r >= 0);                                                        \
        r = engine->RegisterObjectBehaviour(                                   \
            ScriptName, asBEHAVE_CONSTRUCT, "void f()",                        \
            asFUNCTION(Type##_Ctor), asCALL_CDECL_OBJLAST);                    \
        ASSERT(r >= 0);                                                        \
        r = engine->RegisterObjectBehaviour(                                   \
            ScriptName, asBEHAVE_CONSTRUCT,                                    \
            "void f(const " ScriptName " &in)", asFUNCTION(Type##_CopyCtor),   \
            asCALL_CDECL_OBJLAST);                                             \
        ASSERT(r >= 0);                                                        \
        r = engine->RegisterObjectBehaviour(                                   \
            ScriptName, asBEHAVE_DESTRUCT, "void f()",                         \
            asFUNCTION(Type##_Dtor), asCALL_CDECL_OBJLAST);                    \
        ASSERT(r >= 0);                                                        \
    } while (0)

void RegisterQPoint(asIScriptEngine *engine) {
    // QPoint
    REGISTER_VALUE_TYPE_COMMON(QPoint, "point");
    auto r = engine->RegisterObjectBehaviour(
        "point", asBEHAVE_CONSTRUCT, "void f(int, int)",
        asFUNCTIONPR(
            [](int x, int y, QPoint *self) { new (self) QPoint(x, y); },
            (int, int, QPoint *), void),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("point", "int x() const",
                                     asMETHOD(QPoint, x), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("point", "int y() const",
                                     asMETHOD(QPoint, y), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("point", "void setX(int)",
                                     asMETHOD(QPoint, setX), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("point", "void setY(int)",
                                     asMETHOD(QPoint, setY), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("point", "bool isNull() const",
                                     asMETHOD(QPoint, isNull), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("point", "int manhattanLength() const",
                                     asMETHOD(QPoint, manhattanLength),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("point", "point transposed() const",
                                     asMETHOD(QPoint, transposed),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "point", "point &opAssign(const point &in)",
        asMETHODPR(QPoint, operator=, (const QPoint &), QPoint &),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "point", "bool opEquals(const point &in) const",
        asFUNCTION(QPoint_equals), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "point", "point opAdd(const point &in) const", asFUNCTION(QPoint_add),
        asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "point", "point opSub(const point &in) const", asFUNCTION(QPoint_sub),
        asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("point", "point opMul(int) const",
                                     asFUNCTION(QPoint_mul_int),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("point", "point opMul_r(int) const",
                                     asFUNCTION(QPoint_mul_int_rev),
                                     asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("point", "point opMul(double) const",
                                     asFUNCTION(QPoint_mul_double),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("point", "point opMul_r(double) const",
                                     asFUNCTION(QPoint_mul_double_rev),
                                     asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("point", "point opMul(float) const",
                                     asFUNCTION(QPoint_mul_float),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("point", "point opMul_r(float) const",
                                     asFUNCTION(QPoint_mul_float_rev),
                                     asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("point", "point opDiv(double) const",
                                     asFUNCTION(QPoint_div),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("point", "point opNeg() const",
                                     asFUNCTION(QPoint_neg),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "point", "point &opAddAssign(const point &in)",
        asFUNCTION(QPoint_addAssign), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "point", "point &opSubAssign(const point &in)",
        asFUNCTION(QPoint_subAssign), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("point", "point &opMulAssign(int)",
                                     asFUNCTION(QPoint_mulAssignInt),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("point", "point &opMulAssign(float)",
                                     asFUNCTION(QPoint_mulAssignFloat),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("point", "point &opMulAssign(double)",
                                     asFUNCTION(QPoint_mulAssignDouble),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("point", "point &opDivAssign(double)",
                                     asFUNCTION(QPoint_divAssign),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
}

void RegisterQMargins(asIScriptEngine *engine) {
    REGISTER_VALUE_TYPE_COMMON(QMargins, "margins");

    auto r = engine->RegisterObjectBehaviour(
        "margins", asBEHAVE_CONSTRUCT, "void f(int, int, int, int)",
        asFUNCTION(QMargins_Ctor4), asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("margins", "int left() const",
                                     asMETHOD(QMargins, left), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "int top() const",
                                     asMETHOD(QMargins, top), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "int right() const",
                                     asMETHOD(QMargins, right),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "int bottom() const",
                                     asMETHOD(QMargins, bottom),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("margins", "void setLeft(int)",
                                     asMETHOD(QMargins, setLeft),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "void setTop(int)",
                                     asMETHOD(QMargins, setTop),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "void setRight(int)",
                                     asMETHOD(QMargins, setRight),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "void setBottom(int)",
                                     asMETHOD(QMargins, setBottom),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("margins", "bool isNull() const",
                                     asMETHOD(QMargins, isNull),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "margins", "margins opAdd(const margins &in) const",
        asFUNCTION(QMargins_AddMM), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "margins opAdd(int) const",
                                     asFUNCTION(QMargins_AddMI),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "margins opAdd_r(int) const",
                                     asFUNCTION(QMargins_AddIM),
                                     asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "margins", "margins opSub(const margins &in) const",
        asFUNCTION(QMargins_SubMM), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "margins opSub(int) const",
                                     asFUNCTION(QMargins_SubMI),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("margins", "margins opMul(int) const",
                                     asFUNCTION(QMargins_MulMI),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "margins opMul_r(int) const",
                                     asFUNCTION(QMargins_MulIM),
                                     asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "margins opMul(double) const",
                                     asFUNCTION(QMargins_MulMD),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "margins opMul_r(double) const",
                                     asFUNCTION(QMargins_MulDM),
                                     asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("margins", "margins opDiv(int) const",
                                     asFUNCTION(QMargins_DivMI),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "margins opDiv(double) const",
                                     asFUNCTION(QMargins_DivMD),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "margins opNeg() const",
                                     asFUNCTION(QMargins_Neg),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "margins", "margins &opAddAssign(const margins &in)",
        asFUNCTION(QMargins_AddAssignMM), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "margins &opAddAssign(int)",
                                     asFUNCTION(QMargins_AddAssignMI),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "margins", "margins &opSubAssign(const margins &in)",
        asFUNCTION(QMargins_SubAssignMM), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "margins &opSubAssign(int)",
                                     asFUNCTION(QMargins_SubAssignMI),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("margins", "margins &opMulAssign(int)",
                                     asFUNCTION(QMargins_MulAssignMI),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "margins &opMulAssign(double)",
                                     asFUNCTION(QMargins_MulAssignMD),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("margins", "margins &opDivAssign(int)",
                                     asFUNCTION(QMargins_DivAssignMI),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("margins", "margins &opDivAssign(double)",
                                     asFUNCTION(QMargins_DivAssignMD),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "margins", "margins opOr(const margins &in) const",
        asFUNCTION(QMargins_Or), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "margins", "margins &opAssign(const margins &in)",
        asMETHODPR(QMargins, operator=, (const QMargins &), QMargins &),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "margins", "bool opEquals(const margins &in) const",
        asFUNCTION(QMargins_Equal), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
}

void RegisterQSize(asIScriptEngine *engine) {
    REGISTER_VALUE_TYPE_COMMON(QSize, "size");
    auto r = engine->RegisterObjectBehaviour(
        "size", asBEHAVE_CONSTRUCT, "void f(int, int)",
        asFUNCTIONPR([](int w, int h, QSize *self) { new (self) QSize(w, h); },
                     (int, int, QSize *), void),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    // basic accessors
    r = engine->RegisterObjectMethod("size", "int width() const",
                                     asMETHOD(QSize, width), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("size", "int height() const",
                                     asMETHOD(QSize, height), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("size", "void setWidth(int)",
                                     asMETHOD(QSize, setWidth),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("size", "void setHeight(int)",
                                     asMETHOD(QSize, setHeight),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("size", "bool isEmpty() const",
                                     asMETHOD(QSize, isEmpty), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("size", "bool isNull() const",
                                     asMETHOD(QSize, isNull), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("size", "bool isValid() const",
                                     asMETHOD(QSize, isValid), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("size", "void transpose()",
                                     asMETHOD(QSize, transpose),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("size", "size transposed() const",
                                     asMETHOD(QSize, transposed),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "size", "void scale(int w, int h, shape::AspectRatioMode mode)",
        asMETHODPR(QSize, scale, (int, int, Qt::AspectRatioMode), void),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "size", "void scale(const size &in s, shape::AspectRatioMode mode)",
        asMETHODPR(QSize, scale, (const QSize &, Qt::AspectRatioMode), void),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "size", "size scale(int w, int h, shape::AspectRatioMode mode) const",
        asMETHODPR(QSize, scaled, (int, int, Qt::AspectRatioMode) const, QSize),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "size",
        "size scale(const size &in s, shape::AspectRatioMode mode) const",
        asMETHODPR(QSize, scaled, (const QSize &, Qt::AspectRatioMode) const,
                   QSize),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "size", "size expandedTo(const size& in) const",
        asMETHOD(QSize, expandedTo), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "size", "size boundedTo(const size& in) const",
        asMETHOD(QSize, boundedTo), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("size", "size grownBy(margins) const",
                                     asMETHOD(QSize, grownBy), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("size", "size shrunkBy(margins) const",
                                     asMETHOD(QSize, shrunkBy),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("size", "size opAdd(const size &in) const",
                                     asFUNCTION(QSize_Add),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("size", "size opSub(const size &in) const",
                                     asFUNCTION(QSize_Sub),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("size", "size opMul(double) const",
                                     asFUNCTION(QSize_Mul),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("size", "size opMul_r(double) const",
                                     asFUNCTION(QSize_Mul_r),
                                     asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("size", "size opDiv(double) const",
                                     asFUNCTION(QSize_Div),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "size", "size &opAddAssign(const size &in)",
        asMETHODPR(QSize, operator+=, (const QSize &), QSize &),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "size", "size &opSubAssign(const size &in)",
        asMETHODPR(QSize, operator-=, (const QSize &), QSize &),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "size", "size &opMulAssign(double)",
        asMETHODPR(QSize, operator*=, (qreal), QSize &), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "size", "size &opDivAssign(double)",
        asMETHODPR(QSize, operator/=, (qreal), QSize &), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "size", "size &opAssign(const size &in)",
        asMETHODPR(QSize, operator=, (const QSize &), QSize &),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "size", "bool opEquals(const size &in) const", asFUNCTION(QSize_equals),
        asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
}

void RegisterQRect(asIScriptEngine *engine) {
    // QRect
    REGISTER_VALUE_TYPE_COMMON(QRect, "rect");
    auto r = engine->RegisterObjectBehaviour(
        "rect", asBEHAVE_CONSTRUCT, "void f(const point &in, const point &in)",
        asFUNCTIONPR([](const QPoint &tl, const QPoint &br,
                        QRect *self) { new (self) QRect(tl, br); },
                     (const QPoint &, const QPoint &, QRect *), void),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "rect", asBEHAVE_CONSTRUCT, "void f(const point &in, const size &in)",
        asFUNCTIONPR([](const QPoint &tl, const QSize &sz,
                        QRect *self) { new (self) QRect(tl, sz); },
                     (const QPoint &, const QSize &, QRect *), void),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "rect", asBEHAVE_CONSTRUCT, "void f(int, int, int, int)",
        asFUNCTIONPR([](int x, int y, int w, int h,
                        QRect *self) { new (self) QRect(x, y, w, h); },
                     (int, int, int, int, QRect *), void),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "int x() const",
                                     asMETHOD(QRect, x), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "int y() const",
                                     asMETHOD(QRect, y), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "int left() const",
                                     asMETHOD(QRect, left), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "int top() const",
                                     asMETHOD(QRect, top), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "int right() const",
                                     asMETHOD(QRect, right), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "int bottom() const",
                                     asMETHOD(QRect, bottom), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "int width() const",
                                     asMETHOD(QRect, width), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "int height() const",
                                     asMETHOD(QRect, height), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "size size() const",
                                     asMETHOD(QRect, size), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "point topLeft() const",
                                     asMETHOD(QRect, topLeft), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "point bottomRight() const",
                                     asMETHOD(QRect, bottomRight),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "point topRight() const",
                                     asMETHOD(QRect, topRight),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "point bottomLeft() const",
                                     asMETHOD(QRect, bottomLeft),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "point center() const",
                                     asMETHOD(QRect, center), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "void setX(int)",
                                     asMETHOD(QRect, setX), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "void setY(int)",
                                     asMETHOD(QRect, setY), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "void setLeft(int)",
                                     asMETHOD(QRect, setLeft), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "void setTop(int)",
                                     asMETHOD(QRect, setTop), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "void setRight(int)",
                                     asMETHOD(QRect, setRight),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "void setBottom(int)",
                                     asMETHOD(QRect, setBottom),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("rect", "void setTopLeft(const point &in)",
                                     asMETHOD(QRect, setTopLeft),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "void setBottomRight(const point &in)",
        asMETHOD(QRect, setBottomRight), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "void setTopRight(const point &in)",
        asMETHOD(QRect, setTopRight), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "void setBottomLeft(const point &in)",
        asMETHOD(QRect, setBottomLeft), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("rect", "void setWidth(int)",
                                     asMETHOD(QRect, setWidth),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "void setHeight(int)",
                                     asMETHOD(QRect, setHeight),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "void setSize(const size &in)",
                                     asMETHOD(QRect, setSize), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("rect", "rect normalized() const",
                                     asMETHOD(QRect, normalized),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "bool isNull() const",
                                     asMETHOD(QRect, isNull), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "bool isEmpty() const",
                                     asMETHOD(QRect, isEmpty), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "bool isValid() const",
                                     asMETHOD(QRect, isValid), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("rect",
                                     "void setRect(int x, int y, int w, int h)",
                                     asMETHOD(QRect, setRect), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "void getRect(int &out x, int &out y, int &out w, int &out h)",
        asMETHOD(QRect, getRect), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "rect", "void setCoords(int x1, int y1, int x2, int y2)",
        asMETHOD(QRect, setCoords), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect",
        "void getCoords(int &out x1, int &out y1, int &out x2, int &out y2)",
        asMETHOD(QRect, getCoords), asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("rect", "void moveLeft(int)",
                                     asMETHOD(QRect, moveLeft),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "void moveTop(int)",
                                     asMETHOD(QRect, moveTop), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "void moveRight(int)",
                                     asMETHOD(QRect, moveRight),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "void moveBottom(int)",
                                     asMETHOD(QRect, moveBottom),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "void moveTopLeft(const point &in)",
        asMETHOD(QRect, moveTopLeft), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "void moveBottomRight(const point &in)",
        asMETHOD(QRect, moveBottomRight), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "void moveTopRight(const point &in)",
        asMETHOD(QRect, moveTopRight), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "void moveBottomLeft(const point &in)",
        asMETHOD(QRect, moveBottomLeft), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "void moveCenter(const point &in)",
                                     asMETHOD(QRect, moveCenter),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "rect", "void moveTo(int, int)",
        asMETHODPR(QRect, moveTo, (int, int), void), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "void moveTo(const point &in)",
        asMETHODPR(QRect, moveTo, (const QPoint &), void), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "void translate(int, int)",
        asMETHODPR(QRect, translate, (int, int), void), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "void translate(const point &in)",
        asMETHODPR(QRect, translate, (const QPoint &), void), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect", "rect transposed() const",
                                     asMETHOD(QRect, transposed),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "void adjust(int x1, int y1, int x2, int y2)",
        asMETHOD(QRect, adjust), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "rect translated(int, int) const",
        asMETHODPR(QRect, translated, (int, int) const, QRect),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "rect translated(const point &in) const",
        asMETHODPR(QRect, translated, (const QPoint &) const, QRect),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "rect adjusted(int x1, int y1, int x2, int y2) const",
        asMETHOD(QRect, adjusted), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "bool contains(int x, int y) const",
        asMETHODPR(QRect, contains, (int, int) const, bool), asCALL_THISCALL);
    r = engine->RegisterObjectMethod(
        "rect", "bool contains(int x, int y, bool proper) const",
        asMETHODPR(QRect, contains, (int, int, bool) const, bool),
        asCALL_THISCALL);
    r = engine->RegisterObjectMethod(
        "rect", "bool contains(const point &in, bool proper = false) const",
        asMETHODPR(QRect, contains, (const QPoint &, bool) const, bool),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "bool contains(const rect &in, bool proper = false) const",
        asMETHODPR(QRect, contains, (const QRect &, bool) const, bool),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "bool intersects(const rect &in) const",
        asMETHOD(QRect, intersects), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "rect intersected(const rect &in) const",
        asMETHOD(QRect, intersected), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("rect",
                                     "rect united(const rect &in) const",
                                     asMETHOD(QRect, united), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "rect", "bool opEquals(const rect &in) const", asFUNCTION(QRect_equals),
        asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "QRect", "QRect &opAssign(const QRect &in)",
        asMETHODPR(QRect, operator=, (const QRect &), QRect &),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "QRect", "QRect opOr(const QRect &in) const",
        asMETHODPR(QRect, operator|, (const QRect &) const, QRect),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "QRect", "QRect opAnd(const QRect &in) const",
        asMETHODPR(QRect, operator&, (const QRect &) const, QRect),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "QRect", "QRect &opOrAssign(const QRect &in)",
        asMETHODPR(QRect, operator|=, (const QRect &), QRect &),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "QRect", "QRect &opAndAssign(const QRect &in)",
        asMETHODPR(QRect, operator&=, (const QRect &), QRect &),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "QRect", "QRect marginsAdded(const QMargins &in) const",
        asMETHODPR(QRect, marginsAdded, (const QMargins &) const, QRect),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "QRect", "QRect marginsRemoved(const QMargins &in) const",
        asMETHODPR(QRect, marginsRemoved, (const QMargins &) const, QRect),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "QRect", "QRect &opAddAssign(const QMargins &in)",
        asMETHODPR(QRect, operator+=, (const QMargins &), QRect &),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "QRect", "QRect &opSubAssign(const QMargins &in)",
        asMETHODPR(QRect, operator-=, (const QMargins &), QRect &),
        asCALL_THISCALL);
    ASSERT(r >= 0);
}

void RegisterQLine(asIScriptEngine *engine) {
    // QLine
    REGISTER_VALUE_TYPE_COMMON(QLine, "line");
    auto r = engine->RegisterObjectBehaviour(
        "line", asBEHAVE_CONSTRUCT, "void f(const point &in, const point &in)",
        asFUNCTIONPR([](const QPoint &p1, const QPoint &p2,
                        QLine *self) { new (self) QLine(p1, p2); },
                     (const QPoint &, const QPoint &, QLine *), void),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "line", asBEHAVE_CONSTRUCT, "void f(int, int, int, int)",
        asFUNCTIONPR([](int x1, int y1, int x2, int y2,
                        QLine *self) { new (self) QLine(x1, y1, x2, y2); },
                     (int, int, int, int, QLine *), void),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("line", "point p1() const",
                                     asMETHOD(QLine, p1), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("line", "point p2() const",
                                     asMETHOD(QLine, p2), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("line", "int x1() const",
                                     asMETHOD(QLine, x1), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("line", "int y1() const",
                                     asMETHOD(QLine, y1), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("line", "int x2() const",
                                     asMETHOD(QLine, x2), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("line", "int y2() const",
                                     asMETHOD(QLine, y2), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("line", "point center() const",
                                     asMETHOD(QLine, center), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("line", "int dx() const",
                                     asMETHOD(QLine, dx), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("line", "int dy() const",
                                     asMETHOD(QLine, dy), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("line", "bool isNull() const",
                                     asMETHOD(QLine, isNull), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("line", "void setP1(const point &in)",
                                     asMETHOD(QLine, setP1), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("line", "void setP2(const point &in)",
                                     asMETHOD(QLine, setP2), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("line", "void setLine(int, int, int, int)",
                                     asMETHOD(QLine, setLine), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "line", "void setPoints(const point &in, const point &in)",
        asMETHOD(QLine, setPoints), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "line", "void translate(int, int)",
        asMETHODPR(QLine, translate, (int, int), void), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "line", "void translate(const point &in)",
        asMETHODPR(QLine, translate, (const QPoint &), void), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "line", "line translated(int, int) const",
        asMETHODPR(QLine, translated, (int, int) const, QLine),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "line", "line translated(const point &in) const",
        asMETHODPR(QLine, translated, (const QPoint &) const, QLine),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "line", "bool opEquals(const line &in) const", asFUNCTION(QLine_equals),
        asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "QLine", "line &opAssign(const line &in)",
        asMETHODPR(QLine, operator=, (const QLine &), QLine &),
        asCALL_THISCALL);
    ASSERT(r >= 0);
}

void RegisterQPolygon(asIScriptEngine *engine) {
    REGISTER_VALUE_TYPE_COMMON(QPolygon, "polygon");
    auto r = engine->RegisterObjectBehaviour(
        "polygon", asBEHAVE_CONSTRUCT, "void f(const array<point> &in)",
        asFUNCTION(QPolygon_CtorFromArray), asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "polygon", asBEHAVE_CONSTRUCT,
        "void f(const rect &in, bool closed = false)",
        asFUNCTIONPR([](const QRect &r, bool closed,
                        QPolygon *self) { new (self) QPolygon(r, closed); },
                     (const QRect &, bool, QPolygon *), void),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("polygon", "int size() const",
                                     asMETHOD(QPolygon, size), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("polygon", "bool isEmpty() const",
                                     asMETHOD(QPolygon, isEmpty),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "polygon", "void clear()", asMETHOD(QPolygon, clear), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("polygon", "rect boundingRect() const",
                                     asMETHOD(QPolygon, boundingRect),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "polygon", "point pointAt(int) const",
        asMETHODPR(QPolygon, point, (int) const, QPoint), asCALL_THISCALL);
    ASSERT(r >= 0);
    // TODO
    r = engine->RegisterObjectMethod(
        "polygon", "void addPoint(const point &in)",
        asMETHODPR(QPolygon, append, (const QPoint &), void), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "QPolygon", "void addPoints(const array<point> &in)",
        asFUNCTION(QPolygon_AddPoints), asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "polygon", "void insertPoint(int, const point &in)",
        asFUNCTION(QPolygon_insertPoint), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "polygon", "void setPoint(int, const point &in)",
        asMETHODPR(QPolygon, setPoint, (int, const QPoint &), void),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("polygon", "void removePointAt(int)",
                                     asFUNCTION(QPolygon_removePointAt),
                                     asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "polygon", "void translate(int, int)",
        asMETHODPR(QPolygon, translate, (int, int), void), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "polygon", "void translate(const point &in)",
        asMETHODPR(QPolygon, translate, (const QPoint &), void),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "polygon", "polygon translated(int, int) const",
        asMETHODPR(QPolygon, translated, (int, int) const, QPolygon),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "polygon", "polygon translated(const point &in) const",
        asMETHODPR(QPolygon, translated, (const QPoint &) const, QPolygon),
        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "polygon", "bool containsPoint(const point &in, shape::FillRule) const",
        asMETHOD(QPolygon, containsPoint), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "polygon", "bool intersects(const polygon &in) const",
        asMETHOD(QPolygon, intersects), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "polygon", "polygon intersected(const polygon &in) const",
        asMETHOD(QPolygon, intersected), asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "polygon", "polygon united(const polygon &in) const",
        asMETHOD(QPolygon, united), asCALL_THISCALL);

    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "polygon", "bool opEquals(const polygon &in) const",
        asFUNCTION(QPolygon_equals), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "QPolygon", "QPolygon &opAssign(const QPolygon &in)",
        asMETHODPR(QPolygon, operator=, (const QPolygon &), QPolygon &),
        asCALL_THISCALL);
    ASSERT(r >= 0);
}

void RegisterASQtGeometryTypes(asIScriptEngine *engine) {
    ASSERT(engine);

    int r = 0;

    // some essential enums
    r = engine->SetDefaultNamespace("shape");
    ASSERT(r >= 0);
    r = engine->RegisterEnum("FillRule");
    ASSERT(r >= 0);
    r = engine->RegisterEnumValue("FillRule", "OddEvenFill", Qt::OddEvenFill);
    ASSERT(r >= 0);
    r = engine->RegisterEnumValue("FillRule", "WindingFill", Qt::WindingFill);
    ASSERT(r >= 0);

    r = engine->RegisterEnum("AspectRatioMode");
    ASSERT(r >= 0);
    r = engine->RegisterEnumValue("AspectRatioMode", "IgnoreAspectRatio",
                                  Qt::IgnoreAspectRatio);
    ASSERT(r >= 0);
    r = engine->RegisterEnumValue("AspectRatioMode", "KeepAspectRatio",
                                  Qt::KeepAspectRatio);
    ASSERT(r >= 0);
    r = engine->RegisterEnumValue("AspectRatioMode",
                                  "KeepAspectRatioByExpanding",
                                  Qt::KeepAspectRatioByExpanding);
    ASSERT(r >= 0);

    RegisterQPoint(engine);
    RegisterQMargins(engine);
    RegisterQSize(engine);
    RegisterQRect(engine);
    RegisterQLine(engine);

    // QPainterPath
    REGISTER_VALUE_TYPE_COMMON(QPainterPath, "path");
    r = engine->RegisterObjectMethod("path", "void clear()",
                                     asFUNCTION(QPainterPath_clear),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("path", "bool isEmpty() const",
                                     asFUNCTION(QPainterPath_isEmpty),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("path", "point currentPosition() const",
                                     asFUNCTION(QPainterPath_currentPosition),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("path", "rect boundingRect() const",
                                     asFUNCTION(QPainterPath_boundingRect),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("path", "rect controlPointRect() const",
                                     asFUNCTION(QPainterPath_controlPointRect),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("path", "shape::FillRule fillRule() const",
                                     asFUNCTION(QPainterPath_fillRule),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "path", "void setFillRule(shape::FillRule)",
        asFUNCTION(QPainterPath_setFillRule), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("path", "void moveTo(int, int)",
                                     asFUNCTION(QPainterPath_moveTo1),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("path", "void moveTo(const point &in)",
                                     asFUNCTION(QPainterPath_moveTo2),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("path", "void lineTo(int, int)",
                                     asFUNCTION(QPainterPath_lineTo1),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("path", "void lineTo(const point &in)",
                                     asFUNCTION(QPainterPath_lineTo2),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("path", "void quadTo(int, int, int, int)",
                                     asFUNCTION(QPainterPath_quadTo1),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "path", "void quadTo(const point &in, const point &in)",
        asFUNCTION(QPainterPath_quadTo2), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "path", "void cubicTo(int, int, int, int, int, int)",
        asFUNCTION(QPainterPath_cubicTo1), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "path",
        "void cubicTo(const point &in, const point &in, const point &in)",
        asFUNCTION(QPainterPath_cubicTo2), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("path", "void closeSubpath()",
                                     asFUNCTION(QPainterPath_closeSubpath),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("path", "void addPath(const path &in)",
                                     asFUNCTION(QPainterPath_addPath),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("path", "void connectPath(const path &in)",
                                     asFUNCTION(QPainterPath_connectPath),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("path", "void addRect(int, int, int, int)",
                                     asFUNCTION(QPainterPath_addRect1),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("path", "void addRect(const rect &in)",
                                     asFUNCTION(QPainterPath_addRect2),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "path", "void addEllipse(int, int, int, int)",
        asFUNCTION(QPainterPath_addEllipse1), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "path", "void addEllipse(const point &in, int, int)",
        asFUNCTION(QPainterPath_addEllipse2), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("path", "void translate(int, int)",
                                     asFUNCTION(QPainterPath_translate1),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("path", "void translate(const point &in)",
                                     asFUNCTION(QPainterPath_translate2),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("path", "path translated(int, int) const",
                                     asFUNCTION(QPainterPath_translated1),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "path", "path translated(const point &in) const",
        asFUNCTION(QPainterPath_translated2), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("path", "path simplified() const",
                                     asFUNCTION(QPainterPath_simplified),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "path", "bool contains(const point &in) const",
        asFUNCTION(QPainterPath_containsPoint), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "path", "bool contains(const rect &in) const",
        asFUNCTION(QPainterPath_containsRect), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "path", "bool intersects(const path &in) const",
        asFUNCTION(QPainterPath_intersectsPath), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "path", "bool intersects(const rect &in) const",
        asFUNCTION(QPainterPath_intersectsRect), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "path", "bool opEquals(const path &in) const",
        asFUNCTION(QPainterPath_equals), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    engine->SetDefaultNamespace("");
}