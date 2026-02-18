#include "OccViewWidget.h"

#include <QApplication>
#include <QTimer>

// OpenCASCADE includes
#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_View.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>

#ifdef _WIN32
#  include <WNT_Window.hxx>
#else
#  include <Xw_Window.hxx>
#endif

OccViewWidget::OccViewWidget(QWidget* parent)
    : QWidget(parent)
{
    // Required attributes for OCCT to paint directly to the native HWND/XWindow
    setAttribute(Qt::WA_NativeWindow,       true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_PaintOnScreen,      true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(400, 400);
}

void OccViewWidget::init(const Handle(V3d_Viewer)& viewer,
                          const Handle(AIS_InteractiveContext)& context)
{
    m_viewer  = viewer;
    m_context = context;

    // Create the view
    m_view = m_viewer->CreateView();
    m_view->SetBackgroundColor(Quantity_NOC_DARKSLATEBLUE);
    m_view->SetProj(V3d_XposYnegZpos);
    m_view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.08, V3d_ZBUFFER);

    // Attach native window handle (deferred until widget is visible)
    if (isVisible() && winId()) {
        attachWindow();
    }
}

void OccViewWidget::attachWindow()
{
    if (m_view.IsNull() || m_windowAttached) return;

#ifdef _WIN32
    Handle(WNT_Window) occWin =
        new WNT_Window(reinterpret_cast<Aspect_Handle>(winId()));
#else
    Handle(Xw_Window) occWin =
        new Xw_Window(m_viewer->Driver()->GetDisplayConnection(),
                      static_cast<Window>(winId()));
#endif

    m_view->SetWindow(occWin);
    if (!occWin->IsMapped()) occWin->Map();
    m_view->MustBeResized();
    m_windowAttached = true;
}

// ---------------------------------------------------------------------------
// Qt events
// ---------------------------------------------------------------------------

void OccViewWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (!m_view.IsNull() && !m_windowAttached) {
        // 延迟一个事件循环周期，确保 HWND 已分配
        QTimer::singleShot(0, this, [this]() {
            if (!m_windowAttached) {
                attachWindow();
            }
            if (!m_view.IsNull() && m_windowAttached) {
                m_view->MustBeResized();
                m_view->Redraw();
            }
        });
    }
}

void OccViewWidget::paintEvent(QPaintEvent* /*event*/)
{
    if (m_view.IsNull()) return;
    if (!m_windowAttached) {
        attachWindow();
        if (m_windowAttached) m_view->MustBeResized();
    }
    m_view->Redraw();
}

void OccViewWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (!m_view.IsNull() && m_windowAttached) {
        m_view->MustBeResized();
    }
    // If not yet attached (first show), try now
    if (!m_windowAttached && !m_view.IsNull()) {
        attachWindow();
        if (!m_view.IsNull() && m_windowAttached) {
            m_view->MustBeResized();
        }
    }
}

void OccViewWidget::mousePressEvent(QMouseEvent* event)
{
    m_prevX = event->pos().x();
    m_prevY = event->pos().y();

    if (event->button() == Qt::LeftButton) {
        m_rotating = true;
        if (!m_view.IsNull() && m_windowAttached)
            m_view->StartRotation(m_prevX, m_prevY);
    }
    else if (event->button() == Qt::MiddleButton ||
             event->button() == Qt::RightButton) {
        m_panning = true;
    }
}

void OccViewWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_view.IsNull() || !m_windowAttached) return;

    const int x = event->pos().x();
    const int y = event->pos().y();

    if (m_rotating) {
        m_view->Rotation(x, y);
        m_view->Redraw();
    }
    else if (m_panning) {
        m_view->Pan(x - m_prevX, m_prevY - y);
        m_view->Redraw();
    }
    else {
        // Highlight detection (hover)
        m_context->MoveTo(x, y, m_view, Standard_True);
    }

    m_prevX = x;
    m_prevY = y;
}

void OccViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_rotating) {
            m_rotating = false;
        } else {
            // Click: single-select
            if (!m_view.IsNull() && m_windowAttached) {
                m_context->Select(Standard_True);
                m_view->Redraw();
            }
        }
    }
    else if (event->button() == Qt::MiddleButton ||
             event->button() == Qt::RightButton) {
        m_panning = false;
    }
}

void OccViewWidget::wheelEvent(QWheelEvent* event)
{
    if (m_view.IsNull() || !m_windowAttached) return;

    // Zoom factor: positive delta = zoom in, negative = zoom out
    const double delta = event->angleDelta().y();
    const double factor = (delta > 0) ? 1.1 : (1.0 / 1.1);
    m_view->SetZoom(factor, Standard_True);
    m_view->Redraw();
    event->accept();
}
