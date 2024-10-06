#include "imagecropper.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

namespace {
static const QSize WIDGET_MINIMUM_SIZE(100, 100);
}

ImageCropper::ImageCropper(QWidget *parent)
    : QWidget(parent), pimpl(new ImageCropperPrivate) {
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setMinimumSize(WIDGET_MINIMUM_SIZE);
    setMouseTracking(true);
}

ImageCropper::~ImageCropper() { delete pimpl; }

bool ImageCropper::cropperVisible() const { return pimpl->cropVisible; }

QSizeF ImageCropper::proportion() const { return pimpl->proportion; }

bool ImageCropper::proportionFixed() const { return pimpl->isProportionFixed; }

QRectF ImageCropper::selRect() const { return pimpl->croppingRect; }

void ImageCropper::setSelRect(int x, int y, int w, int h) {
    if (pimpl->isProportionFixed) {
        pimpl->croppingRect = QRectF(
            QPointF(x, y), pimpl->proportion.scaled(w, h, Qt::KeepAspectRatio));
    } else {
        pimpl->croppingRect = QRectF(x, y, w, h).intersected(this->rect());
    }
    update();
}

void ImageCropper::setImage(const QImage &_image) {
    pimpl->imageForCropping = _image;
    setFixedSize(_image.size());

    update();
}

void ImageCropper::setBackgroundColor(const QColor &_backgroundColor) {
    pimpl->backgroundColor = _backgroundColor;
    update();
}

void ImageCropper::setCroppingRectBorderColor(const QColor &_borderColor) {
    pimpl->croppingRectBorderColor = _borderColor;
    update();
}

void ImageCropper::setProportion(const QSizeF &_proportion) {
    if (pimpl->proportion != _proportion) {
        pimpl->proportion = _proportion;

        float heightDelta = (float)_proportion.height() / _proportion.width();
        float widthDelta = (float)_proportion.width() / _proportion.height();

        pimpl->deltas.setHeight(heightDelta);
        pimpl->deltas.setWidth(widthDelta);
    }

    if (pimpl->isProportionFixed) {
        float croppintRectSideRelation =
            (float)pimpl->croppingRect.width() / pimpl->croppingRect.height();
        float proportionSideRelation =
            (float)pimpl->proportion.width() / pimpl->proportion.height();

        if (croppintRectSideRelation != proportionSideRelation) {
            bool widthShotrerThenHeight =
                pimpl->croppingRect.width() < pimpl->croppingRect.height();
            if (widthShotrerThenHeight) {
                pimpl->croppingRect.setHeight(pimpl->croppingRect.width() *
                                              pimpl->deltas.height());
            } else {
                pimpl->croppingRect.setWidth(pimpl->croppingRect.height() *
                                             pimpl->deltas.width());
            }

            update();
        }
    }
}

void ImageCropper::setProportionFixed(const bool _isFixed) {
    if (pimpl->isProportionFixed != _isFixed) {
        pimpl->isProportionFixed = _isFixed;
        setProportion(pimpl->proportion);
    }
}

void ImageCropper::setCropperVisible(bool b) {
    pimpl->cropVisible = b;
    update();
}

// ********
// Protected section

void ImageCropper::paintEvent(QPaintEvent *_event) {
    QWidget::paintEvent(_event);

    QPainter widgetPainter(this);

    widgetPainter.fillRect(_event->rect(), pimpl->backgroundColor);

    widgetPainter.drawImage(this->rect(), pimpl->imageForCropping);

    if (pimpl->cropVisible) {
        if (pimpl->croppingRect.isNull()) {
            const int width = WIDGET_MINIMUM_SIZE.width() / 2;
            const int height = WIDGET_MINIMUM_SIZE.height() / 2;

            pimpl->croppingRect.setSize(pimpl->proportion.scaled(
                width, height, Qt::KeepAspectRatioByExpanding));
            float x = (this->width() - pimpl->croppingRect.width()) / 2;
            float y = (this->height() - pimpl->croppingRect.height()) / 2;
            pimpl->croppingRect.moveTo(x, y);
            emit selRectChanged(pimpl->croppingRect);
        }

        QPainterPath p;
        p.addRect(pimpl->croppingRect);
        p.addRect(this->rect());
        widgetPainter.setBrush(QBrush(QColor(0, 0, 0, 120)));
        widgetPainter.setPen(Qt::transparent);
        widgetPainter.drawPath(p);

        widgetPainter.setPen(pimpl->croppingRectBorderColor);

        widgetPainter.setBrush(QBrush(Qt::transparent));
        widgetPainter.drawRect(pimpl->croppingRect);

        widgetPainter.setBrush(QBrush(pimpl->croppingRectBorderColor));

        int leftXCoord = pimpl->croppingRect.left() - 2;
        int centerXCoord = pimpl->croppingRect.center().x() - 3;
        int rightXCoord = pimpl->croppingRect.right() - 2;

        int topYCoord = pimpl->croppingRect.top() - 2;
        int middleYCoord = pimpl->croppingRect.center().y() - 3;
        int bottomYCoord = pimpl->croppingRect.bottom() - 2;
        //
        const QSize pointSize(6, 6);
        //
        QVector<QRect> points;
        points << QRect(QPoint(leftXCoord, topYCoord), pointSize)
               << QRect(QPoint(leftXCoord, middleYCoord), pointSize)
               << QRect(QPoint(leftXCoord, bottomYCoord), pointSize)

               << QRect(QPoint(centerXCoord, topYCoord), pointSize)
               << QRect(QPoint(centerXCoord, middleYCoord), pointSize)
               << QRect(QPoint(centerXCoord, bottomYCoord), pointSize)

               << QRect(QPoint(rightXCoord, topYCoord), pointSize)
               << QRect(QPoint(rightXCoord, middleYCoord), pointSize)
               << QRect(QPoint(rightXCoord, bottomYCoord), pointSize);
        //
        widgetPainter.drawRects(points);

        QPen dashPen(pimpl->croppingRectBorderColor);
        dashPen.setStyle(Qt::DashLine);
        widgetPainter.setPen(dashPen);
        widgetPainter.drawLine(
            QPoint(pimpl->croppingRect.center().x(), pimpl->croppingRect.top()),
            QPoint(pimpl->croppingRect.center().x(),
                   pimpl->croppingRect.bottom()));
        widgetPainter.drawLine(QPoint(pimpl->croppingRect.left(),
                                      pimpl->croppingRect.center().y()),
                               QPoint(pimpl->croppingRect.right(),
                                      pimpl->croppingRect.center().y()));
    }

    widgetPainter.end();
}

void ImageCropper::mousePressEvent(QMouseEvent *_event) {
    if (_event->button() == Qt::LeftButton) {
        pimpl->isMousePressed = true;
        pimpl->startMousePos = _event->pos();
        pimpl->lastStaticCroppingRect = pimpl->croppingRect;
    }
    //
    updateCursorIcon(_event->pos());
}

void ImageCropper::mouseMoveEvent(QMouseEvent *_event) {
    QPointF mousePos = _event->pos();
    if (!pimpl->isMousePressed) {
        pimpl->cursorPosition = cursorPosition(pimpl->croppingRect, mousePos);
        updateCursorIcon(mousePos);
    } else if (pimpl->cursorPosition != CursorPositionUndefined) {
        QPointF mouseDelta;
        mouseDelta.setX(mousePos.x() - pimpl->startMousePos.x());
        mouseDelta.setY(mousePos.y() - pimpl->startMousePos.y());
        //
        if (pimpl->cursorPosition != CursorPositionMiddle) {
            QRectF newGeometry =
                calculateGeometry(pimpl->lastStaticCroppingRect,
                                  pimpl->cursorPosition, mouseDelta);

            if (!newGeometry.isNull()) {
                pimpl->croppingRect = newGeometry;
                emit selRectChanged(pimpl->croppingRect);
            }
        } else {
            auto p = pimpl->lastStaticCroppingRect.topLeft() + mouseDelta;
            if (p.x() < this->rect().left()) {
                p.setX(this->rect().left());
            }
            if (p.y() < this->rect().top()) {
                p.setY(this->rect().top());
            }
            auto width = pimpl->croppingRect.width();
            if (p.x() + width > this->rect().right()) {
                p.setX(this->rect().right() - width);
            }
            auto height = pimpl->croppingRect.height();
            if (p.y() + height > this->rect().bottom()) {
                p.setY(this->rect().bottom() - height);
            }
            pimpl->croppingRect.moveTo(p);
            emit selRectChanged(pimpl->croppingRect);
        }
        update();
    }
}

void ImageCropper::mouseReleaseEvent(QMouseEvent *_event) {
    pimpl->isMousePressed = false;
    updateCursorIcon(_event->pos());
}

void ImageCropper::resizeEvent(QResizeEvent *event) {
    auto s = event->oldSize();
    auto ns = this->size();

    auto oldRect = getImageRect(s);
    auto newRect = getImageRect(ns);

    auto xscale = double(newRect.width()) / oldRect.width();
    auto yscale = double(newRect.height()) / oldRect.height();

    auto lt = pimpl->croppingRect.topLeft() - this->rect().topLeft();
    lt.setX(lt.x() * xscale);
    lt.setY(lt.y() * yscale);
    lt += newRect.topLeft();
    pimpl->croppingRect.moveTo(lt);

    auto rs = pimpl->croppingRect.size();
    rs.setWidth(rs.width() * xscale);
    rs.setHeight(rs.height() * yscale);
    pimpl->croppingRect.setSize(rs);

    emit selRectChanged(pimpl->croppingRect);
    emit sizeChanged();
    update();
}

// ********
// Private section

namespace {
static bool isPointNearSide(const int _sideCoordinate,
                            const int _pointCoordinate) {
    static const int indent = 10;
    return (_sideCoordinate - indent) < _pointCoordinate &&
           _pointCoordinate < (_sideCoordinate + indent);
}
} // namespace

CursorPosition ImageCropper::cursorPosition(const QRectF &_cropRect,
                                            const QPointF &_mousePosition) {
    CursorPosition cursorPosition = CursorPositionUndefined;
    //
    if (_cropRect.contains(_mousePosition)) {
        if (isPointNearSide(_cropRect.top(), _mousePosition.y()) &&
            isPointNearSide(_cropRect.left(), _mousePosition.x())) {
            cursorPosition = CursorPositionTopLeft;
        } else if (isPointNearSide(_cropRect.bottom(), _mousePosition.y()) &&
                   isPointNearSide(_cropRect.left(), _mousePosition.x())) {
            cursorPosition = CursorPositionBottomLeft;
        } else if (isPointNearSide(_cropRect.top(), _mousePosition.y()) &&
                   isPointNearSide(_cropRect.right(), _mousePosition.x())) {
            cursorPosition = CursorPositionTopRight;
        } else if (isPointNearSide(_cropRect.bottom(), _mousePosition.y()) &&
                   isPointNearSide(_cropRect.right(), _mousePosition.x())) {
            cursorPosition = CursorPositionBottomRight;
        } else if (isPointNearSide(_cropRect.left(), _mousePosition.x())) {
            cursorPosition = CursorPositionLeft;
        } else if (isPointNearSide(_cropRect.right(), _mousePosition.x())) {
            cursorPosition = CursorPositionRight;
        } else if (isPointNearSide(_cropRect.top(), _mousePosition.y())) {
            cursorPosition = CursorPositionTop;
        } else if (isPointNearSide(_cropRect.bottom(), _mousePosition.y())) {
            cursorPosition = CursorPositionBottom;
        } else {
            cursorPosition = CursorPositionMiddle;
        }
    }
    //
    return cursorPosition;
}

void ImageCropper::updateCursorIcon(const QPointF &_mousePosition) {
    if (!pimpl->cropVisible) {
        qApp->setOverrideCursor(Qt::ArrowCursor);
        return;
    }

    QCursor cursorIcon;
    //
    switch (cursorPosition(pimpl->croppingRect, _mousePosition)) {
    case CursorPositionTopRight:
    case CursorPositionBottomLeft:
        cursorIcon = QCursor(Qt::SizeBDiagCursor);
        break;
    case CursorPositionTopLeft:
    case CursorPositionBottomRight:
        cursorIcon = QCursor(Qt::SizeFDiagCursor);
        break;
    case CursorPositionTop:
    case CursorPositionBottom:
        cursorIcon = QCursor(Qt::SizeVerCursor);
        break;
    case CursorPositionLeft:
    case CursorPositionRight:
        cursorIcon = QCursor(Qt::SizeHorCursor);
        break;
    case CursorPositionMiddle:
        cursorIcon = pimpl->isMousePressed ? QCursor(Qt::ClosedHandCursor)
                                           : QCursor(Qt::OpenHandCursor);
        break;
    case CursorPositionUndefined:
    default:
        cursorIcon = QCursor(Qt::ArrowCursor);
        break;
    }
    //
    qApp->setOverrideCursor(cursorIcon);
}

const QRectF
ImageCropper::calculateGeometry(const QRectF &_sourceGeometry,
                                const CursorPosition _cursorPosition,
                                const QPointF &_mouseDelta) {
    QRectF resultGeometry;

    if (pimpl->isProportionFixed) {
        resultGeometry = calculateGeometryWithFixedProportions(
            _sourceGeometry, _cursorPosition, _mouseDelta, pimpl->deltas);
    } else {
        resultGeometry = calculateGeometryWithCustomProportions(
            _sourceGeometry, _cursorPosition, _mouseDelta);
    }
    if ((resultGeometry.left() >= resultGeometry.right()) ||
        (resultGeometry.top() >= resultGeometry.bottom())) {
        resultGeometry = QRect();
    } else {
        if (pimpl->isProportionFixed) {
            qreal tldiv = 0;
            qreal brdiv = 0;
            qreal trdiv = 0;
            qreal bldiv = 0;

            auto tl = resultGeometry.topLeft();
            if (!this->rect().contains(tl.toPoint())) {
                auto xdiv =
                    (this->rect().left() - tl.x()) / pimpl->proportion.width();
                auto ydiv =
                    (this->rect().top() - tl.y()) / pimpl->proportion.height();
                tldiv = qMax(xdiv, ydiv);
            }

            auto tr = resultGeometry.topRight();
            if (!this->rect().contains(tr.toPoint())) {
                auto xdiv =
                    (tr.x() - this->rect().right()) / pimpl->proportion.width();
                auto ydiv =
                    (this->rect().top() - tr.y()) / pimpl->proportion.height();
                trdiv = qMax(xdiv, ydiv);
            }

            auto br = resultGeometry.bottomRight();
            if (!this->rect().contains(br.toPoint())) {
                auto xdiv =
                    (br.x() - this->rect().right()) / pimpl->proportion.width();
                auto ydiv = (br.y() - this->rect().bottom()) /
                            pimpl->proportion.height();
                brdiv = qMax(xdiv, ydiv);
            }

            auto bl = resultGeometry.bottomLeft();
            if (!this->rect().contains(bl.toPoint())) {
                auto xdiv =
                    (this->rect().left() - bl.x()) / pimpl->proportion.width();
                auto ydiv = (bl.y() - this->rect().bottom()) /
                            pimpl->proportion.height();
                bldiv = qMax(xdiv, ydiv);
            }

            switch (_cursorPosition) {
            case CursorPositionLeft:
            case CursorPositionTopLeft:
                if (tldiv > 0) {
                    tl.setX(tl.x() + tldiv * pimpl->proportion.width());
                    tl.setY(tl.y() + tldiv * pimpl->proportion.height());
                    resultGeometry.setTopLeft(tl);
                }
                break;
            case CursorPositionTopRight:
            case CursorPositionRight:
            case CursorPositionTop:
                if (trdiv > 0) {
                    tr.setX(tr.x() - trdiv * pimpl->proportion.width());
                    tr.setY(tr.y() + trdiv * pimpl->proportion.height());
                    resultGeometry.setTopRight(tr);
                }
                break;
            case CursorPositionBottomRight:
            case CursorPositionBottom:
                if (brdiv > 0) {
                    br.setX(br.x() - brdiv * pimpl->proportion.width());
                    br.setY(br.y() - brdiv * pimpl->proportion.height());
                    resultGeometry.setBottomRight(br);
                }
                break;
            case CursorPositionBottomLeft:
                if (bldiv > 0) {
                    bl.setX(bl.x() + bldiv * pimpl->proportion.width());
                    bl.setY(bl.y() - bldiv * pimpl->proportion.height());
                    resultGeometry.setBottomLeft(bl);
                }
                break;
            default:
                break;
            }

        } else {
            auto tl = resultGeometry.topLeft();
            if (tl.x() < this->rect().left()) {
                tl.setX(this->rect().left());
            }
            if (tl.y() < this->rect().top()) {
                tl.setY(this->rect().top());
            }
            resultGeometry.setTopLeft(tl);

            auto br = resultGeometry.bottomRight();
            if (br.x() > this->rect().right()) {
                br.setX(this->rect().right());
            }
            if (br.y() > this->rect().bottom()) {
                br.setY(this->rect().bottom());
            }
            resultGeometry.setBottomRight(br);
        }
    }

    return resultGeometry;
}

const QRectF ImageCropper::calculateGeometryWithCustomProportions(
    const QRectF &_sourceGeometry, const CursorPosition _cursorPosition,
    const QPointF &_mouseDelta) {
    QRectF resultGeometry = _sourceGeometry;
    //
    switch (_cursorPosition) {
    case CursorPositionTopLeft:
        resultGeometry.setLeft(_sourceGeometry.left() + _mouseDelta.x());
        resultGeometry.setTop(_sourceGeometry.top() + _mouseDelta.y());
        break;
    case CursorPositionTopRight:
        resultGeometry.setTop(_sourceGeometry.top() + _mouseDelta.y());
        resultGeometry.setRight(_sourceGeometry.right() + _mouseDelta.x());
        break;
    case CursorPositionBottomLeft:
        resultGeometry.setBottom(_sourceGeometry.bottom() + _mouseDelta.y());
        resultGeometry.setLeft(_sourceGeometry.left() + _mouseDelta.x());
        break;
    case CursorPositionBottomRight:
        resultGeometry.setBottom(_sourceGeometry.bottom() + _mouseDelta.y());
        resultGeometry.setRight(_sourceGeometry.right() + _mouseDelta.x());
        break;
    case CursorPositionTop:
        resultGeometry.setTop(_sourceGeometry.top() + _mouseDelta.y());
        break;
    case CursorPositionBottom:
        resultGeometry.setBottom(_sourceGeometry.bottom() + _mouseDelta.y());
        break;
    case CursorPositionLeft:
        resultGeometry.setLeft(_sourceGeometry.left() + _mouseDelta.x());
        break;
    case CursorPositionRight:
        resultGeometry.setRight(_sourceGeometry.right() + _mouseDelta.x());
        break;
    default:
        break;
    }
    //
    return resultGeometry;
}

const QRectF ImageCropper::calculateGeometryWithFixedProportions(
    const QRectF &_sourceGeometry, const CursorPosition _cursorPosition,
    const QPointF &_mouseDelta, const QSizeF &_deltas) {
    QRectF resultGeometry = _sourceGeometry;
    //
    switch (_cursorPosition) {
    case CursorPositionLeft:
        resultGeometry.setTop(_sourceGeometry.top() +
                              _mouseDelta.x() * _deltas.height());
        resultGeometry.setLeft(_sourceGeometry.left() + _mouseDelta.x());
        break;
    case CursorPositionRight:
        resultGeometry.setTop(_sourceGeometry.top() -
                              _mouseDelta.x() * _deltas.height());
        resultGeometry.setRight(_sourceGeometry.right() + _mouseDelta.x());
        break;
    case CursorPositionTop:
        resultGeometry.setTop(_sourceGeometry.top() + _mouseDelta.y());
        resultGeometry.setRight(_sourceGeometry.right() -
                                _mouseDelta.y() * _deltas.width());
        break;
    case CursorPositionBottom:
        resultGeometry.setBottom(_sourceGeometry.bottom() + _mouseDelta.y());
        resultGeometry.setRight(_sourceGeometry.right() +
                                _mouseDelta.y() * _deltas.width());
        break;
    case CursorPositionTopLeft:
        if ((_mouseDelta.x() * _deltas.height()) < (_mouseDelta.y())) {
            resultGeometry.setTop(_sourceGeometry.top() +
                                  _mouseDelta.x() * _deltas.height());
            resultGeometry.setLeft(_sourceGeometry.left() + _mouseDelta.x());
        } else {
            resultGeometry.setTop(_sourceGeometry.top() + _mouseDelta.y());
            resultGeometry.setLeft(_sourceGeometry.left() +
                                   _mouseDelta.y() * _deltas.width());
        }
        break;
    case CursorPositionTopRight:
        if ((_mouseDelta.x() * _deltas.height() * -1) < (_mouseDelta.y())) {
            resultGeometry.setTop(_sourceGeometry.top() -
                                  _mouseDelta.x() * _deltas.height());
            resultGeometry.setRight(_sourceGeometry.right() + _mouseDelta.x());
        } else {
            resultGeometry.setTop(_sourceGeometry.top() + _mouseDelta.y());
            resultGeometry.setRight(_sourceGeometry.right() -
                                    _mouseDelta.y() * _deltas.width());
        }
        break;
    case CursorPositionBottomLeft:
        if ((_mouseDelta.x() * _deltas.height()) < (_mouseDelta.y() * -1)) {
            resultGeometry.setBottom(_sourceGeometry.bottom() -
                                     _mouseDelta.x() * _deltas.height());
            resultGeometry.setLeft(_sourceGeometry.left() + _mouseDelta.x());
        } else {
            resultGeometry.setBottom(_sourceGeometry.bottom() +
                                     _mouseDelta.y());
            resultGeometry.setLeft(_sourceGeometry.left() -
                                   _mouseDelta.y() * _deltas.width());
        }
        break;
    case CursorPositionBottomRight:
        if ((_mouseDelta.x() * _deltas.height()) > (_mouseDelta.y())) {
            resultGeometry.setBottom(_sourceGeometry.bottom() +
                                     _mouseDelta.x() * _deltas.height());
            resultGeometry.setRight(_sourceGeometry.right() + _mouseDelta.x());
        } else {
            resultGeometry.setBottom(_sourceGeometry.bottom() +
                                     _mouseDelta.y());
            resultGeometry.setRight(_sourceGeometry.right() +
                                    _mouseDelta.y() * _deltas.width());
        }
        break;
    default:
        break;
    }
    //
    return resultGeometry;
}

QRectF ImageCropper::getImageRect(QSize wsize) {
    QRectF rect;

    auto scaledImageSize =
        pimpl->imageForCropping.size().scaled(wsize, Qt::KeepAspectRatio);

    rect.setSize(scaledImageSize);
    if (this->size().height() == scaledImageSize.height()) {
        auto p = QPoint((this->width() - scaledImageSize.width()) / 2, 0);
        rect.moveTo(p);
    } else {
        auto p = QPoint(0, (this->height() - scaledImageSize.height()) / 2);
        rect.moveTo(p);
    }

    return rect;
}
