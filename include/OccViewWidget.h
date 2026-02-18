#ifndef OCCVIEWWIDGET_H
#define OCCVIEWWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QShowEvent>

// OpenCASCADE includes
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

/**
 * @brief Qt widget hosting an OpenCASCADE 3D view.
 *
 * Handles:
 * - Native window attachment (WNT_Window on Windows)
 * - Mouse events: rotate (LMB drag), pan (MMB/RMB drag), zoom (wheel)
 * - Selection (LMB click)
 * - Resize
 * - No Qt painting over OCC surface (paintEngine() = nullptr)
 */
class OccViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OccViewWidget(QWidget* parent = nullptr);
    ~OccViewWidget() override = default;

    /** Attach OCC viewer components. Call once after both are created. */
    void init(const Handle(V3d_Viewer)& viewer,
              const Handle(AIS_InteractiveContext)& context);

    Handle(V3d_View)                view()    const { return m_view; }
    Handle(AIS_InteractiveContext)  context() const { return m_context; }

    // Prevent Qt from painting over OCC's GL surface
    QPaintEngine* paintEngine() const override { return nullptr; }

protected:
    void showEvent(QShowEvent* event)         override;
    void paintEvent(QPaintEvent* event)       override;
    void resizeEvent(QResizeEvent* event)     override;
    void mousePressEvent(QMouseEvent* event)  override;
    void mouseMoveEvent(QMouseEvent* event)   override;
    void mouseReleaseEvent(QMouseEvent* event)override;
    void wheelEvent(QWheelEvent* event)       override;

private:
    void attachWindow();

    Handle(V3d_Viewer)               m_viewer;
    Handle(V3d_View)                 m_view;
    Handle(AIS_InteractiveContext)   m_context;

    // Mouse state
    int  m_prevX = 0;
    int  m_prevY = 0;
    bool m_rotating = false;
    bool m_panning   = false;
    bool m_zooming   = false;
    bool m_windowAttached = false;
};

#endif // OCCVIEWWIDGET_H
