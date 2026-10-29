// SPDX-License-Identifier: LGPL-3.0-or-other
// ===========================================================================
// online_jf_gui.cpp -- Qt5 GUI, fast image widgets, CLI parsing, main().
//
// Merged from:
//   onlinedisp_zmq.cpp      (FastImageView, MainWindow, display controls)
//   jf_event_finder_gui.cpp (gate parameters, counters, pedestal loading)
//
// The GUI thread never touches ROOT graphics and never touches a frame
// buffer that an
//other thread is writing: frames come from the analysis
// thread through ImageMailbox and are copied into the widgets' own storage.
// ===========================================================================
#include "online_jf.h"

#include <QComboBox>
#include <functional>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPointer>

#include <csignal>

namespace {

std::sig_atomic_t g_sigCount = 0;

void handleSigint(int) {
    // Async-signal-safe: only count.  The Qt timer performs the shutdown
    // (blocking joins are not signal-safe).  A second Ctrl-C force-quits.
    if (++g_sigCount > 1)
        std::_Exit(0);
}

// ---------------------------------------------------------------------------
// The flow chart is drawn with arrow labels placed in the layout between the
// boxes: 1. storage cell -> Pedestal -> 2. signal gate -> Gain -> ROI save.
// A stage that is switched off is hidden, so the arrows that pointed at it
// disappear and the flow reads as the bypass it is.
// ---------------------------------------------------------------------------
// The strip between the two columns.  It draws the connectors that cross from
// one column to the other as proper orthogonal arrows with a filled head, so
// the flow reads as one path instead of a few loose glyphs.
// Flow colours.  The geometry of the diagram is fixed; only the class of an
// arrow can change (see FlowStrip::setSaveMode), so a bypassed stage is greyed
// as a panel while the data flow stays readable.
enum FlowClass {
    FlowNormal = 0,  // analysis / display flow
    FlowPedestal,    // the pedestal path (incl. the tracking feedback)
    FlowData,        // the chain that ends in the saved file -> orange
    FlowOff          // a feeder that is bypassed at the moment -> pale
};
static QString flowColor(int cls) {
    switch (cls) {
    case FlowPedestal:
        return QString("#a8c8e8"); // lighter blue
    case FlowData:
        return QString("#de7d14"); // orange
    case FlowOff:
        return QString("#c9c9c9"); // bypassed: drawn, but not carrying data
    case FlowNormal:
    default:
        return QString("#5b9bd5"); // light blue
    }
}

class FlowStrip : public QWidget {
  public:
    explicit FlowStrip(QWidget *parent) : QWidget(parent) {
        setMinimumWidth(64);
        setToolTip(
            "Data flow.  Orange: everything the saved data travels through -\n"
            "frames: storage cell -> Pedestal -> Gain -> ROI -> Save, photon\n"
            "maps: ... -> Gain -> counting -> (ROI ->) Save (the maps always\n"
            "use the pedestal).  The ROI is fed by the Gain for frames and by\n"
            "the counting for the maps, always in orange: it only exists for\n"
            "the file.  Light blue: the branches that only feed the display.\n"
            "Lighter blue: the pedestal tracking pair.  The display can use the\n"
            "pedestal while the file does not (or the other way round), which is\n"
            "why both feeders into the gate funnel can be drawn at once.");
        // Fixed width for the strip, Preferred (not Expanding) height: an
        // Expanding vertical policy made the whole column row as tall as the
        // window, which left a large empty band above the save panel.
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    }
    // The endpoints are computed by MainWindow::layoutFlow() (main thread,
    // layout settled) and handed over as plain coordinates in this widget's
    // space, so painting never has to walk the widget tree.
    static const int kFunnelX = 24; // where the flow passes the gate

    struct Path {
        int yFrom = 0, yTo = 0;
        int dir = 1; // +1 left->right, -1 right->left, +2 right rail (Gain->ROI)
        bool on = true;
        int cls = FlowNormal;
        int x = 0;          // 0: default jog, else the vertical runs here
        bool head = true;    // false: the line merges into a junction
        bool startAtJog = false; // begin on the vertical, not at an edge
    };
    // Everything that depends on the stage states: which feeder carries the
    // data (pedestal or the raw bypass), which branch feeds the file, and
    // whether the gate funnel is active.  Geometry never moves.
    // `displayPede`: the display chain uses the pedestal (the Pede/Raw toggle).
    // `savePede`: the data being written does (Pedestal corrected, or photon
    // maps, which need g0).  They can differ - the usual case is a corrected
    // display with raw frames going to the file - so the two feeders into the
    // funnel can be live at the same time, and their colour says what each one
    // carries: orange for the file, light blue for the display.
    void setStages(bool displayPede, bool savePede, bool gateOn,
                   bool photonMaps) {
        const bool pedeUsed = displayPede || savePede;
        // A route that carries nothing is not drawn at all (grey was more
        // confusing than helpful).
        p1_.on = pedeUsed;  // storage cell -> Pedestal
        p2_.on = pedeUsed;  // Pedestal -> funnel
        p8_.on = !savePede; // storage cell -> funnel: the raw save route
        const int pedeCls = savePede ? FlowData : FlowNormal;
        p1_.cls = pedeCls; // storage cell -> Pedestal
        p2_.cls = pedeCls; // Pedestal -> funnel
        p3_.cls = FlowData; // funnel -> Gain: always on the route to the file
        p8_.cls = FlowData; // storage cell -> funnel (raw save)
        // The ROI only exists for the file, so its input is always orange - and
        // it comes from whichever stage produces the saved product: the Gain
        // for frames, the counting for the maps.
        p4_.cls = FlowData; // Gain -> ROI
        p4_.on = !photonMaps;
        p9_.cls = FlowData; // counting -> ROI (the maps are ROI-cropped too)
        p9_.on = photonMaps;
        p6_.cls = photonMaps ? FlowData : FlowNormal; // Gain -> counting
        funOn_ = gateOn;
        update();
    }
    void setPaths(const Path &a, const Path &b, const Path &c, const Path &d,
                  const Path &e, const Path &f, const Path &g, const Path &h,
                  const Path &i, int funnelY) {
        p1_ = a;
        p2_ = b;
        p3_ = c;
        p4_ = d;
        p5_ = e;
        p6_ = f;
        p7_ = g;
        p8_ = h;
        p9_ = i;
        funY_ = funnelY;
        update();
    }

  protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        cross(p, p1_); // 1. storage cell -> Pedestal
        cross(p, p2_); // Pedestal -> 2. signal gate
        cross(p, p3_); // 2. signal gate -> Gain
        cross(p, p4_); // Gain -> ROI, down the right-hand rail
        cross(p, p5_); // 2. signal gate -> Pedestal tracking (dropped frames)
        cross(p, p6_); // Gain -> Virtual photon counting (calibration)
        cross(p, p7_); // Virtual photon counting -> Display (the map)
        cross(p, p8_); // storage cell -> funnel: the raw (no-pede) bypass
        cross(p, p9_); // Virtual photon counting -> ROI (maps)
        drawFunnel(p); // the signal gate, drawn as the filter it is
    }

  private:
    Path p1_, p2_, p3_, p4_, p5_, p6_, p7_, p8_, p9_;
    int funY_ = -1;
    bool funOn_ = true;

    // The signal gate does not change the data, it only decides which frames
    // travel on: it is drawn as a funnel on the line, with a short blue arrow
    // from the gate panel into its side (the gate is what "gates").  With the
    // gate switched off the symbol greys out instead of disappearing.
    void drawFunnel(QPainter &p) {
        if (funY_ < 0)
            return;
        const QColor c(flowColor(funOn_ ? FlowNormal : FlowOff));
        QPen pen(c);
        pen.setWidth(2);
        p.setPen(pen);
        const int hw = 10, hh = 18;
        QPolygon f;
        f << QPoint(kFunnelX - hw, funY_ - hh / 2)
          << QPoint(kFunnelX + hw, funY_ - hh / 2)
          << QPoint(kFunnelX + hw / 3, funY_ + hh / 2)
          << QPoint(kFunnelX - hw / 3, funY_ + hh / 2);
        p.setBrush(Qt::NoBrush);
        p.drawPolygon(f);
        // the gate controls the funnel: arrow from the panel into its side
        const int tipX = kFunnelX - hw / 3 - 1;
        const int fromX = kFunnelX - hw - 14;
        p.drawLine(fromX, funY_, tipX, funY_);
        QPolygon head;
        const int s = 6;
        head << QPoint(tipX, funY_) << QPoint(tipX - s, funY_ - s)
             << QPoint(tipX - s, funY_ + s);
        p.setBrush(c);
        p.drawPolygon(head);
        p.setBrush(Qt::NoBrush);
    }

    // dir = +1 : left edge -> right edge (head on the right)
    // dir = -1 : right edge -> left edge (head on the left)
    void cross(QPainter &p, const Path &l) {
        if (!l.on)
            return;
        const int yFrom = l.yFrom, yTo = l.yTo, dir = l.dir;
        const int w = width();
        QPen pen(QColor(flowColor(l.cls)));
        pen.setWidth(2);
        p.setPen(pen);
        // The vertical part of each connector is put near the edge it ends on,
        // and never in the middle: otherwise the three connectors line up into
        // one long vertical that looks like it crosses the arrows.
        const int jog = (l.x > 0) ? l.x : ((dir > 0) ? (w - 14) : 14);
        QPolygon pts;
        if (dir == 2) { // both ends on the right: a rail beside the panels
            pts << QPoint(w, yFrom) << QPoint(w - 10, yFrom)
                << QPoint(w - 10, yTo) << QPoint(w - 2, yTo);
            p.drawPolyline(pts);
            const QPoint tip(w - 1, yTo);
            const int s2 = 7;
            QPolygon head;
            head << tip << QPoint(tip.x() - s2, tip.y() - s2)
                 << QPoint(tip.x() - s2, tip.y() + s2);
            p.setBrush(p.pen().color());
            p.drawPolygon(head);
            p.setBrush(Qt::NoBrush);
            return;
        }
        if (dir > 0) {
            if (!l.startAtJog)
                pts << QPoint(0, yFrom);
            pts << QPoint(jog, yFrom) << QPoint(jog, yTo);
            if (l.head)
                pts << QPoint(w - 8, yTo);
        } else {
            if (!l.startAtJog)
                pts << QPoint(w, yFrom);
            pts << QPoint(jog, yFrom) << QPoint(jog, yTo);
            if (l.head)
                pts << QPoint(8, yTo);
        }
        p.drawPolyline(pts);
        if (!l.head)
            return; // merged into the junction (the funnel)
        // filled head at the target end
        const QPoint tip = (dir > 0) ? QPoint(w - 1, yTo) : QPoint(1, yTo);
        QPolygon head;
        const int s = 7;
        head << tip << QPoint(tip.x() - dir * s, tip.y() - s)
             << QPoint(tip.x() - dir * s, tip.y() + s);
        p.setBrush(p.pen().color());
        p.drawPolygon(head);
        p.setBrush(Qt::NoBrush);
    }
};

// Greys a stage box (title and contents) without disabling it, so a bypassed
// stage still works the moment it is switched on again.  The widgets in
// 'keepActive' keep their normal text colour: they are the stage's own on/off
// control, and a bypassed stage must still look switchable (everything stays
// clickable either way - this is a look, not an enablement).
void greyStage(QGroupBox *g, bool grey,
               std::initializer_list<QWidget *> keepActive = {}) {
    if (!g)
        return;
    // grey text plus a tinted background, so a bypassed stage is obvious
    static const char *kGrey =
        "QGroupBox { color: #9a9a9a; background-color: #ececec; }"
        "QGroupBox::title { color: #9a9a9a; }"
        "QGroupBox QLabel { color: #9a9a9a; }"
        "QGroupBox QPushButton { color: #9a9a9a; }"
        "QGroupBox QCheckBox { color: #9a9a9a; }";
    // only restyle on a change, so the panel never flickers
    const QVariant was = g->property("flowGrey");
    if (!(was.isValid() && was.toBool() == grey)) {
        g->setProperty("flowGrey", grey);
        g->setStyleSheet(grey ? kGrey : QString());
    }
    for (QWidget *w : keepActive) {
        if (!w)
            continue;
        const QVariant wWas = w->property("flowKeep");
        if (wWas.isValid() && wWas.toBool() == grey)
            continue; // only restyle on a change, so nothing flickers
        w->setProperty("flowKeep", grey);
        // a per-widget rule wins over the group's, so the toggle stays readable
        w->setStyleSheet(grey ? "color: #000000;" : QString());
    }
}

QLabel *makeFlowArrow(const QString &glyph, QWidget *parent,
                      int cls = FlowNormal) {
    auto *l = new QLabel(glyph, parent);
    l->setAlignment(Qt::AlignCenter);
    l->setStyleSheet(QString("color: %1; font-size: 20px; font-weight: bold;")
                         .arg(flowColor(cls)));
    l->setFixedHeight(24);
    return l;
}

// recolours one of those arrows when the save mode changes
void setFlowArrowClass(QLabel *l, int cls) {
    if (l)
        l->setStyleSheet(QString("color: %1; font-size: 20px; font-weight: "
                                 "bold;")
                             .arg(flowColor(cls)));
}

// ---------------------------------------------------------------------------
// FastImageView: the original widget, with its own pixel storage so that the
// analysis thread can never hand it a buffer it is still writing.
// ---------------------------------------------------------------------------
const QRgb *makePalette() {
    static QRgb pal[256];
    for (int i = 0; i < 256; i++) {
        double v = i / 255.0;
        double r, g, b;
        if (v < 0.25) {
            r = 0;
            g = v * 4;
            b = 1;
        } else if (v < 0.5) {
            r = 0;
            g = 1;
            b = 1 - (v - 0.25) * 4;
        } else if (v < 0.75) {
            r = (v - 0.5) * 4;
            g = 1;
            b = 0;
        } else {
            r = 1;
            g = 1 - (v - 0.75) * 4;
            b = 0;
        }
        pal[i] = qRgb(int(r * 255), int(g * 255), int(b * 255));
    }
    return pal;
}
const QRgb *kPalette = makePalette();
const int MARGIN = 5;
const int SCALE_W = 50;
// The image is drawn as a framed plot: axes with tick labels *outside* the
// plot area (left/bottom), a thin frame, grid lines inside.
const int AXIS_L = 52, AXIS_B = 26, AXIS_T = 6, AXIS_R = 6;
// Deepest zoom: the smallest viewport, in image pixels.  The strip-xel build
// maps 25/15/18.75 um pixels into panels of ~1000 x 1900, so it is allowed two
// image pixels across (single-pixel inspection); the plain build stops at 8.
#ifdef JFSTRX
const double kMinViewport = 2.0;
#else
const double kMinViewport = 8.0;
#endif

class FastImageView : public QWidget {
  public:
    FastImageView(QWidget *parent = nullptr);

    // copies 'data' into widget-owned storage (GUI thread only)
    void setImageCopy(const float *data, int nx, int ny);
    // replaces the 256-entry colour table (GUI thread only)
    void setPaletteLut(const QRgb *lut);
    // log intensity mapping: values <= 0 are capped at 1e-2 before the log
    void setLogScale(bool on) {
        m_logScale = on;
        m_dirty = true;
        update();
    }
    bool logScale() const { return m_logScale; }
    // the in-pixel labels: whole numbers for photon counts, one decimal for the
    // real-valued images (ADC, keV)
    void setIntegerValues(bool on) {
        m_integerValues = on;
        update();
    }
    // histogram of the last image over its own value range (256 bins), for the
    // scale panel that replaced the in-view colour bar
    const std::vector<int> &histogram() const { return m_hist; }
    void histogramRange(double &lo, double &hi) const {
        lo = m_histLo;
        hi = m_histHi;
    }
    // called (GUI thread) when a new image or histogram is ready
    std::function<void()> onImageChanged;
    // called when the colour range changes: the strixel panels mirror the ADC
    // scale instead of keeping a stale copy of it
    std::function<void(double, double)> onColorRangeChanged;
    std::function<void(const QRgb *)> onPaletteChanged;
    // draw pixels with no counts (value <= 0) white instead of the palette's
    // lowest colour, the way ROOT's colz draws empty cells on a white canvas.
    // Used by the photon map only; the ADC/gain images keep the full palette.
    void setZeroWhite(bool on) {
        m_zeroWhite = on;
        m_dirty = true;
        update();
    }
    bool hasImage() const { return m_nx > 0 && m_ny > 0; }
    void viewport(double &x, double &y, double &w, double &h) const {
        x = m_vpX;
        y = m_vpY;
        w = vpW();
        h = vpH();
    }
    int imageWidth() const { return m_nx; }
    int imageHeight() const { return m_ny; }
    // Thin yellow rectangle drawn over the image, in DATA coordinates (data
    // row 0 is displayed at the bottom, so the view flips it here).  Used to
    // show the gate ROI, which is a snapshot and does not follow the zoom.
    void setOverlayRect(double x0, double y0, double x1, double y1, bool on) {
        m_ovX0 = x0;
        m_ovY0 = y0;
        m_ovX1 = x1;
        m_ovY1 = y1;
        m_ovOn = on;
        update();
    }

    // ---- ROI editor -------------------------------------------------------
    // While active the yellow rectangle can be dragged (inside = move, edges
    // and corners = resize, outside = draw a new one) instead of being captured
    // from the current zoom.  onRoiEdited() reports the rectangle in DATA
    // coordinates when the mouse button is released.
    void beginRoiEdit(bool on) {
        m_roiEdit = on;
        m_roiGrab = 0;
        setCursor(on ? Qt::CrossCursor : Qt::ArrowCursor);
        update();
    }
    bool roiEditActive() const { return m_roiEdit; }
    std::function<void(int x0, int y0, int x1, int y1)> onRoiEdited;
    // widget pixel -> data coordinates (x right, y up, (0,0) bottom-left)
    QPointF widgetToData(QPoint wpos) const {
        const QPointF i = widgetToImage(wpos);
        return QPointF(i.x(), double(m_ny) - i.y());
    }

    // Red receiver-ROI box, in the same DATA coordinates as the gate ROI.
    void setReceiverRoiRect(double x0, double y0, double x1, double y1,
                            bool on) {
        m_rxX0 = x0;
        m_rxY0 = y0;
        m_rxX1 = x1;
        m_rxY1 = y1;
        m_rxOn = on;
        update();
    }

    void setColorRange(double lo, double hi);
    // Horizontal size of one image pixel relative to its height (1 = square).
    // Used by the strixel panels, whose mapped pixels are 9, 25 and 16 times
    // wider than tall (group pitch X = 75*(75/Y), Y = 25/15/18.75 um).
    void setPixelAspectX(double a) {
        m_pixelAspectX = (a > 0.0) ? a : 1.0;
        update();
    }
    // the colour scale currently in use (also draggable on the colour bar)
    void colorRange(double &lo, double &hi) const {
        lo = m_clo;
        hi = m_chi;
    }
    void setViewport(double x1, double y1, double x2, double y2);
    void resetView();
    std::function<void(double, double, double, double)> onZoomChanged;

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

  private:
    void renderImage();
    void clampViewport();
    // makes the visible region the same shape as the image area of the widget
    void matchViewportToWindow();
    double vpW() const { return (m_vpW < 0) ? m_nx : m_vpW; }
    double vpH() const { return (m_vpH < 0) ? m_ny : m_vpH; }
    QRect imageRect() const;
    QRect scaleRect() const;
    double valueAt(const QPoint &pos) const;
    QPointF widgetToImage(QPoint wpos) const;

    std::vector<float> m_owned; // widget-owned copy of the last frame
    const float *m_data = nullptr;
    int m_nx = 0, m_ny = 0;
    QImage m_image;
    bool m_dirty = true;
    QRgb m_palette[256];

    double m_clo = 0, m_chi = 4000;
    double m_defClo = 0, m_defChi = 4000;
    bool m_zeroWhite = false; // photon map: no counts -> white
    bool m_showBar = false;   // the in-view colour bar (replaced by the panel)
    bool m_logScale = false;  // log intensity mapping (cap <= 0 at 1e-2)
    bool m_integerValues = false; // in-pixel labels: ints (counts) or 0.0
    double m_sx = 1.0;        // screen pixels per image pixel (last paint)
    std::vector<int> m_hist;  // histogram of the last image
    double m_histLo = 0.0, m_histHi = 1.0;
    qint64 m_histMs = 0;
    double m_pixelAspectX = 1.0; // >1: pixels wider than tall
    double m_vpX = 0, m_vpY = 0;
    double m_vpW = -1, m_vpH = -1;

    // gate ROI overlay (data coordinates)
    bool m_ovOn = false;
    double m_ovX0 = 0, m_ovY0 = 0, m_ovX1 = 0, m_ovY1 = 0;

    // ROI editor state
    bool m_roiEdit = false;
    int m_roiGrab = 0;    // 0 none, 1 move, 2 left, 3 right, 4 bottom, 5 top,
                          // 6 bl, 7 br, 8 tl, 9 tr, 10 new rectangle
    QPointF m_roiPress{0, 0};         // data coordinates at press
    double m_roiStart[4] = {0, 0, 0, 0}; // x0,y0,x1,y1 at press

    // receiver ROI overlay (RED), same data coordinates
    bool m_rxOn = false;
    double m_rxX0 = 0, m_rxY0 = 0, m_rxX1 = 0, m_rxY1 = 0;

    bool m_dragging = false;
    QPoint m_lastPos;
    double m_dragVpX, m_dragVpY;
    mutable QTransform m_widgetToImg;
    bool m_dragScale = false;

  public:
    // the Z-scale strip inside the image is being dragged: the panel must not
    // re-fill its histogram until the button comes back up
    bool isScaleDragging() const { return m_dragScale; }
    std::function<void()> onScaleReleased;

  private:
    bool m_dragMax = false; // true = dragging max, false = dragging min
    int m_dragScaleY = 0;
    double m_dragClo, m_dragChi;
};

FastImageView::FastImageView(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(200, 200);
    m_chi = 4000;
    m_defChi = 4000;
    m_clo = 0;
    m_defClo = 0;
    for (int i = 0; i < 256; i++)
        m_palette[i] = kPalette[i];
}

void FastImageView::setImageCopy(const float *data, int nx, int ny) {
    if (!data || nx <= 0 || ny <= 0)
        return;
    const size_t n = static_cast<size_t>(nx) * static_cast<size_t>(ny);
    if (m_owned.size() != n)
        m_owned.resize(n);
    memcpy(m_owned.data(), data, n * sizeof(float));
    m_data = m_owned.data();
    m_nx = nx;
    m_ny = ny;
    m_dirty = true;
    if (m_vpW < 0)
        m_vpW = nx;
    if (m_vpH < 0)
        m_vpH = ny;
    update();
}

void FastImageView::setPaletteLut(const QRgb *lut) {
    if (onPaletteChanged && lut)
        onPaletteChanged(lut);
    if (!lut)
        return;
    for (int i = 0; i < 256; i++)
        m_palette[i] = lut[i];
    m_dirty = true;
    update();
}

void FastImageView::setColorRange(double lo, double hi) {
    m_clo = lo;
    m_chi = hi;
    m_defClo = lo;
    m_defChi = hi;
    m_histMs = 0; // the bins follow the window: recompute on the next render
    m_dirty = true;
    if (onColorRangeChanged)
        onColorRangeChanged(lo, hi);
    update();
}

void FastImageView::setViewport(double x1, double y1, double x2, double y2) {
    if (x2 - x1 < kMinViewport || y2 - y1 < kMinViewport)
        return;
    m_vpX = x1;
    m_vpY = y1;
    m_vpW = x2 - x1;
    m_vpH = y2 - y1;
    update();
}

void FastImageView::resetView() {
    if (!hasImage())
        return; // nothing to reset yet (and (0,0,0,0) would collapse the map)
    m_vpX = 0;
    m_vpY = 0;
    m_vpW = m_nx;
    m_vpH = m_ny;
    if (onZoomChanged)
        onZoomChanged(0, 0, m_nx, m_ny);
    update();
}

QRect FastImageView::imageRect() const {
    int x0 = AXIS_L, y0 = AXIS_T;
    int x1 = width() - AXIS_R - (m_showBar ? SCALE_W : 0);
    int y1 = height() - AXIS_B;
    if (x1 - x0 < 10)
        x1 = x0 + 10;
    if (y1 - y0 < 10)
        y1 = y0 + 10;
    return QRect(x0, y0, x1 - x0, y1 - y0);
}

QRect FastImageView::scaleRect() const {
    if (!m_showBar)
        return QRect(); // the scale panel took its place
    QRect ir = imageRect();
    return QRect(ir.right() + 4, ir.top(), SCALE_W - 4, ir.height());
}

// keeps the viewport inside the image; used after pan, zoom and resize
void FastImageView::clampViewport() {
    if (!hasImage())
        return;
    if (m_vpW > m_nx)
        m_vpW = m_nx;
    if (m_vpH > m_ny)
        m_vpH = m_ny;
    if (m_vpX < 0)
        m_vpX = 0;
    if (m_vpX + vpW() > m_nx)
        m_vpX = m_nx - vpW();
    if (m_vpY < 0)
        m_vpY = 0;
    if (m_vpY + vpH() > m_ny)
        m_vpY = m_ny - vpH();
}

// The ADC window is the tool that selects the gate ROI, so the region it
// shows takes the shape of the window: make the window tall and "ROI only"
// captures a narrow, tall region.  The shorter side is grown so that nothing
// the operator was looking at disappears; the whole-frame view is left alone
// (it is letterboxed on purpose) so that it still captures the full sensor.
void FastImageView::matchViewportToWindow() {
    if (!hasImage())
        return;
    if (m_vpW >= m_nx - 0.5 && m_vpH >= m_ny - 0.5)
        return; // whole frame: keep it, whatever the window shape
    QRect ir = imageRect();
    if (ir.width() <= 0 || ir.height() <= 0)
        return;
    // shape the viewport so the physical pixels (m_pixelAspectX) fill the
    // window with the same shape, not the square-pixel shape
    const double ar = double(ir.width()) / double(ir.height()) / m_pixelAspectX;
    double w = vpW(), h = vpH();
    if (w / h > ar)
        h = w / ar; // too wide -> grow vertically
    else
        w = h * ar; // too tall -> grow horizontally
    if (w > m_nx) { // grown past the sensor: shrink both, keep the shape
        h *= m_nx / w;
        w = m_nx;
    }
    if (h > m_ny) {
        w *= m_ny / h;
        h = m_ny;
    }
    const double cx = m_vpX + vpW() / 2, cy = m_vpY + vpH() / 2;
    m_vpW = w;
    m_vpH = h;
    m_vpX = cx - w / 2;
    m_vpY = cy - h / 2;
    clampViewport();
    if (onZoomChanged)
        onZoomChanged(m_vpX, m_vpY, m_vpX + vpW(), m_vpY + vpH());
}

void FastImageView::resizeEvent(QResizeEvent *) {
    matchViewportToWindow();
    update();
}

void FastImageView::renderImage() {    if (!m_data || m_nx <= 0 || m_ny <= 0)
        return;
    if (m_image.size() != QSize(m_nx, m_ny))
        m_image = QImage(m_nx, m_ny, QImage::Format_ARGB32);
    // log: values <= 0 are capped at 1e-2 (log10 = -2), so an empty pixel is
    // still distinguishable instead of being -inf
    const double loD = m_logScale ? std::log10(std::max(m_clo, 1e-2)) : m_clo;
    const double hiD = m_logScale ? std::log10(std::max(m_chi, 1e-2)) : m_chi;
    const double range = (hiD > loD) ? (hiD - loD) : 1.0;
    static const QRgb kWhite = qRgb(255, 255, 255);
    double dmin = 0.0, dmax = 1.0;
    bool first = true;
    for (int p = 0; p < m_nx * m_ny; p++) {
        const double v = m_data[p];
        if (std::isfinite(v)) {
            if (first) {
                dmin = dmax = v;
                first = false;
            } else {
                if (v < dmin)
                    dmin = v;
                if (v > dmax)
                    dmax = v;
            }
        }
        if (m_zeroWhite && !(v > 0.0)) {
            // empty cell: white background, as the ROOT colz plot showed it
            m_image.setPixel(p % m_nx, m_ny - 1 - p / m_nx, kWhite);
            continue;
        }
        const double vv = m_logScale ? std::log10(std::max(v, 1e-2)) : v;
        int idx = int((vv - loD) / range * 255);
        if (idx < 0)
            idx = 0;
        if (idx > 255)
            idx = 255;
        m_image.setPixel(p % m_nx, m_ny - 1 - p / m_nx, m_palette[idx]);
    }
    m_dirty = false;

    // Histogram for the scale panel: walking the whole frame is cheap enough at
    // the image cadence, but the panel only needs the shape, so a few times a
    // second is plenty.
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_hist.empty() || now - m_histMs > 300) {
        m_histMs = now;
        if (first || dmax <= dmin)
            dmax = dmin + 1.0;
        // Bin over the range the strip actually shows (the colour range plus
        // half a range either side), not over the image's whole value range: a
        // photon map accumulates to thousands of counts while its scale shows
        // 0..12, so full-range binning put the entire visible window inside one
        // bin - the "too big bins".  Pixels outside the window are skipped
        // instead of being piled into the edge bins.
        const double caspan = (m_chi > m_clo) ? (m_chi - m_clo) : 0.0;
        double hlo = dmin, hhi = dmax;
        if (caspan > 0.0 && std::isfinite(m_clo) && std::isfinite(m_chi)) {
            hlo = m_clo - 0.5 * caspan;
            hhi = m_chi + 0.5 * caspan;
        }
        if (m_logScale) {
            hlo = std::log10(std::max(hlo, 1e-2));
            hhi = std::log10(std::max(hhi, 1e-2));
            if (hhi <= hlo)
                hhi = hlo + 1.0;
        }
        if (!(hhi > hlo))
            hhi = hlo + 1.0;
        m_histLo = hlo;
        m_histHi = hhi;
        m_hist.assign(256, 0);
        const double sc = 255.0 / (hhi - hlo);
        for (int p = 0; p < m_nx * m_ny; p++) {
            const double raw = m_data[p];
            if (!std::isfinite(raw))
                continue;
            const double v = m_logScale ? std::log10(std::max(raw, 1e-2)) : raw;
            if (v < hlo || v > hhi)
                continue; // outside the visible window
            int b = int((v - hlo) * sc);
            if (b < 0)
                b = 0;
            if (b > 255)
                b = 255;
            m_hist[b]++;
        }
        if (onImageChanged)
            onImageChanged();
    }
}

void FastImageView::paintEvent(QPaintEvent *) {
    if (m_dirty)
        renderImage();
    QPainter p(this);
    p.fillRect(rect(), Qt::black);
    if (!m_data || m_nx <= 0 || m_ny <= 0)
        return;

    QRect ir = imageRect();
    // Pixel aspect: a value > 1 makes each image pixel that many times wider
    // than tall (the strixel groups have rectangular pixels), so the fit is
    // computed in physical units and the image is scaled non-uniformly.
    double sx = 0.0, sy = 0.0;
    if (m_pixelAspectX != 1.0) {
        const double fitY = double(ir.height()) / vpH();
        const double fitX = double(ir.width()) / (vpW() * m_pixelAspectX);
        sy = (fitY < fitX) ? fitY : fitX;
        sx = sy * m_pixelAspectX;
    } else {
        const double fitX = double(ir.width()) / vpW();
        const double fitY = double(ir.height()) / vpH();
        sx = sy = (fitX < fitY) ? fitX : fitY;
    }

    p.save();
    // Zoomed in, the scaled image is larger than the plot area: without this
    // clip it spills over the frame and the tick labels.
    p.setClipRect(ir);
    p.translate(ir.left(), ir.top());
    p.translate((ir.width() - vpW() * sx) * 0.5,
                (ir.height() - vpH() * sy) * 0.5);
    p.translate(-m_vpX * sx, -m_vpY * sy);
    p.scale(sx, sy);
    const QTransform imgToWidget = p.combinedTransform();
    m_widgetToImg = imgToWidget.inverted();
    m_sx = sx; // screen pixels per image pixel: drives the pixel grid/labels
    p.drawImage(QPoint(0, 0), m_image);
    p.restore();

    // the overlays live in the same plot area: clip them as well
    p.save();
    p.setClipRect(ir);
    // gate ROI overlay: convert data rows (row 0 = bottom) to display space
    if (m_ovOn && m_ovX1 > m_ovX0 && m_ovY1 > m_ovY0) {
        const double dx = m_ovX0;
        const double dy = double(m_ny) - m_ovY1;
        const double dw = m_ovX1 - m_ovX0;
        const double dh = m_ovY1 - m_ovY0;
        const QRectF r = imgToWidget.mapRect(QRectF(dx, dy, dw, dh));
        QPen pen(Qt::yellow);
        pen.setWidth(m_roiEdit ? 2 : 1);
        pen.setCosmetic(true);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawRect(r);
        if (m_roiEdit) { // corner and edge handles: what can be grabbed
            const double hw = 3.0; // half handle size, in widget pixels
            const double xs[3] = {r.left(), r.center().x(), r.right()};
            const double ys[3] = {r.top(), r.center().y(), r.bottom()};
            p.setBrush(QColor(255, 255, 80));
            for (int iy = 0; iy < 3; iy++) {
                for (int ix = 0; ix < 3; ix++) {
                    if (ix == 1 && iy == 1)
                        continue; // centre handle would only be confusing
                    p.drawRect(QRectF(xs[ix] - hw, ys[iy] - hw, 2 * hw,
                                      2 * hw));
                }
            }
        }
    }

    // receiver ROI overlay (RED), from the "rx_roi" field of the stream
    // header.  The ADC image is the raw layout, so this is a plain rectangle.
    if (m_rxOn && m_rxX1 > m_rxX0 && m_rxY1 > m_rxY0) {
        QPen pen(Qt::red);
        pen.setWidth(1);
        pen.setCosmetic(true);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        const double dx = m_rxX0;
        const double dy = double(m_ny) - m_rxY1;
        const double dw = m_rxX1 - m_rxX0;
        const double dh = m_rxY1 - m_rxY0;
        p.drawRect(imgToWidget.mapRect(QRectF(dx, dy, dw, dh)));
    }
    p.restore(); // back to the full widget for the axes

    // ── axes: a frame around the plot, ticks and labels outside, grid inside ──
    {
        QFont f = p.font();
        f.setPixelSize(10);
        p.setFont(f);
        // 0..8 grid lines, on 1/2/5 steps
        auto step = [](double span) {
            static const double cand[] = {1,   2,   5,    10,   20,  50,
                                          100, 200, 500,  1000, 2000, 5000};
            for (double c : cand)
                if (span / c <= 8.0)
                    return c;
            return 10000.0;
        };
        const double x0 = m_vpX, x1 = m_vpX + vpW();
        const double y0 = m_vpY, y1 = m_vpY + vpH(); // display rows (top down)
        const double sxStep = step(x1 - x0), syStep = step(y1 - y0);
        const QPen grid(QColor(95, 95, 95));
        const QPen tick(QColor(215, 215, 215));
        for (double gx = std::ceil(x0 / sxStep) * sxStep; gx <= x1;
             gx += sxStep) {
            const double wx = imgToWidget.map(QPointF(gx, 0)).x();
            QPen g2 = grid;
            g2.setStyle(Qt::DotLine);
            p.setPen(g2);
            p.drawLine(QPointF(wx, ir.top()), QPointF(wx, ir.bottom()));
            p.setPen(tick);
            p.drawLine(QPointF(wx, ir.bottom() + 1), QPointF(wx, ir.bottom() + 4));
            p.drawText(QRectF(wx - 26, ir.bottom() + 5, 52, 14),
                       Qt::AlignHCenter | Qt::AlignTop,
                       QString::number(gx, 'f', 0));
        }
        for (double gy = std::ceil(y0 / syStep) * syStep; gy <= y1;
             gy += syStep) {
            const double wy = imgToWidget.map(QPointF(0, gy)).y();
            QPen g2 = grid;
            g2.setStyle(Qt::DotLine);
            p.setPen(g2);
            p.drawLine(QPointF(ir.left(), wy), QPointF(ir.right(), wy));
            p.setPen(tick);
            p.drawLine(QPointF(ir.left() - 4, wy), QPointF(ir.left() - 1, wy));
            // the label is the DATA row (row 0 at the bottom), as everywhere else
            p.drawText(QRectF(ir.left() - 50, wy - 7, 44, 14),
                       Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(double(m_ny) - 1.0 - gy, 'f', 0));
        }
        p.setPen(QPen(QColor(225, 225, 225)));
        p.setBrush(Qt::NoBrush);
        p.drawRect(ir.adjusted(0, 0, -1, -1));
    }

    // ── pixel grid and in-pixel values, once a pixel is big enough ──────────
    if (m_sx >= 16.0) {
        const double x0 = m_vpX, x1 = m_vpX + vpW();
        const double y0 = m_vpY, y1 = m_vpY + vpH();
        // thin outline around every pixel (only when a sane number is visible)
        if (vpW() <= 256 && vpH() <= 256) {
            QPen px(QColor(55, 55, 55));
            px.setCosmetic(true);
            p.setPen(px);
            for (double gx = std::floor(x0); gx <= x1 + 1; gx += 1.0) {
                const double wx = imgToWidget.map(QPointF(gx, 0)).x();
                p.drawLine(QPointF(wx, ir.top()), QPointF(wx, ir.bottom()));
            }
            for (double gy = std::floor(y0); gy <= y1 + 1; gy += 1.0) {
                const double wy = imgToWidget.map(QPointF(0, gy)).y();
                p.drawLine(QPointF(ir.left(), wy), QPointF(ir.right(), wy));
            }
        }
        if (m_sx >= 50.0 && vpW() <= 64 && vpH() <= 64) {
            QFont f = p.font();
            // The label has to fit *inside* the pixel: six characters for the
            // real-valued images ("XXXXX.y"), five for the photon counts, at
            // roughly 0.62 em per digit.  The 0.30 em cap still gives the
            // "small, tidy number" look at deep zoom.
            const int maxChars = m_integerValues ? 5 : 6;
            const double bySize = m_sx * 0.30;
            const double byWidth = m_sx * 0.92 / (0.62 * maxChars);
            f.setPixelSize(std::max(
                7, std::min(20, int(std::min(bySize, byWidth)))));
            p.setFont(f);
            for (int gy = int(std::floor(y0)); gy <= int(y1); gy++) {
                for (int gx = int(std::floor(x0)); gx <= int(x1); gx++) {
                    if (gx < 0 || gy < 0 || gx >= m_nx || gy >= m_ny)
                        continue;
                    const double v = m_data[gx + (m_ny - 1 - gy) * m_nx];
                    if (!std::isfinite(v))
                        continue;
                    // text colour from the pixel that was actually painted
                    const QRgb c = m_image.pixel(gx, gy);
                    const int lum = (qRed(c) * 299 + qGreen(c) * 587 +
                                     qBlue(c) * 114) /
                                    1000;
                    p.setPen(lum > 140 ? QColor(15, 15, 15)
                                       : QColor(245, 245, 245));
                    const QString txt =
                        m_integerValues
                            ? QString::number(qint64(std::llround(v)))
                            : QString::number(v, 'f', 1);
                    const QPointF ctr =
                        imgToWidget.map(QPointF(gx + 0.5, gy + 0.5));
                    p.drawText(QRectF(ctr.x() - m_sx * 0.5,
                                      ctr.y() - m_sx * 0.5, m_sx, m_sx),
                               Qt::AlignCenter, txt);
                }
            }
        }
    }

    // in-view colour bar: kept for builds that still ask for it (the scale
    // panel replaced it in the windows)
    if (!m_showBar)
        return;
    QRect sr = scaleRect();
    for (int yy = sr.top(); yy <= sr.bottom(); yy++) {
        double frac = 1.0 - double(yy - sr.top()) / sr.height();
        int idx = int(frac * 255);
        if (idx < 0)
            idx = 0;
        if (idx > 255)
            idx = 255;
        p.setPen(m_palette[idx]);
        p.drawLine(sr.left(), yy, sr.right(), yy);
    }
    p.setPen(Qt::white);
    p.drawRect(sr.adjusted(-1, -1, 1, 1));
    QFont f = p.font();
    f.setPixelSize(10);
    p.setFont(f);
    p.drawText(sr.left(), sr.top() + 1, sr.width(), 12, Qt::AlignCenter,
               QString::number(m_chi, 'f', 0));
    p.drawText(sr.left(), sr.bottom() - 11, sr.width(), 12, Qt::AlignCenter,
               QString::number(m_clo, 'f', 0));
}

void FastImageView::mousePressEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton)
        return;
    QPoint ip = e->pos();
    if (m_roiEdit && hasImage()) {
        // which part of the yellow rectangle was grabbed?
        const QPointF d = widgetToData(ip);
        const QPointF d2 = widgetToData(ip + QPoint(8, 0));
        const double tol = std::max(1e-9, std::abs(d2.x() - d.x()));
        const bool nearL = std::abs(d.x() - m_ovX0) <= tol;
        const bool nearR = std::abs(d.x() - m_ovX1) <= tol;
        const bool nearB = std::abs(d.y() - m_ovY0) <= tol;
        const bool nearT = std::abs(d.y() - m_ovY1) <= tol;
        const bool inX = d.x() >= m_ovX0 - tol && d.x() <= m_ovX1 + tol;
        const bool inY = d.y() >= m_ovY0 - tol && d.y() <= m_ovY1 + tol;
        m_roiGrab = 0;
        if (nearL && nearB)
            m_roiGrab = 6;
        else if (nearR && nearB)
            m_roiGrab = 7;
        else if (nearL && nearT)
            m_roiGrab = 8;
        else if (nearR && nearT)
            m_roiGrab = 9;
        else if (nearL && inY)
            m_roiGrab = 2;
        else if (nearR && inY)
            m_roiGrab = 3;
        else if (nearB && inX)
            m_roiGrab = 4;
        else if (nearT && inX)
            m_roiGrab = 5;
        else if (inX && inY)
            m_roiGrab = 1;
        else
            m_roiGrab = 10; // start a new rectangle at the cursor
        m_roiPress = d;
        m_roiStart[0] = m_ovX0;
        m_roiStart[1] = m_ovY0;
        m_roiStart[2] = m_ovX1;
        m_roiStart[3] = m_ovY1;
        if (m_roiGrab == 10) {
            m_ovX0 = m_ovX1 = std::max(0.0, std::min(double(m_nx), d.x()));
            m_ovY0 = m_ovY1 = std::max(0.0, std::min(double(m_ny), d.y()));
            m_ovOn = true;
        }
        update();
        return;
    }
    QRect sr = scaleRect();
    if (sr.contains(ip)) {
        m_dragScale = true;
        m_dragScaleY = ip.y();
        m_dragClo = m_clo;
        m_dragChi = m_chi;
        m_dragMax = (ip.y() < sr.top() + sr.height() / 2);
        setCursor(Qt::SizeVerCursor);
        return;
    }
    m_dragging = true;
    m_lastPos = ip;
    m_dragVpX = m_vpX;
    m_dragVpY = m_vpY;
    setCursor(Qt::ClosedHandCursor);
}

void FastImageView::mouseMoveEvent(QMouseEvent *e) {
    if (m_roiEdit && hasImage()) {
        const QPointF d = widgetToData(e->pos());
        const double x = std::max(0.0, std::min(double(m_nx), d.x()));
        const double y = std::max(0.0, std::min(double(m_ny), d.y()));
        constexpr double kMin = 2.0; // a ROI smaller than this is unusable
        if (m_roiGrab == 10) { // dragging out a new rectangle
            m_ovX0 = std::min(m_roiPress.x(), x);
            m_ovX1 = std::max(m_roiPress.x(), x);
            m_ovY0 = std::min(m_roiPress.y(), y);
            m_ovY1 = std::max(m_roiPress.y(), y);
            m_ovOn = true;
        } else if (m_roiGrab == 1) { // move the whole rectangle
            const double dx = x - m_roiPress.x();
            const double dy = y - m_roiPress.y();
            double x0 = m_roiStart[0] + dx, x1 = m_roiStart[2] + dx;
            double y0 = m_roiStart[1] + dy, y1 = m_roiStart[3] + dy;
            if (x0 < 0) { x1 -= x0; x0 = 0; }
            if (x1 > m_nx) { x0 -= (x1 - m_nx); x1 = m_nx; }
            if (y0 < 0) { y1 -= y0; y0 = 0; }
            if (y1 > m_ny) { y0 -= (y1 - m_ny); y1 = m_ny; }
            m_ovX0 = x0;
            m_ovX1 = x1;
            m_ovY0 = y0;
            m_ovY1 = y1;
        } else if (m_roiGrab != 0) { // one edge or one corner
            double x0 = m_roiStart[0], y0 = m_roiStart[1];
            double x1 = m_roiStart[2], y1 = m_roiStart[3];
            const bool left = (m_roiGrab == 2 || m_roiGrab == 6 || m_roiGrab == 8);
            const bool right = (m_roiGrab == 3 || m_roiGrab == 7 || m_roiGrab == 9);
            const bool bottom = (m_roiGrab == 4 || m_roiGrab == 6 || m_roiGrab == 7);
            const bool top = (m_roiGrab == 5 || m_roiGrab == 8 || m_roiGrab == 9);
            if (left && x <= x1 - kMin)
                x0 = x;
            if (right && x >= x0 + kMin)
                x1 = x;
            if (bottom && y <= y1 - kMin)
                y0 = y;
            if (top && y >= y0 + kMin)
                y1 = y;
            m_ovX0 = x0;
            m_ovY0 = y0;
            m_ovX1 = x1;
            m_ovY1 = y1;
        }
        update();
        if (m_roiGrab == 0) { // hovering: show what grabbing here would do
            const QPointF dd = widgetToData(e->pos());
            const QPointF dd2 = widgetToData(e->pos() + QPoint(8, 0));
            const double tol = std::max(1e-9, std::abs(dd2.x() - dd.x()));
            const bool nearL = std::abs(dd.x() - m_ovX0) <= tol;
            const bool nearR = std::abs(dd.x() - m_ovX1) <= tol;
            const bool nearB = std::abs(dd.y() - m_ovY0) <= tol;
            const bool nearT = std::abs(dd.y() - m_ovY1) <= tol;
            const bool inside = dd.x() >= m_ovX0 - tol &&
                                dd.x() <= m_ovX1 + tol &&
                                dd.y() >= m_ovY0 - tol &&
                                dd.y() <= m_ovY1 + tol;
            if ((nearL && nearT) || (nearR && nearB))
                setCursor(Qt::SizeFDiagCursor);
            else if ((nearL && nearB) || (nearR && nearT))
                setCursor(Qt::SizeBDiagCursor);
            else if (nearL || nearR)
                setCursor(Qt::SizeHorCursor);
            else if (nearT || nearB)
                setCursor(Qt::SizeVerCursor);
            else
                setCursor(inside ? Qt::OpenHandCursor : Qt::CrossCursor);
        }
        return;
    }
    if (m_dragScale) {
        QRect sr = scaleRect();
        double delta = double(e->y() - m_dragScaleY) / double(sr.height()) *
                       (m_dragChi - m_dragClo);
        if (m_dragMax) {
            m_chi = m_dragChi - delta;
            if (m_chi < m_clo + 1)
                m_chi = m_clo + 1;
        } else {
            m_clo = m_dragClo - delta;
            if (m_clo > m_chi - 1)
                m_clo = m_chi - 1;
        }
        m_dirty = true;
        update();
        return;
    }
    if (!m_dragging) {
        QPoint hp = e->pos();
        QRect ir2 = imageRect();
        QRect sr = scaleRect();
        if (sr.contains(hp)) {
            setCursor(Qt::SizeVerCursor);
            double v = valueAt(hp);
            setToolTip(QString("Scale: %1").arg(v, 0, 'f', 0));
        } else if (ir2.contains(hp)) {
            setCursor(Qt::OpenHandCursor);
            // widgetToImage() is in QImage space (row 0 = TOP); the reported
            // pixel and the value are converted to DATA rows so that the
            // numbering matches the ROOT plots and the ROI list: (0,0) is the
            // bottom-left pixel everywhere.
            QPointF ipt = widgetToImage(hp);
            const int ix = int(ipt.x());
            const int dy = int(ipt.y());
            const int iy = m_ny - 1 - dy;
            if (ix >= 0 && ix < m_nx && iy >= 0 && iy < m_ny)
                setToolTip(QString("(x=%1, y=%2) = %3")
                               .arg(ix)
                               .arg(iy)
                               .arg(m_data[ix + iy * m_nx], 0, 'f', 1));
        } else {
            setCursor(Qt::ArrowCursor);
            setToolTip(QString());
        }
        return;
    }
    QPointF now = widgetToImage(e->pos());
    QPointF then = widgetToImage(m_lastPos);
    m_vpX = m_dragVpX - (now.x() - then.x());
    m_vpY = m_dragVpY - (now.y() - then.y());
    clampViewport();
    update();
    // follow live: the gain view and the photon map pan with the ADC view
    if (onZoomChanged)
        onZoomChanged(m_vpX, m_vpY, m_vpX + vpW(), m_vpY + vpH());
}

void FastImageView::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton && m_roiGrab != 0) {
        m_roiGrab = 0;
        if (onRoiEdited && m_ovX1 - m_ovX0 >= 2.0 && m_ovY1 - m_ovY0 >= 2.0)
            onRoiEdited(int(std::lround(m_ovX0)), int(std::lround(m_ovY0)),
                        int(std::lround(m_ovX1)), int(std::lround(m_ovY1)));
        return;
    }
    if (e->button() == Qt::LeftButton) {
        if (m_dragging && onZoomChanged)
            onZoomChanged(m_vpX, m_vpY, m_vpX + vpW(), m_vpY + vpH());
        m_dragging = false;
        const bool wasScale = m_dragScale;
        m_dragScale = false;
        setCursor(Qt::ArrowCursor);
        if (wasScale && onScaleReleased)
            onScaleReleased(); // one histogram/scale refresh, after the drag
    }
}

void FastImageView::wheelEvent(QWheelEvent *e) {
    if (!m_data || m_nx <= 0 || m_ny <= 0)
        return;
    QPointF imgPt = widgetToImage(e->pos());
    double mx = imgPt.x(), my = imgPt.y();

    double factor = (e->angleDelta().y() > 0) ? 1.0 / 1.18 : 1.18;
    double nw = vpW() * factor, nh = vpH() * factor;
    if (nw < kMinViewport)
        nw = kMinViewport;
    if (nh < kMinViewport)
        nh = kMinViewport;
    if (nw > m_nx)
        nw = m_nx;
    if (nh > m_ny)
        nh = m_ny;

    double fracX = (mx - m_vpX) / vpW();
    double fracY = (my - m_vpY) / vpH();
    m_vpX = mx - fracX * nw;
    m_vpY = my - fracY * nh;
    m_vpW = nw;
    m_vpH = nh;
    if (factor > 1.0) {
        // Zooming OUT: do not reshape to the window here.  matchViewportToWindow
        // grows/shrinks to the window aspect, so the image dimension that hits
        // its limit first froze the other one and the whole frame could never
        // be shown.  Let both grow to the image bounds; the window-shape rule
        // still applies while zooming IN and on resize.
        clampViewport();
    } else {
        // keep the visible region the same shape as the window also while
        // zooming, so that the captured ROI has the shape of the window
        matchViewportToWindow();
    }

    if (onZoomChanged)
        onZoomChanged(m_vpX, m_vpY, m_vpX + vpW(), m_vpY + vpH());
    setToolTip(QString("Zoom: %1%").arg(int(100.0 * m_nx / vpW())));
    update();
}

void FastImageView::mouseDoubleClickEvent(QMouseEvent *e) {
    QRect sr = scaleRect();
    if (sr.contains(e->pos())) {
        m_clo = m_defClo;
        m_chi = m_defChi;
        m_dirty = true;
        update();
    } else {
        resetView();
    }
}

double FastImageView::valueAt(const QPoint &pos) const {
    QRect sr = scaleRect();
    if (!sr.contains(pos))
        return m_clo - 1;
    double frac = 1.0 - double(pos.y() - sr.top()) / sr.height();
    return m_clo + frac * (m_chi - m_clo);
}

QPointF FastImageView::widgetToImage(QPoint wpos) const {
    return m_widgetToImg.map(QPointF(wpos));
}

QLabel *makeValueLabel(const QString &text, QWidget *parent) {
    QLabel *label = new QLabel(text, parent);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}

} // namespace

// ---------------------------------------------------------------------------
// MainWindow
// ---------------------------------------------------------------------------
// The histogram the scale panel shows: the image's own distribution with the
// currently mapped range drawn over it, so the effect of the two ends is
// visible while they are set.
// The histogram strip of the scale panel.  It is deliberately thin (a fixed
// ~38 px) and the two range ends are drawn at FIXED positions - a quarter and
// three quarters of the width - so the *value axis* moves under them: the data
// window shown is the selected range widened by half a range on either side.
// Dragging a marker changes that end of the range; the histogram rescales so
// the marker returns to its fixed spot.  The horizontal axis follows the image
// mapping (lin or log), the bar heights have their own lin/log switch.
class ScaleHisto : public QWidget {
  public:
    explicit ScaleHisto(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedHeight(38);
        setMouseTracking(true);
        setToolTip("Distribution of the last image (its own value range); the\n"
                   "band between the two markers is the range mapped to the\n"
                   "palette.  Markers sit at 1/4 and 3/4 and the axis rescales\n"
                   "under them - drag either one to change that end.");
    }
    // dlo/dhi: value range of the bins; clo/chi: the mapped range
    void setData(const std::vector<int> *h, double dlo, double dhi, double clo,
                 double chi, bool logX, bool logY) {
        // Frozen while a marker is being dragged: re-filling the bars from the
        // live image mid-drag fed the drag back into the histogram (the bars
        // moved, which moved the marker, which moved the bars...).  The strip
        // is refreshed once, when the button comes back up.
        if (dragging_)
            return;
        h_ = h;
        dlo_ = dlo;
        dhi_ = dhi;
        clo_ = clo;
        chi_ = chi;
        logX_ = logX;
        logY_ = logY;
        update();
    }
    // dragging a marker reports the new range (GUI thread)
    std::function<void(double, double)> onRangeChanged;

  protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.fillRect(rect(), QColor(28, 28, 28));
        const int W = width(), H = height();
        // While a marker is dragged the axis stays exactly as it was when the
        // drag started: the bars (and the two numbers) must not move with the
        // cursor, only the dragged end is previewed as a line.  Releasing
        // rebuilds the axis from the new range.
        const double dclo = dragging_ ? preClo_ : clo_;
        const double dchi = dragging_ ? preChi_ : chi_;
        // the shown window is the selected range +- half a range
        const double span = (dchi > dclo) ? (dchi - dclo) : 1.0;
        double wlo = dclo - 0.5 * span, whi = dchi + 0.5 * span;
        if (logX_) { // work in log space, capping <= 0 at 1e-2
            wlo = std::log10(std::max(wlo, 1e-2));
            whi = std::log10(std::max(whi, 1e-2));
            if (whi <= wlo)
                whi = wlo + 1.0;
        }
        auto mapv = [&](double v) {
            return logX_ ? std::log10(std::max(v, 1e-2)) : v;
        };
        auto xof = [&](double v) {
            return (mapv(v) - wlo) / (whi - wlo) * W;
        };
        // The image keeps arriving while the marker is dragged and its
        // histogram is recomputed on every repaint, so the bars themselves were
        // still moving under the frozen axis.  While dragging, paint from the
        // snapshot taken when the button went down.
        const std::vector<int> *hv =
            (dragging_ && !snap_.empty()) ? &snap_ : h_;
        if (hv && !hv->empty() && dhi_ > dlo_) {
            int mx = 1;
            for (int v : *hv)
                if (v > mx)
                    mx = v;
            const double lmx = std::log10(double(mx) + 1.0);
            const double binW = (dhi_ - dlo_) / double(hv->size());
            QBrush bar(QColor(120, 190, 255));
            p.setPen(Qt::NoPen);
            p.setBrush(bar);
            for (size_t i = 0; i < hv->size(); i++) {
                if ((*hv)[i] <= 0)
                    continue;
                // solid: the bar is as wide as the bin spacing on screen
                const double x0 = xof(dlo_ + double(i) * binW);
                const double x1 = xof(dlo_ + double(i + 1) * binW);
                const double bh =
                    logY_ ? (std::log10(double((*hv)[i]) + 1.0) / lmx)
                          : (double((*hv)[i]) / double(mx));
                const double w = std::max(1.0, x1 - x0);
                if (x0 > W || x0 + w < 0)
                    continue;
                p.drawRect(QRectF(x0, H - 3 - bh * (H - 6), w,
                                  std::max(1.0, bh * (H - 6))));
            }
        }
        // the two markers at their fixed places (they are what the axis follows)
        const double xa = 0.25 * W, xb = 0.75 * W;
        if (dragging_) { // the dragged end, previewed only
            QPen pv(QColor(255, 205, 110, 180));
            pv.setWidth(1);
            pv.setStyle(Qt::DashLine);
            p.setPen(pv);
            const double x = std::max(0.0, std::min(double(W - 1), double(previewX_)));
            p.drawLine(QPointF(x, 0), QPointF(x, H));
        }
        p.fillRect(QRectF(xa, 0, xb - xa, H), QColor(255, 255, 255, 26));
        QPen mk(QColor(255, 205, 110));
        mk.setWidth(2);
        p.setPen(mk);
        p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(xa, 0), QPointF(xa, H));
        p.drawLine(QPointF(xb, 0), QPointF(xb, H));
        QFont f = p.font();
        f.setPixelSize(9);
        p.setFont(f);
        p.setPen(QColor(255, 225, 160));
        p.drawText(QRectF(xa + 3, 1, W - xa - 6, 11), Qt::AlignLeft | Qt::AlignTop,
                   QString::number(dclo, 'g', 4));
        p.drawText(QRectF(3, 1, xb - 3, 11), Qt::AlignRight | Qt::AlignTop,
                   QString::number(dchi, 'g', 4));
    }
    void mousePressEvent(QMouseEvent *e) override {
        const double xa = 0.25 * width(), xb = 0.75 * width();
        const double d = std::abs(e->pos().x() - xa);
        const double d2 = std::abs(e->pos().x() - xb);
        dragMin_ = d <= d2;
        if (std::min(d, d2) > 10) {
            dragMin_ = -1; // too far from either marker: ignore
            return;
        }
        // Dragging only moves this strip: re-rendering a megapixel image on
        // every mouse move is what made it crawl, so the new range is applied
        // when the button comes back up (see mouseReleaseEvent).
        dragging_ = true;
        if (h_)
            snap_ = *h_; // the bars: frozen as well (the image keeps updating)
        preClo_ = clo_; // the axis basis: frozen until the button is released
        preChi_ = chi_;
        previewX_ = e->pos().x();
        mouseMoveEvent(e);
    }
    void mouseMoveEvent(QMouseEvent *e) override {
        if (dragMin_ < 0)
            return;
        const int W = width();
        // The cursor -> value mapping uses the axis FROZEN at press time.  Using
        // the live clo_/chi_ here was the positive feedback: the move changed the
        // range, the range changed the window, and the same cursor position then
        // mapped to a different value, so the marker chased itself.
        const double span = (preChi_ > preClo_) ? (preChi_ - preClo_) : 1.0;
        double wlo = preClo_ - 0.5 * span, whi = preChi_ + 0.5 * span;
        if (logX_) {
            wlo = std::log10(std::max(wlo, 1e-2));
            whi = std::log10(std::max(whi, 1e-2));
            if (whi <= wlo)
                whi = wlo + 1.0;
        }
        // the value under the cursor in the window in force right now
        const double t = std::max(0.0, std::min(1.0, double(e->pos().x()) / W));
        const double v =
            logX_ ? std::pow(10.0, wlo + t * (whi - wlo)) : wlo + t * (whi - wlo);
        if (dragMin_)
            clo_ = std::min(v, chi_ - 1e-9);
        else
            chi_ = std::max(v, clo_ + 1e-9);
        previewX_ = e->pos().x();
        update(); // only the preview line moves; bars and axis stay frozen
    }
    void mouseReleaseEvent(QMouseEvent *) override {
        if (dragMin_ >= 0 && dragging_) {
            dragging_ = false; // unfreeze first, so the refresh is applied
            snap_.clear();     // back to the live histogram
            if (onRangeChanged)
                onRangeChanged(clo_, chi_); // one update, as soon as released
            update();
        }
        dragMin_ = -1;
    }

  private:
    const std::vector<int> *h_ = nullptr;
    double dlo_ = 0.0, dhi_ = 1.0, clo_ = 0.0, chi_ = 1.0;
    bool logX_ = false, logY_ = false;
    int dragMin_ = -1;
    bool dragging_ = false; // a marker drag in progress: preview only
    double preClo_ = 0.0, preChi_ = 1.0; // axis basis captured when the drag began
    std::vector<int> snap_;              // bins captured at press (frozen bars)
    int previewX_ = 0;                   // cursor x, for the dashed preview line
};

// What replaced the colour bar at the side of the image: a small panel with the
// histogram, the palette, a lin/log switch and the two ends of the mapped range.
class ScalePanel : public QWidget {
  public:
    ScalePanel(FastImageView *view, QWidget *parent = nullptr)
        : QWidget(parent), view_(view) {
        auto *col = new QVBoxLayout(this);
        col->setContentsMargins(4, 2, 4, 4);
        col->setSpacing(3);
        histo_ = new ScaleHisto(this);
        col->addWidget(histo_);
        auto *row = new QHBoxLayout();
        row->setSpacing(4);
        pal_ = new QComboBox(this);
        for (int i = 0; i <= int(Palette::Sunset); i++)
            pal_->addItem(paletteName(static_cast<Palette>(i)));
        scale_ = new QComboBox(this);
        scale_->addItem("lin");
        scale_->addItem("log");
        scale_->setToolTip("image: log10(value), values <= 0 are capped at\n"
                           "1e-2 so empty pixels stay visible");
        hscale_ = new QComboBox(this);
        hscale_->addItem("lin");
        hscale_->addItem("log");
        hscale_->setToolTip("histogram bars: linear counts, or log10(count+1).\n"
                            "The histogram's value axis always follows the image\n"
                            "mapping above.");
        lo_ = new QDoubleSpinBox(this);
        hi_ = new QDoubleSpinBox(this);
        for (QDoubleSpinBox *b : {lo_, hi_}) {
            b->setRange(-1e9, 1e9);
            b->setDecimals(1);
            b->setSingleStep(1.0);
            b->setKeyboardTracking(false);
        }
        row->addWidget(new QLabel("palette:", this));
        row->addWidget(pal_);
        row->addWidget(new QLabel("image", this));
        row->addWidget(scale_);
        row->addWidget(new QLabel("histo", this));
        row->addWidget(hscale_);
        row->addWidget(new QLabel("min", this));
        row->addWidget(lo_);
        row->addWidget(new QLabel("max", this));
        row->addWidget(hi_);
        row->addStretch(1);
        col->addLayout(row);
        connect(pal_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this](int) { push(); });
        connect(scale_, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this](int i) {
                    if (suppress_ || !view_)
                        return;
                    view_->setLogScale(i == 1);
                    refresh();
                });
        connect(hscale_, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this](int) { refresh(); });
        connect(lo_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                [this](double) { push(); });
        connect(hi_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                [this](double) { push(); });
        if (histo_)
            histo_->onRangeChanged = [this](double a, double b) {
                if (suppress_ || !view_)
                    return;
                printf("[scale] range %.4g .. %.4g\n", a, b);
                fflush(stdout);
                view_->setColorRange(a, b);
                suppress_++;
                if (lo_)
                    lo_->setValue(a);
                if (hi_)
                    hi_->setValue(b);
                suppress_--;
                refresh();
            };
        // as thin as asked: the image takes the window, the panel stays small
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        setFixedHeight(38 + 30);
        if (view_) {
            view_->onImageChanged = [this]() { refresh(); };
            refresh();
        }
    }
    // pull the view's state into the controls (after Set Ranges, a palette key…)
    void syncFromView() { refresh(); }
    // 0 = colour, 1 = BW, 2 = BW 2 - used by the b/B/c shortcuts
    void setPaletteChoice(int i) {
        if (pal_ && i >= 0 && i < pal_->count())
            pal_->setCurrentIndex(i);
    }

  private:
    void push() { // controls -> view
        if (suppress_ || !view_)
            return;
        const int pi = pal_ ? pal_->currentIndex() : 0;
        view_->setPaletteLut(paletteLut(static_cast<Palette>(pi)));
        double a = lo_ ? lo_->value() : 0.0, b = hi_ ? hi_->value() : 1.0;
        if (b <= a) {
            b = a + 1.0;
            suppress_++;
            if (hi_)
                hi_->setValue(b);
            suppress_--;
        }
        view_->setColorRange(a, b);
        refresh();
    }
  public:
    void refresh() { // view -> controls
        if (!view_)
            return;
        // Dragging the Z scale on the image changes the range on every mouse
        // move; re-filling the histogram from the image with that half-applied
        // range fed the drag back into the bars.  Frozen until release, when
        // onScaleReleased() calls this again.
        if (view_->isScaleDragging())
            return;
        suppress_++;
        double a = 0.0, b = 1.0;
        view_->colorRange(a, b);
        if (lo_)
            lo_->setValue(a);
        if (hi_)
            hi_->setValue(b);
        if (scale_)
            scale_->setCurrentIndex(view_->logScale() ? 1 : 0);
        double lo = 0.0, hi = 1.0;
        view_->histogramRange(lo, hi);
        if (histo_)
            histo_->setData(&view_->histogram(), lo, hi, a, b,
                            view_->logScale(),
                            hscale_ && hscale_->currentIndex() == 1);
        suppress_--;
    }
  private:
    FastImageView *view_ = nullptr;
    ScaleHisto *histo_ = nullptr;
    QComboBox *pal_ = nullptr, *scale_ = nullptr, *hscale_ = nullptr;
    QDoubleSpinBox *lo_ = nullptr, *hi_ = nullptr;
    int suppress_ = 0;
};

class MainWindow : public QMainWindow {
  public:
    MainWindow();
    ~MainWindow() override;
    void start();
    void stop();
  protected:
    void closeEvent(QCloseEvent *event) override;

  private:
    void setupUI();
    void connectShortcuts();

    // periodic GUI work
    void processImages(); // every 30 ms: mailbox -> widgets
    void updateReceiverRoiOverlay(); // red rx_roi box on the ADC image
    void updateStatus();  // every 500 ms: counters / labels
    void checkSignals();  // every 200 ms: Ctrl-C shutdown

    // shared actions (buttons + keyboard shortcuts)
    void toggleStartPause();
    void faster();
    void slower();
    void toggleGain();
    void setApplyGain(bool on);
    void zoomTo(FastImageView *src, double x1, double y1, double x2,
                double y2);
    void bwPalette1();
    void bwPalette2();
    void colPal();
    void toggleGateRoi(bool on);
    void setRoiEditMode(bool on);   // in-image ROI editor
    void saveRoi();
    void loadRoiDialog();
    // applies a gate ROI directly (no capture from the ADC view)
    // restrict == true (command line) also turns "ROI only" on; picking an
    // entry from the list only makes it the current ROI
    void applyRoi(int x, int y, int w, int h, const QString &src,
                  bool restrict = false);
    void resetHis();
    void resetAxis();
    void toggleRoiSave(bool on);
    void browseSaveDir();
    void saveDestDialog();
    void updateSaveLabel();
    void acquirePede();
    void savePede();
    void readPede();
    void loadAllPede();
    void loadGainMap();
    // lines up the boxes of the flow chart (gates on the left, Pedestal/Gain
    // on the right) and repaints the arrows
    void layoutFlow();
    void updateFlow();
    void updateGainMapLabel();
    void updateRoiLabel();
    void captureRoi(); // select the ADC zoom as the current ROI
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void showRangeDialog();
    void showRateDialog();
    void unsetRoi();         // forget the current ROI (full frame again)
    // While a run writes, the settings that would change what lands in the file
    // are locked (display-only controls stay live).
    void applySaveLock(bool saving);
    // Gate criterion changes stay possible during a run, but they ask first and
    // leave a note in the JSON of the file being written.
    bool confirmGateChangeDuringRun(const QString &what);
    void quitApp();
    void closeSideWindows(); // the ADC / gain / photon / panel windows
    // what the flow diagram has to show (see FlowStrip::setStages)
    void flowStageStates(bool &displayPede, bool &savePede, bool &gateOn,
                         bool &pmap) const;
    // asked before quitting; returns false when the operator cancels
    bool confirmQuit();

    void updateDrawLabel();
    void setRanges(int amin, int amax, int pmin, int pmax, int cmin, int cmax);
    // current on-screen ranges (read from the displayed histograms)
    void liveAdcImageRange(int &lo, int &hi) const;
    void liveMapRange(int &lo, int &hi) const;
    void liveChipRange(int &lo, int &hi) const;

    Pipeline pipeline_;
    QWidget *leftPad_ = nullptr, *pedePad_ = nullptr, *gainPad_ = nullptr;
    // leftPad, pedePad, gainPad, gatePad - kept between solving passes
    int flowPads_[4] = {0, 0, 0, 24};
    int flowPass_ = 0;
    bool flowQueued_ = false;
    QVBoxLayout *midCol_ = nullptr, *leftCol_ = nullptr, *rightCol_ = nullptr;
    QWidget *midSpacer_ = nullptr;
    FlowStrip *strip_ = nullptr;
    QWidget *gatePad_ = nullptr; // alignment gap between the two gates
    QLabel *arrGainDown_ = nullptr;
    QLabel *arrRoiDown_ = nullptr;
    QLabel *arrTrackUp_ = nullptr;
    QGroupBox *scGroup_ = nullptr, *sigGroup_ = nullptr;
    QGroupBox *displayGroup_ = nullptr, *saveGroup_ = nullptr;
    QGroupBox *pedeGroup_ = nullptr, *gainGroup_ = nullptr;
    QGroupBox *pmapGroup_ = nullptr; // virtual photon counting (needs a pede)
    QCheckBox *gainStateChk_ = nullptr; // read-only "gain corrected" indicator

    QTimer *processTimer = nullptr;
    QTimer *statusTimer = nullptr;
    QTimer *sigTimer = nullptr;

    QLabel *frameLabel = nullptr;
    QLabel *statusLabel = nullptr;
    QLabel *drawLabel = nullptr;
    QLabel *streamVal_ = nullptr;
    QLabel *pedeVal_ = nullptr;
    QLabel *errVal_ = nullptr;
    QLabel *framesInCumVal_ = nullptr;
    QLabel *framesInCurVal_ = nullptr;
    QLabel *framesPassCumVal_ = nullptr;
    QLabel *framesPassCurVal_ = nullptr;
    QLabel *framesDropCumVal_ = nullptr;
    QLabel *framesDropCurVal_ = nullptr;
    QLabel *pixelsAboveCumVal_ = nullptr;
    QLabel *pixelsAboveCurVal_ = nullptr;
    QLabel *rateInVal_ = nullptr;
    QLabel *ratePassVal_ = nullptr;
    QLabel *dropFracVal_ = nullptr;
    QLabel *displayDropVal_ = nullptr;
    QLabel *alignVal_ = nullptr;
    QLabel *gateRoiVal_ = nullptr;
    QSpinBox *saveFrames_ = nullptr;
    QCheckBox *saveCorrected_ = nullptr;
    QRadioButton *saveModeFrames_ = nullptr;
    QRadioButton *saveModePhoton_ = nullptr;
    QSpinBox *pmapSaveSpin_ = nullptr;
    QLabel *saveBudgetLabel_ = nullptr;
    QLineEdit *saveDirEdit_ = nullptr;
    // the destination as it was at start-up: that pair is the default, and the
    // "previous destinations" list must not collect it on every run
    QString saveDefaultDir_, saveDefaultBase_;
    QLineEdit *saveBaseEdit_ = nullptr;
    QPushButton *btnSaveDest_ = nullptr;
    QPushButton *btnSaveStart = nullptr;
    QLabel *saveVal_ = nullptr;
    bool m_saveWasRunning = false;

    QCheckBox *gateChk_ = nullptr;
    QLabel *thrLabel_ = nullptr; // says whether the gate threshold is on raw ADC
    QCheckBox *contigChk_ = nullptr;
    QCheckBox *hotChk_ = nullptr;
    QPushButton *btnGetHot_ = nullptr;
    std::vector<float> adcScratch_;    // ADC image with hot pixels replaced
    std::vector<float> photonScratch_; // photon map with hot pixels zeroed
    QLabel *hotVal_ = nullptr;
    QPushButton *btnResetHot_ = nullptr;
    QDoubleSpinBox *thrSpin_ = nullptr;
    QSpinBox *minPixSpin_ = nullptr;
    QPushButton *btnGateRoi = nullptr;
    QComboBox *scCombo_ = nullptr;
    QLabel *scLastVal_ = nullptr;
    QLabel *scDropCumVal_ = nullptr;
    QLabel *scDropCurVal_ = nullptr;
    QPushButton *btnSaveRoi = nullptr;
    QPushButton *btnLoadRoi = nullptr;
    QPushButton *btnSelectRoi_ = nullptr;
    QPushButton *btnUnsetRoi_ = nullptr;
    QGroupBox *roiGroup_ = nullptr;
    QLabel *roiListVal_ = nullptr;
    bool m_applyingRoi = false; // guards toggleGateRoi() during applyRoi()

    QPushButton *btnStartPause = nullptr;
    QPushButton *btnFaster = nullptr;
    QPushButton *btnSlower = nullptr;
    QPushButton *btnGainToggle = nullptr;
    QPushButton *btnBWPalette = nullptr;
    QPushButton *btnBWPalette2 = nullptr;
    QPushButton *btnLoadPede = nullptr;
    QPushButton *btnSavePede = nullptr;
    QPushButton *btnReadPede = nullptr;
    QPushButton *btnLoadAllPede = nullptr;
    QPushButton *btnGainMap = nullptr;
    QCheckBox *kevChk_ = nullptr;
    QPushButton *btnNoPede_ = nullptr;
    QPushButton *btnPedeTrack_ = nullptr;
    QLabel *pedeTrackVal_ = nullptr;
    QGroupBox *pedeTrackGroup_ = nullptr;
    QLabel *noPedeVal_ = nullptr;
    QLabel *gainMapVal_ = nullptr;
    QPushButton *btnResetHis = nullptr;
    QPushButton *btnResetAxis = nullptr;
    QPushButton *btnQuit = nullptr;
    QPushButton *btnSetRanges = nullptr;
    QPushButton *btnRates = nullptr;
    QDoubleSpinBox *photonThrSpin_ = nullptr; // ADU, or keV with gain on
    QLabel *photonThrLabel_ = nullptr;
    QPushButton *btnColPal = nullptr;
    QPushButton *btnResetCounters = nullptr;

    FastImageView *m_adcView = nullptr;
    FastImageView *m_gainView = nullptr;
    QWidget *m_adcWin = nullptr;
    ScalePanel *m_adcScale_ = nullptr;
    ScalePanel *m_photonScale_ = nullptr;
    QWidget *m_gainWin = nullptr;
    QWidget *m_photonWin = nullptr;
    FastImageView *m_photonView = nullptr;
    QCheckBox *pmapCumulative_ = nullptr;
    QCheckBox *zoomCoordChk_ = nullptr;
    QPushButton *btnShowGain_ = nullptr;  // Display: reveal the Gain window
    QPushButton *btnShowPlots_ = nullptr; // Display: reveal the ROOT canvases // Display: link the image zoom levels
    bool zoomCoord_ = true;             // on = any window zooms them all
    QSpinBox *pmapEverySpin_ = nullptr;
    uint64_t pmapSeq_ = 0;
#ifdef JFSTRX
    // separate strixel window: six group panels, display only (no ROI)
    std::array<FastImageView *, kNumAdcPanels> m_adcPanelView{};
    QWidget *m_adcPanelWin = nullptr;
    QLabel *m_adcPanelZ = nullptr;   // "Z: lo .. hi" legend of the strixel panels
#endif

    // last receiver ROI mirrored onto the ADC image (red box)
    bool m_rxRoiShown = false;
    int m_rxRoiLast[4] = {-1, -1, -1, -1};
};

MainWindow::MainWindow() : QMainWindow(nullptr) {
    setWindowTitle("Online JF Display + Event Finder");
    setMinimumWidth(420);

    setupUI();
    connectShortcuts();

    goout = 1;
    gain_flag = false;
    HDraw_every = 20;
    // the operator's range defaults (Set Ranges) are in force from the start;
    // "Reset Axis" releases them and the axes auto-scale again
    fixranges = true;

    initGeometry();

    hchptr = (short *)malloc(NCH * sizeof(short));
    for (int k = 0; k < NCH; k++)
        hchptr[k] = 0;

    cout << "opening the rootapp" << endl;
    int argc_copy = 1;
    char dummy_name[] = "onlinedisp";
    char *argv_copy[] = {dummy_name, nullptr};
    rootapp = new TApplication("Online JF display", &argc_copy, argv_copy);

    loadRootPalette(Palette::Color);

    char hname[100];
#ifdef JFSTRX
    int nxx = 79;
    int nyy = 165;
    his1000 = new TH2F("his1000", "2d, chip 1 group 1 (25um) pede corr.", nxx,
                       -0.5, nxx - 0.5, nyy, -0.5, nyy - 0.5);
    his1060 = new TH2F("his1060", "2d, chip 6 group 1 (25um) pede corr.", nxx,
                       -0.5, nxx - 0.5, nyy, -0.5, nyy - 0.5);
    nxx = 47;
    nyy = 320;
    his1001 = new TH2F("his1001", "2d, chip 1 group 2 (15um) pede corr.", nxx,
                       -0.5, nxx - 0.5, nyy, -0.5, nyy - 0.5);
    his1061 = new TH2F("his1061", "2d, chip 6 group 2 (15um) pede corr.", nxx,
                       -0.5, nxx - 0.5, nyy, -0.5, nyy - 0.5);
    nxx = 59;
    nyy = 476;
    his1002 = new TH2F("his1002", "2d, chip 1 group 3 (18.75um) pede corr.", nxx,
                       -0.5, nxx - 0.5, nyy, -0.5, nyy - 0.5);
    his1062 = new TH2F("his1062", "2d, chip 6 group 3 (18.75um) pede corr.", nxx,
                       -0.5, nxx - 0.5, nyy, -0.5, nyy - 0.5);
    his1000->SetOption("colz");
    his1001->SetOption("colz");
    his1002->SetOption("colz");
    his1060->SetOption("colz");
    his1061->SetOption("colz");
    his1062->SetOption("colz");
#else
    his1000 = new TH2F("his1000", "2d, ev. pede corrected", nx, -0.5, nx - 0.5,
                       ny, -0.5, ny - 0.5);
    his1000->SetOption("colz");
#endif

    his2000 = new TH2F("his2000", "2d gain", nx, -0.5, nx - 0.5, ny, -0.5,
                       ny - 0.5);
    his2000->GetZaxis()->SetRangeUser(0, 4);

    // The photon map has no ROOT histogram any more: it is filled in the
    // analysis and shown in its own Qt window (see "virtual photon counting").

    his4500 = new TH2F("his4500", "T vs B", 101, -500, MAX_POS, 101, -500,
                       MAX_POS);
    hisLvsR = new TH2F("hisLvsR", "L vs R", 101, -500, MAX_POS, 101, -500,
                       MAX_POS);
    hchip = new TH1I *[8];
#ifdef JFSTRX
    for (int k = 0; k < 8; k++) {
        if (k < 3)
            sprintf(hname, "%d_hchip1group%d", k, k + 1);
        else if (k > 4)
            sprintf(hname, "%d_hchip6group%d", k, k - 4);
        else
            sprintf(hname, "%d_", k);
        hchip[k] = new TH1I(hname, hname, NBIN, MIN_POS, MAX_POS);
    }
#else
    for (int k = 0; k < 8; k++) {
        sprintf(hname, "hchip%d", k);
        hchip[k] = new TH1I(hname, hname, NBIN, MIN_POS, MAX_POS);
    }
#endif
    // ONE spectrum of the gate-ROI pixels (canvas A7): identical binning to
    // the chip spectra, summed over the whole selection
    hroi = new TH1I("hroi", "hroi", NBIN, MIN_POS, MAX_POS);
    // the same two sets again in keV (canvases A8/A9).  The axes are the ADU
    // axes divided by KEV_NOMINAL_ADU with the same number of bins, so a
    // pixel's keV value selects the same bin index as its ADU value and the
    // two sets always show the same window.
    hchipKev = new TH1I *[8];
    for (int k = 0; k < 8; k++) {
        char kn[64];
        sprintf(kn, "hchip%d_keV", k);
        hchipKev[k] = new TH1I(kn, kn, NBIN, MIN_POS / KEV_NOMINAL_ADU,
                               MAX_POS / KEV_NOMINAL_ADU);
    }
    hroiKev = new TH1I("hroi_keV", "hroi_keV", NBIN, MIN_POS / KEV_NOMINAL_ADU,
                       MAX_POS / KEV_NOMINAL_ADU);

    cout << "end of histo booking" << endl;
#ifdef JFSTRX
    if (A3 == nullptr) {
        A3 = new TCanvas("A3", "Plotting Canvas ADC", 150, 360, 1200, 750);
        p1 = new TPad("p1", "p1", 0.01, 0.5, 0.49, 1.);
        p1->Draw();
        p2 = new TPad("p2", "p2", 0.51, 0.75, 0.99, 0.97);
        p2->Draw();
        p3 = new TPad("p3", "p3", 0.01, 0.25, 0.49, 0.47);
        p3->Draw();
        p4 = new TPad("p4", "p4", 0.51, 0.53, 0.99, 0.75);
        p4->Draw();
        p5 = new TPad("p5", "p5", 0.01, 0.03, 0.49, 0.25);
        p5->Draw();
        p6 = new TPad("p6", "p6", 0.51, 0., 0.99, 0.5);
        p6->Draw();
    }
#endif
    if (A4 == nullptr)
        A4 = new TCanvas("A4", "Plotting Canvas PHs", 750, 300, 1000, 800);
    A4->Clear();
    A4->Divide(4, 2, 0.005, 0.005);
    if (false) // the photon map canvas is gone; keep the pointer unused
        A5 = new TCanvas("A5", "Plotting Canvas Photon Map", 750, 300, 1000,
                         600);
    if (A6 == nullptr)
        A6 = new TCanvas("A6", "Plotting Canvas LvsR", 650, 250, 650, 660);
    if (A7 == nullptr)
        A7 = new TCanvas("A7", "ROI PH", 770, 340, 1000, 700);
    A7->Clear(); // one pad: all the gate-ROI pixels in one spectrum
    // The keV canvases are NOT created here: a ROOT canvas maps its window in
    // the constructor, and these two must stay invisible until a gain map is
    // loaded.  They are created lazily by the ROOT thread in
    // Plot1DHistosKev()/PlotRoiHistosKev() the first time they are needed.


    // display clones painted by the ROOT thread (sources stay exclusive to
    // the analysis thread)
    disp4500 = (TH2F *)his4500->Clone("disp4500");
    dispLvsR = (TH2F *)hisLvsR->Clone("dispLvsR");
    for (int ih = 0; ih < 8; ih++) {
        char nm[64];
        sprintf(nm, "dispChip%d", ih);
        dispChip[ih] = (TH1I *)hchip[ih]->Clone(nm);
    }
    dispRoi = (TH1I *)hroi->Clone("dispRoi");
    for (int ih = 0; ih < 8; ih++) {
        char nm[64];
        sprintf(nm, "dispChipKev%d", ih);
        dispChipKev[ih] = (TH1I *)hchipKev[ih]->Clone(nm);
    }
    dispRoiKev = (TH1I *)hroiKev->Clone("dispRoiKev");

    // start with the operator's ranges (Set Ranges dialog): the spectra open
    // on the PH window, the main plot keeps its value window (fixranges)
    for (int ih = 0; ih < 8; ih++) {
        if (dispChip[ih])
            dispChip[ih]->GetXaxis()->SetRangeUser(float(chipxmin.load()),
                                                   float(chipxmax.load()));
    }
    if (dispRoi)
        dispRoi->GetXaxis()->SetRangeUser(float(chipxmin.load()),
                                          float(chipxmax.load()));
    if (dispRoiKev)
        dispRoiKev->GetXaxis()->SetRangeUser(
            chipxmin.load() / KEV_NOMINAL_ADU,
            chipxmax.load() / KEV_NOMINAL_ADU);
    for (int ih = 0; ih < 8; ih++) {
        if (dispChipKev[ih])
            dispChipKev[ih]->GetXaxis()->SetRangeUser(
                chipxmin.load() / KEV_NOMINAL_ADU,
                chipxmax.load() / KEV_NOMINAL_ADU);
    }
    // the photon window opens on the photon-map colour range (the ADC image
    // was given the ADC range in setupUI())
    if (m_photonView)
        m_photonView->setColorRange(pmmin.load(), pmmax.load());
#ifdef JFSTRX
    disp1000 = (TH2F *)his1000->Clone("disp1000");
    disp1001 = (TH2F *)his1001->Clone("disp1001");
    disp1002 = (TH2F *)his1002->Clone("disp1002");
    disp1060 = (TH2F *)his1060->Clone("disp1060");
    disp1061 = (TH2F *)his1061->Clone("disp1061");
    disp1062 = (TH2F *)his1062->Clone("disp1062");
#endif

    printhelp();
}

MainWindow::~MainWindow() { stop(); }

// ---------------------------------------------------------------------------
// UI
// ---------------------------------------------------------------------------
void MainWindow::setupUI() {
    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *rootLayout = new QVBoxLayout(central);
    auto *columns = new QHBoxLayout();
    rootLayout->addLayout(columns);

    auto *leftCol = new QVBoxLayout();
    auto *rightCol = new QVBoxLayout();
    // The connectors that cross from one column to the other are drawn by a
    // strip placed by hand in the gap (not in the layout: an item here would
    // stretch the column row and leave empty space above the save panel).  A
    // zero-height spacer reserves the width.
    midCol_ = new QVBoxLayout();
    leftCol_ = leftCol;
    rightCol_ = rightCol;
    midSpacer_ = new QWidget(central);
    midSpacer_->setFixedWidth(64);
    midSpacer_->setFixedHeight(0);
    midCol_->addWidget(midSpacer_);
    strip_ = new FlowStrip(central);
    columns->addLayout(leftCol, 1);
    columns->addLayout(midCol_, 0);
    columns->addLayout(rightCol, 1);
    // bottom row: what the program is doing (Status) and what it has counted.
    // It is added to the layout *after* the save panel (see the end of this
    // function) so the save ends up above the counters.
    auto *bottomRow = new QHBoxLayout();

    // ── Status ──
    auto *statusGroup = new QGroupBox("Status", central);
    auto *statusLayout = new QVBoxLayout(statusGroup);
    frameLabel = new QLabel("Frame: --", statusGroup);
    statusLabel = new QLabel("Status: idle", statusGroup);
    drawLabel = new QLabel(statusGroup);
    streamVal_ = makeValueLabel("disconnected", statusGroup);
    pedeVal_ = makeValueLabel("not loaded", statusGroup);
    errVal_ = makeValueLabel(QString(), statusGroup);
    errVal_->setWordWrap(true);
    displayDropVal_ = makeValueLabel("0", statusGroup);
    alignVal_ = makeValueLabel(QString(), statusGroup);
    alignVal_->setWordWrap(true);
    statusLayout->addWidget(frameLabel);
    statusLayout->addWidget(statusLabel);
    statusLayout->addWidget(drawLabel);
    statusLayout->addWidget(new QLabel("Stream:", statusGroup));
    statusLayout->addWidget(streamVal_);
    statusLayout->addWidget(new QLabel("Pedestal:", statusGroup));
    statusLayout->addWidget(pedeVal_);
    statusLayout->addWidget(new QLabel("Display drops (analysis too slow):",
                                       statusGroup));
    statusLayout->addWidget(displayDropVal_);
    statusLayout->addWidget(new QLabel("Frame alignment:", statusGroup));
    statusLayout->addWidget(alignVal_);
    statusLayout->addWidget(errVal_);
    bottomRow->addWidget(statusGroup, 1);

    // ── Gate (event finder): two gates, applied in series ──
    auto *gateGroup = new QGroupBox("Event finder gate", central);
    auto *gateCol = new QVBoxLayout(gateGroup);

    // gate 1: storage cell.  This is a header field, so it is checked first
    // and frames from other cells cost no CPU at all.
    auto *scGroup = new QGroupBox("1. Storage cell (header)", gateGroup);
    scGroup_ = scGroup;
    auto *scForm = new QFormLayout(scGroup);
    scCombo_ = new QComboBox(scGroup);
    scCombo_->addItem("ALL"); // index 0 == no storage-cell gating
    for (int i = 0; i <= 15; i++)
        scCombo_->addItem(QString::number(i));
    scCombo_->setToolTip(
        "Pass only frames acquired in this storage cell (bits 8-11 of the\n"
        "header's detector specific 3; 0..15, 15 = default).  \"ALL\" passes\n"
        "every cell.  Checked before any pixel is touched, so the signal\n"
        "gate only ever sees frames that survive this one.");
    scLastVal_ = makeValueLabel("last frame: ?", scGroup);
    scForm->addRow("Only storage cell:", scCombo_);
    scForm->addRow(scLastVal_);
    gateCol->addWidget(scGroup);
    // Plain invisible spacer between the two gates: the path from the storage
    // cell to the signal gate goes through the Pedestal and is drawn by the
    // strip, so a direct arrow here would be wrong.  layoutFlow() stretches
    // this gap so the signal gate's middle lands on the Gain's top.
    gatePad_ = new QWidget(gateGroup);
    gatePad_->setFixedHeight(24);
    gateCol->addWidget(gatePad_);

    // gate 2: the signal gate (the pixel counting)
    auto *sigGroup = new QGroupBox("2. Signal gate (pixel counting)", gateGroup);
    sigGroup_ = sigGroup;
    auto *gateForm = new QFormLayout(sigGroup);
    gateChk_ = new QCheckBox("Gate enabled (off = passthrough)", sigGroup);
    gateChk_->setChecked(g_cfg.gateEnabled);
    contigChk_ = new QCheckBox("Contiguous (N px within circle, r = N/2+1)",
                               sigGroup);
    contigChk_->setChecked(g_cfg.contiguous);
    thrSpin_ = new QDoubleSpinBox(sigGroup);
    thrSpin_->setRange(0.0, 1000000.0);
    thrSpin_->setDecimals(1);
    thrSpin_->setSingleStep(10.0);
    thrSpin_->setValue(g_cfg.threshold);
    minPixSpin_ = new QSpinBox(sigGroup);
    minPixSpin_->setRange(1, 10000000);
    minPixSpin_->setValue(static_cast<int>(g_cfg.minPixels));
    btnResetCounters = new QPushButton("Reset counters", gateGroup);
    btnGateRoi = new QPushButton("ROI only (i)", sigGroup);
    btnGateRoi->setCheckable(true);
    btnGateRoi->setToolTip(
        "Count only inside the ROI captured from the ADC zoom at the moment\n"
        "this is pressed.  The ROI is a snapshot: panning or zooming the ADC\n"
        "image afterwards does not move it (it stays outlined in yellow).");
    gateRoiVal_ = makeValueLabel("full frame", sigGroup);
    {
        auto *gateRow = new QHBoxLayout();
        gateRow->addWidget(gateChk_);
        gateRow->addWidget(btnGateRoi);
        gateForm->addRow(gateRow);
    }
    gateForm->addRow(contigChk_);
    hotChk_ = new QCheckBox("Suppress hot pixels in the gate", sigGroup);
    hotChk_->setChecked(g_cfg.hotSuppress);
    hotChk_->setToolTip(
        "Masked pixels (see Get hot pixels) are excluded from the gate: they\n"
        "do not count towards min pixels and cannot form a contiguous cluster.");
    hotVal_ = makeValueLabel("hot: 0 px", sigGroup);
    btnGetHot_ = new QPushButton("Get hot pixels...", sigGroup);
    btnGetHot_->setToolTip(
        "Mark hot pixels from the current photon map: every pixel whose map\n"
        "value is above the threshold you enter becomes a hot pixel.");
    btnResetHot_ = new QPushButton("Clear hot pixels", sigGroup);
    {
        auto *hotRow = new QHBoxLayout();
        hotRow->addWidget(hotChk_);
        hotRow->addWidget(hotVal_);
        gateForm->addRow("Hot pixels:", hotRow);
        auto *hotBtnRow = new QHBoxLayout();
        hotBtnRow->addWidget(btnGetHot_);
        hotBtnRow->addWidget(btnResetHot_);
        gateForm->addRow(hotBtnRow);
    }
    thrLabel_ = new QLabel("Threshold (ADC, pede-subtracted):", sigGroup);
    gateForm->addRow(thrLabel_, thrSpin_);
    gateForm->addRow("Min pixels above threshold:", minPixSpin_);
    gateForm->addRow("Gate ROI:", gateRoiVal_);
    gateCol->addWidget(sigGroup);

    auto *seriesHint = new QLabel(
        "Applied in series - storage cell first, so frames from other cells "
        "never reach the pixel counting (all 0-15, no gating, and both "
        "together are all possible).",
        gateGroup);
    seriesHint->setWordWrap(true);
    gateCol->addWidget(seriesHint);
    gateCol->addWidget(btnResetCounters);
    // 1) Storage cell then 2) Signal gate: the left column *is* the gate flow.
    // leftPad_ is sized at run time so that the middle of the storage-cell box
    // lands on the top edge of the Pedestal box (see layoutFlow()).
    // (Quit is added at the top of the right column, right aligned)
    leftPad_ = new QWidget(central);
    leftPad_->setFixedHeight(0);
    leftCol->addWidget(leftPad_);
    leftCol->addWidget(gateGroup);

    // ── Counters ──
    auto *counterBox = new QGroupBox("Counters", central);
    auto *grid = new QGridLayout(counterBox);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(2, 1);
    auto *headCum = new QLabel("Cumulative", counterBox);
    auto *headCur = new QLabel("Current", counterBox);
    headCum->setStyleSheet("font-weight: bold;");
    headCur->setStyleSheet("font-weight: bold;");
    grid->addWidget(new QLabel(QString(), counterBox), 0, 0);
    grid->addWidget(headCum, 0, 1);
    grid->addWidget(headCur, 0, 2);

    framesInCumVal_ = makeValueLabel("0", counterBox);
    framesInCurVal_ = makeValueLabel("0", counterBox);
    grid->addWidget(new QLabel("Frames in", counterBox), 1, 0);
    grid->addWidget(framesInCumVal_, 1, 1);
    grid->addWidget(framesInCurVal_, 1, 2);

    framesPassCumVal_ = makeValueLabel("0", counterBox);
    framesPassCurVal_ = makeValueLabel("0", counterBox);
    grid->addWidget(new QLabel("Frames passed", counterBox), 2, 0);
    grid->addWidget(framesPassCumVal_, 2, 1);
    grid->addWidget(framesPassCurVal_, 2, 2);

    framesDropCumVal_ = makeValueLabel("0", counterBox);
    framesDropCurVal_ = makeValueLabel("0", counterBox);
    grid->addWidget(new QLabel("Frames dropped", counterBox), 3, 0);
    grid->addWidget(framesDropCumVal_, 3, 1);
    grid->addWidget(framesDropCurVal_, 3, 2);

    scDropCumVal_ = makeValueLabel("0", counterBox);
    scDropCurVal_ = makeValueLabel("0", counterBox);
    grid->addWidget(new QLabel("Frames dropped (SC)", counterBox), 4, 0);
    grid->addWidget(scDropCumVal_, 4, 1);
    grid->addWidget(scDropCurVal_, 4, 2);

    pixelsAboveCumVal_ = makeValueLabel("0", counterBox);
    pixelsAboveCurVal_ = makeValueLabel("0", counterBox);
    grid->addWidget(new QLabel("Pixels above", counterBox), 5, 0);
    grid->addWidget(pixelsAboveCumVal_, 5, 1);
    grid->addWidget(pixelsAboveCurVal_, 5, 2);

    rateInVal_ = makeValueLabel("0 fps / 0 MB/s", counterBox);
    grid->addWidget(new QLabel("Rate in (fps / MB/s)", counterBox), 6, 0);
    grid->addWidget(rateInVal_, 6, 1, 1, 2);
    ratePassVal_ = makeValueLabel("0 fps / 0 MB/s", counterBox);
    grid->addWidget(new QLabel("Rate pass (fps / MB/s)", counterBox), 7, 0);
    grid->addWidget(ratePassVal_, 7, 1, 1, 2);
    dropFracVal_ = makeValueLabel("0.000", counterBox);
    grid->addWidget(new QLabel("Drop fraction (cum.)", counterBox), 8, 0);
    grid->addWidget(dropFracVal_, 8, 1, 1, 2);
    bottomRow->addWidget(counterBox, 1);

    // ── Display controls ──
    auto *ctrlGroup = new QGroupBox("Display", central);
    displayGroup_ = ctrlGroup;
    auto *ctrlGrid = new QGridLayout(ctrlGroup);
    btnStartPause = new QPushButton("Start/Pause (s)", ctrlGroup);
    btnFaster = new QPushButton("Faster (+)", ctrlGroup);
    btnSlower = new QPushButton("Slower (-)", ctrlGroup);
    btnGainToggle = new QPushButton("gain correction on/off (g)", ctrlGroup);
    btnGainToggle->setCheckable(true);
    btnGainToggle->setToolTip(
        "Divide every pixel by its ADU/keV - the per-pixel value from the gain\n"
        "map when one is loaded, the nominal 40 ADU/keV otherwise - to get keV.\n"
        "\"keV PH\" then shows the PH spectra in keV; with this off (or keV PH\n"
        "unticked) they show the raw pedestal-subtracted ADC.");
    btnBWPalette = new QPushButton("BW Pal. (b)", ctrlGroup);
    btnBWPalette2 = new QPushButton("BW Pal. 2 (B)", ctrlGroup);
    btnSetRanges = new QPushButton("Set Ranges (S)", ctrlGroup);
    btnColPal = new QPushButton("Col Pal (c)", ctrlGroup);
    btnRates = new QPushButton("Update Rates", ctrlGroup);
    btnResetHis = new QPushButton("Reset His (r)", ctrlGroup);
    btnResetAxis = new QPushButton("Reset Axis (R)", ctrlGroup);
    photonThrSpin_ = new QDoubleSpinBox(central);
    photonThrSpin_->setRange(0.0, 100000.0);
    photonThrSpin_->setDecimals(0); // ADU: whole numbers
    photonThrSpin_->setSingleStep(10.0);
    photonThrSpin_->setValue(static_cast<double>(g_pheneThreshold));
    photonThrSpin_->setToolTip(
        "Photon-map threshold (pede-subtracted ADC): only pixels above this\n"
        "contribute to the map, and the number of photons a pixel counts for is\n"
        "(value + threshold) / (2 x threshold).  In ADU while the gain\n"
        "correction is off, in keV while it is on (the value is then divided by\n"
        "the pixel's ADU/keV first); both are remembered separately.  It does\n"
        "not affect the event-finder gate, which has its own threshold.");
    // ── Virtual photon counting ──
    // Left of the Display column (the Display lives in the right column at this
    // height).  The map itself is the same photon counting as before, but shown
    // in its own Qt window: a fresh image every N analysed frames, and
    // cumulative only if asked.
    auto *pmapGroup = new QGroupBox("Virtual photon counting", central);
    pmapGroup_ = pmapGroup;
    auto *pmapGrid = new QGridLayout(pmapGroup);
    pmapCumulative_ = new QCheckBox("cumulative (toggle to reset)", pmapGroup);
    pmapCumulative_->setChecked(gPmapCumulative.load());
    pmapCumulative_->setToolTip(
        "off: every displayed image shows only the photons counted since the\n"
        "previous one; on: the map keeps adding up (the old behaviour)");
    pmapGrid->addWidget(pmapCumulative_, 0, 0, 1, 2);
    pmapGrid->addWidget(
        new QLabel("write plot every N (gated) images:", pmapGroup), 1, 0);
    pmapEverySpin_ = new QSpinBox(pmapGroup);
    pmapEverySpin_->setRange(1, 1000000);
    pmapEverySpin_->setSingleStep(100);
    pmapEverySpin_->setValue(gPmapEvery.load());
    pmapEverySpin_->setToolTip(
        "How many analysed frames are accumulated into one photon map image.\n"
        "It counts the frames that reach the analysis, i.e. AFTER the gate and\n"
        "the frame decimation, so the wall-clock interval between two maps\n"
        "grows when frames are rejected or skipped (it is not independent of\n"
        "them).  Default 2000.");
    pmapGrid->addWidget(pmapEverySpin_, 1, 1);
    photonThrLabel_ = new QLabel("photon threshold (ADU):", pmapGroup);
    pmapGrid->addWidget(photonThrLabel_, 2, 0);
    // the photon threshold moves here from the Display panel
    pmapGrid->addWidget(photonThrSpin_, 2, 1);
    // under the gates, left of the Display column
    leftCol->insertWidget(2, pmapGroup);
    leftCol->addStretch(1);

    ctrlGrid->addWidget(btnStartPause, 0, 0);
    ctrlGrid->addWidget(btnFaster, 0, 1);
    ctrlGrid->addWidget(btnSlower, 0, 2);
    // the three palette buttons share one row
    ctrlGrid->addWidget(btnBWPalette, 1, 0);
    ctrlGrid->addWidget(btnBWPalette2, 1, 1);
    ctrlGrid->addWidget(btnColPal, 1, 2);
    // (btnGainToggle lives in the Gain panel, below)
    ctrlGrid->addWidget(btnSetRanges, 2, 0);
    ctrlGrid->addWidget(btnRates, 2, 1);
    ctrlGrid->addWidget(btnResetHis, 3, 0);
    ctrlGrid->addWidget(btnResetAxis, 3, 1);
    zoomCoordChk_ = new QCheckBox("zoom coordination", ctrlGroup);
    zoomCoordChk_->setChecked(true); // the historic ADC-master behaviour
    zoomCoordChk_->setToolTip(
        "On: zooming any image window zooms all of them (and the ROOT plots)\n"
        "to the same rectangle.  Off: every window keeps its own zoom, and\n"
        "nothing is propagated.");
    ctrlGrid->addWidget(zoomCoordChk_, 4, 0, 1, 3);
    // Both start hidden ("disabled by default"): the operator reveals them.
    btnShowGain_ = new QPushButton("show gain image", ctrlGroup);
    btnShowGain_->setCheckable(true);
    btnShowGain_->setChecked(false);
    btnShowGain_->setToolTip(
        "Show or hide the separate Gain Image window.  It is hidden at\n"
        "startup; the gain map itself is still used when the gain correction\n"
        "is on.");
    btnShowPlots_ = new QPushButton("show ROOT plots", ctrlGroup);
    btnShowPlots_->setCheckable(true);
    btnShowPlots_->setChecked(false);
    btnShowPlots_->setToolTip(
        "Show or hide the ROOT spectra canvases (PH spectra, ROI PH, L vs R,\n"
        "the 2D map).  They are hidden at startup; the histograms keep being\n"
        "filled, so showing them later shows the accumulated data.");
    ctrlGrid->addWidget(btnShowGain_, 5, 0, 1, 2);
    ctrlGrid->addWidget(btnShowPlots_, 6, 0, 1, 2);
    connect(btnShowGain_, &QPushButton::toggled, this, [this](bool on) {
        if (m_gainWin)
            m_gainWin->setVisible(on);
        printf("[display] gain image %s\n", on ? "shown" : "hidden");
        fflush(stdout);
        statusLabel->setText(on ? "Status: gain image shown"
                                : "Status: gain image hidden");
    });
    connect(btnShowPlots_, &QPushButton::toggled, this, [this](bool on) {
        RootCmd c;
        c.type = RootCmdType::ShowPlots;
        c.a = on ? 1.0 : 0.0;
        pushRootCmd(c);
        printf("[display] ROOT plots %s\n", on ? "shown" : "hidden");
        fflush(stdout);
        statusLabel->setText(on ? "Status: ROOT plots shown"
                                : "Status: ROOT plots hidden");
    });
    connect(zoomCoordChk_, &QCheckBox::toggled, this, [this](bool on) {
        zoomCoord_ = on;
        printf("[zoom] coordination %s\n", on ? "on" : "off");
        fflush(stdout);
        statusLabel->setText(on ? "Status: zoom coordination on (all linked)"
                                : "Status: zoom coordination off (independent)");
    });

    // (the Display panel is added below the Gain panel, see below: it shows
    // the result, so it comes after the chain that produces it)

    // ── ROI save (HDF5) ──
    auto *saveGroup = new QGroupBox("Save", central);
    saveGroup_ = saveGroup;
    auto *saveForm = new QFormLayout(saveGroup);
    saveFrames_ = new QSpinBox(saveGroup);
    saveFrames_->setRange(0, 1000000000);
    saveFrames_->setValue(1000);
    saveFrames_->setSpecialValueText("unlimited");
    if (g_cfg.saveBudget > 0) // --save-budget: frames, or maps in photon mode
        saveFrames_->setValue(static_cast<int>(g_cfg.saveBudget));
    saveFrames_->setToolTip(
        "Number of gate-passed frames to write; 0 = unlimited (stop with the\n"
        "button).  The run also stops by itself when this many frames are in\n"
        "the file.  In virtual photon counting mode it counts the maps.");
    btnSaveStart = new QPushButton("Start/Stop save (n)", saveGroup);
    btnSaveStart->setCheckable(true);
    btnSaveStart->setToolTip(
        "Write the active gate ROI of every gate-passed frame to an HDF5\n"
        "file (slsReceiver layout, slice 0 = pedestal) plus a JSON sidecar.");
    saveCorrected_ = new QCheckBox("Pedestal corrected", saveGroup);
    saveCorrected_->setChecked(false);
    saveCorrected_->setToolTip(
        "Off (default): /data holds the raw 16-bit words and slice 0 is the\n"
        "pedestal image.\n"
        "On: /data holds pedestal-corrected float32 values with no pedestal\n"
        "slice, and the pedestal image is written to the separate dataset\n"
        "/pedestal data.  Needs a loaded pedestal.");
    saveDirEdit_ = new QLineEdit(QString::fromStdString(g_cfg.h5Dir), saveGroup);
    saveDirEdit_->setToolTip("Directory the HDF5 file and JSON sidecar go to.");
    auto *dirBtn = new QPushButton("...", saveGroup);
    dirBtn->setFixedWidth(32);
    auto *dirRow = new QHBoxLayout();
    dirRow->addWidget(saveDirEdit_);
    dirRow->addWidget(dirBtn);
    saveBaseEdit_ = new QLineEdit(QString::fromStdString(g_cfg.h5Base), saveGroup);
    saveDefaultBase_ = saveBaseEdit_->text().trimmed(); // start-up default
    saveBaseEdit_->setToolTip(
        "File-name base.  The file is <base>_<YYYYmmdd_HHMMSS>_f0_0.h5 (and the\n"
        "JSON the same with .json), so re-using a base never overwrites.");
    btnSaveDest_ = new QPushButton("Previous destinations...", saveGroup);
    btnSaveDest_->setToolTip(
        "List of destinations used before (directory + base), most recent\n"
        "first; pick one to fill the fields, or delete entries.");
    saveDefaultDir_ = saveDirEdit_->text().trimmed(); // start-up default
    saveVal_ = makeValueLabel("idle", saveGroup);
    saveVal_->setWordWrap(true);
    saveBudgetLabel_ =
        new QLabel("Number of frames to be saved:", saveGroup);
    saveForm->addRow(saveBudgetLabel_, saveFrames_);
    // what gets written: the gate-passed frames, or the virtual photon
    // counting maps (non-cumulative, one every M analysed frames)
    saveModeFrames_ = new QRadioButton("save frames", saveGroup);
    saveModePhoton_ = new QRadioButton("virtual photon counting", saveGroup);
    saveModeFrames_->setChecked(gSaveMode.load() != 1);
    saveModePhoton_->setChecked(gSaveMode.load() == 1);
    saveForm->addRow(saveModeFrames_);
    // the correction is a property of the *frames* being saved: the photon maps
    // are always pedestal-subtracted, so the box lives under that radio and is
    // greyed while the maps are selected
    saveForm->addRow(saveCorrected_);
    saveForm->addRow(saveModePhoton_); // (was dropped by the move above)
    pmapSaveSpin_ = new QSpinBox(saveGroup);
    pmapSaveSpin_->setRange(200, 1000000); // the requested minimum
    pmapSaveSpin_->setSingleStep(100);
    pmapSaveSpin_->setValue(gPmapSaveEvery.load());
    pmapSaveSpin_->setEnabled(gSaveMode.load() == 1);
    pmapSaveSpin_->setToolTip(
        "virtual photon counting: one non-cumulative map every this many\n"
        "analysed frames (minimum 200), independent of the display cadence");
    saveForm->addRow("photon map every N frames:", pmapSaveSpin_);
    saveForm->addRow("Directory:", dirRow);
    saveForm->addRow("Base name:", saveBaseEdit_);
    saveForm->addRow(btnSaveDest_);
    saveForm->addRow(btnSaveStart);
    saveForm->addRow("Save status:", saveVal_);
    // (the ROI-save panel is added below the Gain panel, see the end of this
    // function: the right column reads Display, Pedestal, Gain, Save)

    // ── Pedestal ──
    auto *pedeGroup = new QGroupBox("Pedestal", central);
    pedeGroup_ = pedeGroup;
    auto *pedeGrid = new QGridLayout(pedeGroup);
    btnLoadPede = new QPushButton("Acquire Pede (p)", pedeGroup);
    btnSavePede = new QPushButton("Save Pede (O)", pedeGroup);
    btnReadPede = new QPushButton("Read Pede (o)", pedeGroup);
    btnLoadAllPede = new QPushButton("Load Pede File (P)", pedeGroup);
    btnGainMap = new QPushButton("Gain map (keV)...", pedeGroup);
    kevChk_ = new QCheckBox("keV PH", pedeGroup);
    kevChk_->setToolTip(
        "Show the PH spectra in keV instead of ADU.  With a gain map the "
        "pixel's own ADU/keV is used; without one the nominal 40 ADU/keV is "
        "used and the panel says so.");
    // usable without a map: the nominal 40 ADU/keV is used then
    gainMapVal_ = new QLabel("no gain map loaded", pedeGroup);
    btnNoPede_ = new QPushButton("Pede/Raw toggle", pedeGroup);
    btnNoPede_->setCheckable(true);
    btnNoPede_->setChecked(gNoPede.load()); // startup default: raw ADC
    btnNoPede_->setToolTip(
        "Switch between the loaded pedestal and raw ADC.  Pressed = raw ADC:\n"
        "the display, the photon map, the spectra, the event finder and the ROI\n"
        "save then work on raw 14-bit values, and the Pedestal box is greyed\n"
        "(this button stays active, it is how you switch back).");
    noPedeVal_ = new QLabel("pede subtracted", pedeGroup);
    pedeGrid->addWidget(btnLoadPede, 0, 0);
    pedeGrid->addWidget(btnSavePede, 0, 1);
    pedeGrid->addWidget(btnReadPede, 1, 0);
    pedeGrid->addWidget(btnLoadAllPede, 1, 1);
    // calibration for the keV spectra (the ADC plots stay in ADU)
    // (the gain map moved to the Gain panel below)
    pedeTrackGroup_ = new QGroupBox("Pedestal tracking", central);
    auto *trackCol = new QVBoxLayout(pedeTrackGroup_);
    btnPedeTrack_ =
        new QPushButton("pedestal tracking on/off", pedeTrackGroup_);
    btnPedeTrack_->setCheckable(true);
    btnPedeTrack_->setChecked(gPedeTracking.load()); // --pede-tracking
    btnPedeTrack_->setToolTip(
        "Follow the pedestal drift from the frames the signal gate drops\n"
        "(the beam is off then).  While the gate drops more than 90 % of the\n"
        "frames reaching it, 0.2 % of every dropped frame feeds a per-pixel\n"
        "running average over a 10 M sample window; the pedestal in use is\n"
        "replaced when the largest difference exceeds 5 ADU, at most once\n"
        "every 10 s and at least every 10 min.  Needs the signal gate on.");
    pedeTrackVal_ = makeValueLabel("off", pedeTrackGroup_);
    pedeGrid->addWidget(btnNoPede_, 2, 0);
    pedeGrid->addWidget(noPedeVal_, 2, 1);
    trackCol->addWidget(btnPedeTrack_);
    trackCol->addWidget(pedeTrackVal_);
    gainMapVal_->setWordWrap(true);
    pedePad_ = new QWidget(central);
    pedePad_->setFixedHeight(0);
    rightCol->addWidget(pedePad_);
    rightCol->addWidget(pedeGroup);
    // the tracker reads the frames the signal gate drops and feeds its result
    // back into the Pedestal box above it, so the arrow points up
    // the tracking feedback goes back into the pedestal, so it is pedestal path
    arrTrackUp_ = makeFlowArrow(QString::fromUtf8("\u2191"), central,
                                FlowPedestal);
    rightCol->addWidget(arrTrackUp_, 0, Qt::AlignHCenter);
    rightCol->addWidget(pedeTrackGroup_);
    gainPad_ = new QWidget(central);
    gainPad_->setFixedHeight(0);
    rightCol->addWidget(gainPad_);

    auto *gainGroup = new QGroupBox("Gain", central);
    gainGroup_ = gainGroup;
    auto *gainCol = new QVBoxLayout(gainGroup);
    {
        auto *gainRow = new QHBoxLayout();
        gainRow->addWidget(btnGainToggle);
        // Read-only state indicator (the button is the only control): it shows
        // at a glance whether the gain correction is applied - itself if a map
        // is loaded, the nominal 40 ADU/keV otherwise.
        gainStateChk_ = new QCheckBox("gain corrected", gainGroup);
        gainStateChk_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        gainStateChk_->setFocusPolicy(Qt::NoFocus);
        gainStateChk_->setToolTip(
            "Whether the gain correction is applied (read-only: switch it with\n"
            "the button on the left).  Ticked = pixels are divided by their\n"
            "ADU/keV, so the PH spectra can be shown in keV.");
        gainRow->addWidget(gainStateChk_);
        gainRow->addStretch(1);
        gainCol->addLayout(gainRow);
    }
    {
        auto *gmRow = new QHBoxLayout();
        gmRow->addWidget(new QLabel("Gain map (keV PH):", gainGroup));
        gmRow->addWidget(btnGainMap);
        gmRow->addWidget(kevChk_);
        gmRow->addStretch(1);
        gainCol->addLayout(gmRow);
    }
    gainCol->addWidget(gainMapVal_);
    rightCol->addWidget(gainGroup);
    arrGainDown_ = makeFlowArrow(QString::fromUtf8("\u2193"), central);
    rightCol->addWidget(arrGainDown_, 0, Qt::AlignHCenter);
    rightCol->addWidget(ctrlGroup);

    // ROI panel: selecting / loading / saving ROIs (the gate keeps its own
    // "ROI only" capture and its ROI read-out next to the signal gate)
    roiGroup_ = new QGroupBox("ROI", central);
    {
        auto *roiCol = new QVBoxLayout(roiGroup_);
        btnSaveRoi = new QPushButton("Save ROI", roiGroup_);
        btnLoadRoi = new QPushButton("Load ROI...", roiGroup_);
        btnSaveRoi->setToolTip(
            "Add the active gate ROI to the list kept in the program folder\n"
            "(updates its timestamp if the same rectangle is already there).");
        btnLoadRoi->setToolTip(
            "Pick a ROI from the list (most recently saved/loaded first) and\n"
            "apply it to the gate.  The list dialog can also delete entries.");
        btnSelectRoi_ = new QPushButton("Select ROI", roiGroup_);
        btnSelectRoi_->setCheckable(true);
        btnSelectRoi_->setToolTip(
            "Toggle the ROI editor: a yellow rectangle appears in the ADC\n"
            "image.  Drag inside it to move it, its edges/corners to resize it,\n"
            "or drag on empty space to draw a new one.  Selecting a ROI only\n"
            "stores it: press \"ROI only\" next to the signal gate to restrict\n"
            "the gate to it.");
        roiCol->addWidget(btnSelectRoi_);
        btnUnsetRoi_ = new QPushButton("Unset ROI", roiGroup_);
        btnUnsetRoi_->setToolTip(
            "Forget the current ROI: the gate and the saved data go back to the\n"
            "full frame (the ROI list is not touched - use \"Load ROI...\" for\n"
            "that).");
        roiCol->addWidget(btnUnsetRoi_);
        auto *row = new QHBoxLayout();
        row->addWidget(btnSaveRoi);
        row->addWidget(btnLoadRoi);
        roiCol->addLayout(row);
        roiListVal_ = makeValueLabel("no ROI in the list", roiGroup_);
        roiCol->addWidget(roiListVal_);
    }
    rightCol->addWidget(roiGroup_);
    // ROI -> Save: on the data path while "save frames" is selected (the
    // colour follows the save mode, see updateFlow)
    arrRoiDown_ = makeFlowArrow(QString::fromUtf8("\u2193"), central, FlowData);
    rightCol->addWidget(arrRoiDown_, 0, Qt::AlignHCenter);

    // the save panel spans the whole window, below the flow but above the
    // Status/Counters row
    rootLayout->addWidget(saveGroup);
    rootLayout->addLayout(bottomRow);

    btnQuit = new QPushButton("Quit (q)", central);
    // first row of the right column, right aligned: out of the gate/arrow area
    rightCol->insertWidget(0, btnQuit, 0, Qt::AlignRight);
    // (Quit was added at the top left, above the gates)
    rightCol->addStretch(1);

    // ── Signals: gate parameters ──
    connect(gateChk_, &QCheckBox::toggled, this, [this](bool on) {
        pipeline_.gate().setEnabled(on);
        statusLabel->setText(on ? "Status: gate enabled"
                                : "Status: passthrough (gate off)");
        // the tracking works off the frames the gate drops, so switching the
        // gate off turns it off too
        if (!on && btnPedeTrack_ && btnPedeTrack_->isChecked())
            btnPedeTrack_->setChecked(false);
    });
    connect(thrSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this](double v) {
                if (!confirmGateChangeDuringRun(
                        QString("Signal-gate threshold -> %1 ADU").arg(v))) {
                    const QSignalBlocker b(thrSpin_);
                    thrSpin_->setValue(
                        static_cast<double>(pipeline_.gate().threshold()));
                    return;
                }
                pipeline_.gate().setThreshold(static_cast<float>(v));
                pipeline_.gate().resetCounters();
            });
    connect(minPixSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int v) {
                if (!confirmGateChangeDuringRun(
                        QString("Min pixels above threshold -> %1").arg(v))) {
                    const QSignalBlocker b(minPixSpin_);
                    minPixSpin_->setValue(
                        static_cast<int>(pipeline_.gate().minPixels()));
                    return;
                }
                pipeline_.gate().setMinPixels(static_cast<uint64_t>(v));
                pipeline_.gate().resetCounters();
            });
    connect(contigChk_, &QCheckBox::toggled, this, [this](bool on) {
        pipeline_.gate().setContiguous(on);
        pipeline_.gate().resetCounters();
    });
    connect(hotChk_, &QCheckBox::toggled, this, [this](bool on) {
        if (!confirmGateChangeDuringRun(
                QString("Hot pixel suppression -> %1").arg(on ? "on" : "off"))) {
            const QSignalBlocker b(hotChk_);
            hotChk_->setChecked(pipeline_.gate().hotSuppress());
            return;
        }
        pipeline_.gate().setHotSuppress(on);
        pipeline_.gate().resetCounters();
    });
    connect(btnGetHot_, &QPushButton::clicked, this, [this]() {
        QDialog dlg(this);
        dlg.setWindowTitle("Get hot pixels from the photon map");
        auto *form = new QFormLayout(&dlg);
        form->addRow(new QLabel(QString("Photon map filled with %1 frames.")
                                    .arg(static_cast<qulonglong>(
                                        pipeline_.pmapFrames())),
                                &dlg));
        auto *spin = new QDoubleSpinBox(&dlg);
        spin->setRange(0.0, 1e9);
        spin->setDecimals(1);
        spin->setSingleStep(1.0);
        // Default: a tenth of the frames the map was accumulated over, i.e. a
        // pixel that fires in more than ~10 % of them.
        const double def =
            static_cast<double>(pipeline_.pmapFrames()) / 10.0;
        spin->setValue(def < 1.0 ? 1.0 : def);
        form->addRow("Marking pixels above:", spin);
        auto *bb = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        form->addRow(bb);
        connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        if (dlg.exec() != QDialog::Accepted)
            return;
        pipeline_.setHotMaskFromPmap(spin->value());
        statusLabel->setText(
            QString("Status: %1 hot pixels (photon map > %2)")
                .arg(static_cast<qulonglong>(pipeline_.hotPixels()))
                .arg(spin->value(), 0, 'f', 1));
    });
    connect(btnResetHot_, &QPushButton::clicked, this, [this]() {
        hotClearMask();
        pipeline_.gate().resetCounters();
        statusLabel->setText("Status: hot pixel mask cleared");
    });
    connect(btnResetCounters, &QPushButton::clicked, this,
            [this]() { pipeline_.gate().resetCounters(); });
    connect(scCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                // index 0 is "ALL" (no storage-cell gating), 1..16 are SC 0..15
                const int sc = (idx <= 0) ? -1 : idx - 1;
                pipeline_.gate().setScFilter(sc);
                statusLabel->setText(
                    sc < 0 ? "Status: all storage cells"
                           : QString("Status: only storage cell %1").arg(sc));
            });
    connect(btnGateRoi, &QPushButton::toggled, this,
            [this](bool on) { toggleGateRoi(on); });
    connect(btnSaveStart, &QPushButton::toggled, this,
            [this](bool on) { toggleRoiSave(on); });
    connect(saveModePhoton_, &QRadioButton::toggled, this, [this](bool on) {
        gSaveMode = on ? 1 : 0;
        pmapSaveSpin_->setEnabled(on);
        // in photon mode the same spinbox budgets MAPS, so say so
        if (saveBudgetLabel_)
            saveBudgetLabel_->setText(
                on ? "Number of photon maps to be saved:"
                   : "Number of frames to be saved:");
        // the correction only applies to the frames mode
        if (saveCorrected_)
            saveCorrected_->setEnabled(!on &&
                                       !pipeline_.saver().status().running);
        updateFlow(); // the data-path arrows follow the mode
        printf("[save] mode: %s\n",
               on ? "virtual photon counting" : "frames");
        fflush(stdout);
        statusLabel->setText(on
            ? "Status: saving virtual photon counting maps"
            : "Status: saving gate-passed frames");
    });
    connect(pmapSaveSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int v) {
                if (v < 200) {
                    pmapSaveSpin_->setValue(200);
                    return;
                }
                gPmapSaveEvery = v;
            });
    connect(dirBtn, &QPushButton::clicked, this,
            [this]() { browseSaveDir(); });
    connect(btnSaveDest_, &QPushButton::clicked, this,
            [this]() { saveDestDialog(); });
    connect(btnSelectRoi_, &QPushButton::toggled, this,
            [this](bool on) { setRoiEditMode(on); });
    connect(btnUnsetRoi_, &QPushButton::clicked, this,
            [this]() { unsetRoi(); });
    connect(btnSaveRoi, &QPushButton::clicked, this, [this]() { saveRoi(); });
    connect(btnLoadRoi, &QPushButton::clicked, this,
            [this]() { loadRoiDialog(); });

    // ── Signals: everything else ──
    connect(btnStartPause, &QPushButton::clicked, this,
            [this]() { toggleStartPause(); });
    connect(btnFaster, &QPushButton::clicked, this, [this]() { faster(); });
    connect(btnSlower, &QPushButton::clicked, this, [this]() { slower(); });
    connect(btnGainToggle, &QPushButton::toggled, this,
            [this](bool on) { setApplyGain(on); });
    connect(btnBWPalette, &QPushButton::clicked, this,
            [this]() { bwPalette1(); });
    connect(btnBWPalette2, &QPushButton::clicked, this,
            [this]() { bwPalette2(); });
    connect(btnSetRanges, &QPushButton::clicked, this,
            [this]() { showRangeDialog(); });
    connect(btnRates, &QPushButton::clicked, this,
            [this]() { showRateDialog(); });
    connect(pmapCumulative_, &QCheckBox::toggled, this, [this](bool on) {
        gPmapCumulative = on;
        // "toggle to reset": either way the map starts from scratch, so the
        // switch itself is the reset (no need to reach for Reset His)
        resetHis();
        statusLabel->setText(on ? "Status: photon map cumulative (restarted)"
                                : "Status: photon map per image (restarted)");
    });
    connect(pmapEverySpin_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int v) {
                gPmapEvery = v > 0 ? v : 1;
                statusLabel->setText(
                    QString("Status: one photon map every %1 analysed frames")
                        .arg(v));
            });
    connect(photonThrSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double v) {
                // the photon map is filled by the analysis thread, which reads
                // the atomic every frame; the multiplicity follows as 2 x v.
                // Which one is being edited depends on the gain stage.
                const bool kev = gain_flag.load();
                if (kev)
                    g_pheneThresholdKev = static_cast<float>(v);
                else
                    g_pheneThreshold = static_cast<float>(v);
                statusLabel->setText(
                    QString("Status: photon-map threshold %1 %2")
                        .arg(v, 0, 'f', kev ? 1 : 0)
                        .arg(kev ? "keV" : "ADU"));
            });
    // "Pedestal corrected" decides whether the *saved* frames use the pedestal,
    // which is what the orange route in the flow diagram shows
    connect(saveCorrected_, &QCheckBox::toggled, this, [this](bool on) {
        updateFlow();
        statusLabel->setText(
            on ? "Status: saved frames pedestal corrected"
               : "Status: saved frames raw (pedestal not subtracted)");
    });
    connect(btnResetHis, &QPushButton::clicked, this,
            [this]() { resetHis(); });
    connect(btnResetAxis, &QPushButton::clicked, this,
            [this]() { resetAxis(); });
    connect(btnColPal, &QPushButton::clicked, this,
            [this]() { colPal(); });
    connect(btnLoadPede, &QPushButton::clicked, this,
            [this]() { acquirePede(); });
    connect(btnSavePede, &QPushButton::clicked, this,
            [this]() { savePede(); });
    connect(btnReadPede, &QPushButton::clicked, this,
            [this]() { readPede(); });
    connect(btnLoadAllPede, &QPushButton::clicked, this,
            [this]() { loadAllPede(); });
    connect(btnPedeTrack_, &QPushButton::toggled, this, [this](bool on) {
        // needs the signal gate: without it "dropped" has no meaning
        if (on && gateChk_ && !gateChk_->isChecked()) {
            QMessageBox::information(
                this, "Pedestal tracking",
                "Pedestal tracking follows the frames the signal gate drops, "
                "so it needs \"Gate enabled\" on.");
            btnPedeTrack_->setChecked(false);
            return;
        }
        gPedeTracking = on;
        printf("[pedetrack] %s\n", on ? "on" : "off");
        fflush(stdout);
        statusLabel->setText(on ? "Status: pedestal tracking on"
                                : "Status: pedestal tracking off");
        updateFlow();
    });
    connect(btnNoPede_, &QPushButton::toggled, this, [this](bool on) {
        gNoPede = on;
        updateFlow();
        statusLabel->setText(on ? "Status: pedestal NOT subtracted (raw ADC)"
                                : "Status: pedestal subtraction on");
    });
    connect(btnGainMap, &QPushButton::clicked, this,
            [this]() { loadGainMap(); });
    connect(kevChk_, &QCheckBox::toggled, this, [this](bool on) {
        kevSpectra = on;
        forceRootRedraw();
        updateFlow(); // the Gain panel is in use when keV is shown
        if (on && !gainMapLoaded()) {
            if (gainMapVal_)
                gainMapVal_->setText(
                    "no gain map selected - using nominal values "
                    "(40 ADU/keV)");
            statusLabel->setText(
                "Status: PH spectra in keV (nominal 40 ADU/keV, no gain map)");
        } else {
            updateGainMapLabel();
            statusLabel->setText(on ? "Status: PH spectra in keV"
                                    : "Status: PH spectra in ADU");
        }
    });
    connect(btnQuit, &QPushButton::clicked, this, [this]() { quitApp(); });

    // ── Fast image views (separate top-level windows, as in the original) ──
    m_adcView = new FastImageView();
    m_adcView->setPaletteLut(paletteLut(Palette::Color));
    if (m_adcScale_)
        m_adcScale_->setPaletteChoice(0); // match the ROOT plots
    // the ADC (main) image opens on the ADC range of the Set Ranges dialog
    m_adcView->setColorRange(adcmin.load(), adcmax.load());
    m_adcWin = new QWidget(nullptr, Qt::Window);
    m_adcWin->setWindowTitle("ADC Image");
    m_adcWin->resize(1024 + 80, 512 + 40 + 90); // + the scale panel
    auto *adcLay = new QVBoxLayout(m_adcWin);
    adcLay->setContentsMargins(0, 0, 0, 0);
    adcLay->addWidget(m_adcView, 1);
    m_adcScale_ = new ScalePanel(m_adcView, m_adcWin);
    m_adcView->onScaleReleased = [this]() { m_adcScale_->refresh(); };
    m_adcView->onRoiEdited = [this](int x0, int y0, int x1, int y1) {
        applyRoi(x0, y0, x1 - x0, y1 - y0, "ADC image", false);
    };
    adcLay->addWidget(m_adcScale_, 0);
    m_adcWin->show();

    m_gainView = new FastImageView();
    // The gain image has four valid values, so it gets a fixed four-colour code
    // (0 blue, 1 green, 2 yellow, 3 red, all desaturated) and no scale panel:
    // there is no colour scale to choose and no distribution worth showing.
    m_gainView->setPaletteLut(paletteLut(Palette::Gain4));
    m_gainView->setIntegerValues(true);
    m_gainView->setColorRange(0, 3);
    m_gainWin = new QWidget(nullptr, Qt::Window);
    m_gainWin->setWindowTitle("Gain Image");
    m_gainWin->resize(1024 + 80, 512 + 40); // no scale panel here
    auto *gainLay = new QVBoxLayout(m_gainWin);
    gainLay->setContentsMargins(0, 0, 0, 0);
    gainLay->addWidget(m_gainView, 1);
    m_gainWin->hide(); // hidden by default: "show gain image" in Display

    m_photonView = new FastImageView();
    m_photonView->setPaletteLut(paletteLut(Palette::Color));
    m_photonView->setZeroWhite(true); // 0 photons = white, like the ROOT plot
    m_photonView->setColorRange(0, 8); // photons per pixel over one image
    m_photonWin = new QWidget(nullptr, Qt::Window);
    m_photonWin->setWindowTitle("Photon Map (virtual photon counting)");
    m_photonWin->resize(1024 + 80, 512 + 40 + 90);
    auto *photonLay = new QVBoxLayout(m_photonWin);
    photonLay->setContentsMargins(0, 0, 0, 0);
    m_photonView->setIntegerValues(true); // photon counts: whole numbers
    photonLay->addWidget(m_photonView, 1);
    m_photonScale_ = new ScalePanel(m_photonView, m_photonWin);
    m_photonView->onScaleReleased = [this]() { m_photonScale_->refresh(); };
    photonLay->addWidget(m_photonScale_, 0);
    m_photonWin->show();

#ifdef JFSTRX
    // ── Separate strixel window: the six ADC groups, arranged like the
    // original ROOT ADC canvas (left column chip1 g3/g2/g1, right column
    // chip6 g1/g2/g3).  Display only: the gate ROI is selected on the raw
    // ADC image, which stays unmapped so the coordinates are raw pixels.
    m_adcPanelWin = new QWidget(nullptr, Qt::Window);
    m_adcPanelWin->setWindowTitle("ADC Strixel Panels (JFSTRX)");
    m_adcPanelWin->resize(900, 900);
    auto *panelOuter = new QVBoxLayout(m_adcPanelWin);
    panelOuter->setContentsMargins(4, 2, 4, 2);
    panelOuter->setSpacing(2);
    m_adcPanelZ = new QLabel(m_adcPanelWin);
    m_adcPanelZ->setAlignment(Qt::AlignCenter);
    m_adcPanelZ->setToolTip(
        "The six strixel panels use the ADC image's Z scale (colour range and\n"
        "palette): change it in the ADC window's scale panel and they follow.");
    panelOuter->addWidget(m_adcPanelZ);
    auto *panelRow = new QHBoxLayout();
    panelOuter->addLayout(panelRow, 1);
    panelRow->setContentsMargins(2, 2, 2, 2);
    auto *panelCol0 = new QVBoxLayout();
    auto *panelCol1 = new QVBoxLayout();
    panelRow->addLayout(panelCol0);
    panelRow->addLayout(panelCol1);
    QVBoxLayout *panelCol[2] = {panelCol0, panelCol1};
    // panel index order: 0..2 = chip1 g1/g2/g3, 3..5 = chip6 g1/g2/g3
    static const int panelOrder[2][3] = {{2, 1, 0}, {3, 4, 5}}; // top -> bottom
    static const int panelStretch[2][3] = {{5, 2, 2}, {2, 2, 5}};
    // mapped-pixel aspect X/Y: X = 75*(75/Y), Y = 25/15/18.75 um
    // -> 225/25 = 9, 375/15 = 25, 300/18.75 = 16
    static const double panelAspect[kNumAdcPanels] = {9.0, 25.0, 16.0,
                                                      9.0, 25.0, 16.0};
    static const char *panelTitle[kNumAdcPanels] = {
        "chip 1 group 1 (25um)", "chip 1 group 2 (15um)",
        "chip 1 group 3 (18.75um)", "chip 6 group 1 (25um)",
        "chip 6 group 2 (15um)", "chip 6 group 3 (18.75um)"};
    for (int col = 0; col < 2; col++) {
        for (int k = 0; k < 3; k++) {
            const int p = panelOrder[col][k];
            auto *cell = new QWidget();
            auto *cv = new QVBoxLayout(cell);
            cv->setContentsMargins(0, 0, 0, 0);
            cv->setSpacing(0);
            auto *lab = new QLabel(panelTitle[p], cell);
            lab->setAlignment(Qt::AlignCenter);
            cv->addWidget(lab);
            auto *v = new FastImageView();
            v->setPaletteLut(paletteLut(Palette::Color));
            v->setColorRange(adcmin.load(), adcmax.load());
            v->setPixelAspectX(panelAspect[p]);
            cv->addWidget(v, 1);
            panelCol[col]->addWidget(cell, panelStretch[col][k]);
            m_adcPanelView[p] = v;
        }
    }
    // The strixel panels have no scale of their own: they follow the ADC
    // image's colour range and palette, which is where the Z scale lives.
    auto syncPanels = [this]() {
        double lo = 0.0, hi = 1.0;
        m_adcView->colorRange(lo, hi);
        for (auto *v : m_adcPanelView)
            if (v)
                v->setColorRange(lo, hi);
        if (m_adcPanelZ)
            m_adcPanelZ->setText(
                QString("Z scale: follows the ADC image  (Z: %1 .. %2)")
                    .arg(lo, 0, 'g', 5)
                    .arg(hi, 0, 'g', 5));
    };
    m_adcView->onColorRangeChanged = [syncPanels](double, double) {
        syncPanels();
    };
    // only the ADC view propagates: the panels must not re-emit (that would
    // bounce the palette back and forth for ever)
    m_adcView->onPaletteChanged = [this](const QRgb *lut) {
        for (auto *w : m_adcPanelView)
            if (w)
                w->setPaletteLut(lut);
    };
    syncPanels();
    m_adcPanelWin->show();
#endif

    // Master-slave zoom: the ADC view is the master; the gain view follows
    // immediately (same thread) and the ROOT photon map follows through a
    // coalesced command, so only the newest viewport is ever redrawn.
    // all three image windows report their zoom; zoomTo() decides what follows
    auto wireZoom = [this](FastImageView *v) {
        if (!v)
            return;
        v->onZoomChanged = [this, v](double x1, double y1, double x2,
                                     double y2) { zoomTo(v, x1, y1, x2, y2); };
    };
    wireZoom(m_adcView);
    wireZoom(m_gainView);
    wireZoom(m_photonView);

    // ── Timers (started by start()) ──
    processTimer = new QTimer(this);
    connect(processTimer, &QTimer::timeout, this,
            [this]() { processImages(); });
    statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, [this]() { updateStatus(); });
    sigTimer = new QTimer(this);
    connect(sigTimer, &QTimer::timeout, this, [this]() { checkSignals(); });

    updateFlow();

    updateDrawLabel();
}

void MainWindow::connectShortcuts() {
    auto sc = [this](const char *key, const std::function<void()> &fn) {
        QShortcut *s = new QShortcut(QKeySequence(QString::fromLatin1(key)),
                                     this);
        connect(s, &QShortcut::activated, this, fn);
    };
    sc("s", [this]() { toggleStartPause(); });
    sc("+", [this]() { faster(); });
    sc("=", [this]() { faster(); });
    sc("-", [this]() { slower(); });
    sc("g", [this]() { toggleGain(); });
    sc("b", [this]() { bwPalette1(); });
    sc("Shift+B", [this]() { bwPalette2(); });
    sc("c", [this]() { colPal(); });
    sc("i", [this]() { btnGateRoi->setChecked(!btnGateRoi->isChecked()); });
    sc("e", [this]() {
        // keV PH (e for energy): keeps its tick while the gain is off, see
        // updateFlow.  Not "k": that is already Load ROI.
        if (kevChk_)
            kevChk_->setChecked(!kevChk_->isChecked());
    });
    sc("n", [this]() { btnSaveStart->setChecked(!btnSaveStart->isChecked()); });
    sc("j", [this]() { saveRoi(); });
    sc("k", [this]() { loadRoiDialog(); });
    sc("r", [this]() { resetHis(); });
    sc("Shift+R", [this]() { resetAxis(); });
    sc("p", [this]() { acquirePede(); });
    sc("Shift+O", [this]() { savePede(); });
    sc("o", [this]() { readPede(); });
    sc("Shift+P", [this]() { loadAllPede(); });
    sc("q", [this]() { quitApp(); });
    sc("Shift+S", [this]() { showRangeDialog(); });
}

// ---------------------------------------------------------------------------
// lifecycle
// ---------------------------------------------------------------------------
void MainWindow::start() {
    // The ROOT canvases are hidden by default ("show ROOT plots" in Display);
    // the command is queued here because the ROOT thread runs by now.
    {
        RootCmd c;
        c.type = RootCmdType::ShowPlots;
        c.a = 0.0;
        pushRootCmd(c);
    }
    // Pedestals must be installed before the threads start reading frames.
    std::string perr;
    if (pathExists(g_cfg.pedeFile)) {
        if (pedeLoadFile(g_cfg.pedeFile, perr)) {
            printf("[pede] loaded %s\n", g_cfg.pedeFile.c_str());
            pedeVal_->setText(QString("loaded (%1)").arg(
                QString::fromStdString(g_cfg.pedeFile)));
            pipeline_.gate().setLastError("");
        } else {
            fprintf(stderr, "[pede] %s\n", perr.c_str());
            pipeline_.gate().setLastError("loadPedestal: " + perr);
        }
    } else if (g_cfg.pedeExplicit) {
        fprintf(stderr, "[pede] file not found: %s\n", g_cfg.pedeFile.c_str());
        pipeline_.gate().setLastError("loadPedestal: file not found: " +
                                      g_cfg.pedeFile);
    } else {
        printf("[pede] %s not found, running on raw values\n",
               g_cfg.pedeFile.c_str());
    }

    if (!g_cfg.gainMapFile.empty()) {
        std::string gerr;
        if (gainMapLoadFile(g_cfg.gainMapFile, gerr)) {
            if (gainMapVal_)
                gainMapVal_->setText(QFileInfo(QString::fromStdString(
                                                   g_cfg.gainMapFile))
                                         .fileName());
            if (kevChk_)
                kevChk_->setChecked(true); // show what was just calibrated
            updateFlow();
        } else {
            fprintf(stderr, "[gain] %s\n", gerr.c_str());
            pipeline_.gate().setLastError("gainMap: " + gerr);
        }
    }

    std::string err;
    if (!pipeline_.init(err)) {
        fprintf(stderr, "FATAL: %s\n", err.c_str());
        pipeline_.gate().setLastError(err);
        statusLabel->setText("Status: socket error");
        QMessageBox::critical(this, "ZMQ", QString::fromStdString(err));
    } else {
        pipeline_.start();
        pipeline_.gate().setScFilter(g_cfg.scFilter);
        if (scCombo_)
            scCombo_->setCurrentIndex(g_cfg.scFilter < 0 ? 0
                                                         : g_cfg.scFilter + 1);
        statusLabel->setText("Status: running");
        if (g_cfg.acquirePede)
            pipeline_.requestPedeAcquire(g_cfg.pedeFrames);
        if (g_cfg.gateRoiX >= 0) { // --gate-roi: same path as the "ROI only"
                                   // button, so the box and the gate agree
            applyRoi(g_cfg.gateRoiX, g_cfg.gateRoiY, g_cfg.gateRoiW,
                     g_cfg.gateRoiH, "command line", /*restrict=*/true);
            // reflect the restriction in the button, so the first press turns
            // it off instead of toggling the wrong way (m_applyingRoi keeps the
            // toggle handler from re-capturing or warning)
            if (btnGateRoi && pipeline_.gate().roiActive() &&
                !btnGateRoi->isChecked()) {
                m_applyingRoi = true;
                btnGateRoi->setChecked(true);
                m_applyingRoi = false;
            }
        }
    }

    startRootThread();
    processTimer->start(30);
    statusTimer->start(500);
    sigTimer->start(200);

    // --apply-gain: same as pressing the Gain button (it also swaps the photon
    // threshold to keV and relabels its row)
    if (g_cfg.applyGain)
        btnGainToggle->setChecked(true);

    // --start-save: press the Save button for scripted/headless runs.  The
    // check triggers toggleRoiSave() exactly like the GUI click.
    if (g_cfg.startSave) {
        printf("[save] --start-save: starting the %s writer\n",
               gSaveMode.load() == 1 ? "virtual photon counting"
                                     : "frame");
        fflush(stdout);
        btnSaveStart->setChecked(true);
    }

    signal(SIGINT, handleSigint);
    signal(SIGTERM, handleSigint);
}

void MainWindow::stop() {
    if (processTimer)
        processTimer->stop();
    if (statusTimer)
        statusTimer->stop();
    if (sigTimer)
        sigTimer->stop();
    {
        const RoiSaver::Status st = pipeline_.saver().status();
        if (st.running) {
            printf("[shutdown] flushing the ROI save (%llu frames, %s)...\n",
                   (unsigned long long)st.saved, st.h5Path.c_str());
            fflush(stdout);
            if (statusLabel)
                statusLabel->setText(
                    "Status: flushing the ROI save before exit...");
        }
    }
    pipeline_.saver().requestStop("program exit");
    pipeline_.saver().finish(); // drain the queue, close the file, write JSON
    pipeline_.stop();
    stopRootThread();
}

// Question asked before quitting (both from the Quit button/shortcut and from
// closing the window).  A running ROI save is called out explicitly, because
// quitting flushes it: the writer drains its queue, closes the HDF5 file and
// writes the JSON sidecar before the program exits.
bool MainWindow::confirmQuit() {
    const RoiSaver::Status st = pipeline_.saver().status();
    QString text = "Quit the online display?";
    if (st.running) {
        text += QString("\n\nA ROI save is still running: %1 frames written "
                        "to\n%2\n\nIt will be stopped and flushed (queued "
                        "frames written, file closed, JSON sidecar written) "
                        "before the program exits.")
                    .arg(static_cast<qulonglong>(st.saved))
                    .arg(QString::fromStdString(st.h5Path));
    } else if (!st.h5Path.empty()) {
        text += QString("\n\nThe last ROI save is closed:\n%1\n(%2 frames)")
                    .arg(QString::fromStdString(st.h5Path))
                    .arg(static_cast<qulonglong>(st.saved));
    }
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this, "Quit", text, QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    printf("[quit] %s\n", answer == QMessageBox::Yes ? "confirmed"
                                                     : "cancelled");
    fflush(stdout);
    return answer == QMessageBox::Yes;
}

// Sizes the two empty pads that turn the panels into a flow chart:
//   * the gates start low enough that the middle of "1. Storage cell" sits on
//     the top edge of the Pedestal box, and
//   * the Gain box starts at the middle of "2. Signal gate".
// The only thing that matters is that the two columns stay top-aligned, which
// they do, so this runs once at start-up and again if the window is resized
// wider/narrower (text wrapping can change the heights).
void MainWindow::layoutFlow() {
    if (!centralWidget() || !leftPad_ || !gainPad_ || !scGroup_ ||
        !sigGroup_ || !pedeGroup_ || !gainGroup_)
        return;
    static bool inLayout = false;
    if (inLayout)
        return; // setFixedHeight() re-enters through resizeEvent()
    inLayout = true;
    flowQueued_ = false; // this call is the queued one, if it was

    // The first call comes before Qt has computed the layout, when every box
    // still has a token size; solving from that produced a huge pad and, with
    // it, a window minimum that stayed large for the rest of the session.
    if (scGroup_->height() < 40 || sigGroup_->height() < 40 ||
        pedeGroup_->height() < 40 || gainGroup_->height() < 40) {
        inLayout = false;
        return;
    }

    // Iterative, because every change alters what is being measured (boxes
    // grow when their labels wrap, and the pads move the boxes).  Three knobs:
    //   leftPad   - pushes the gates down until the storage-cell middle is on
    //               the top edge of the Pedestal
    //   gatePad   - the gap *between* the two gates: stretching it moves the
    //               signal gate down without touching the storage cell
    //   gainPad   - fine adjustment so the Gain starts at the signal-gate middle
    const int arrowMin = 10; // keep the arrows visible
    leftPad_->setFixedHeight(flowPads_[0]);
    pedePad_->setFixedHeight(flowPads_[1]);
    gainPad_->setFixedHeight(flowPads_[2]);
    if (gatePad_)
        gatePad_->setFixedHeight(flowPads_[3]);
    if (QLayout *l = centralWidget()->layout())
        l->activate();

    {
        const int scMid = scGroup_->mapTo(centralWidget(),
                                          QPoint(0, scGroup_->height() / 2))
                              .y();
        const int sigMid = sigGroup_->mapTo(centralWidget(),
                                            QPoint(0, sigGroup_->height() / 2))
                               .y();
        const int pedeTop =
            pedeGroup_->mapTo(centralWidget(), QPoint(0, 0)).y();
        const int gainTop =
            gainGroup_->mapTo(centralWidget(), QPoint(0, 0)).y();

        // storage-cell middle must sit on the Pedestal's top edge: move
        // whichever of the two has room (the Pedestal down, or the gates down)
        int wantLeft = flowPads_[0], wantPede = flowPads_[1];
        const int errPede = scMid - pedeTop; // >0: the Pedestal is too high
        if (errPede > 0)
            wantPede += errPede;
        else
            wantLeft += -errPede;
        if (wantLeft < 0)
            wantLeft = 0;
        if (wantPede < 0)
            wantPede = 0;
        // The Gain must start at the signal gate's middle.  If it sits too
        // high, push it down with gainPad; if it sits too low, push the signal
        // gate down instead by stretching the arrow between the gates (the
        // storage cell above is not affected by that arrow).
        const int err = sigMid - gainTop; // >0: the gain is too high
        int wantArrow = flowPads_[3];
        int wantGain = flowPads_[2];
        if (err > 0) {
            wantGain += err; // the Gain sits above the signal middle
        } else if (err < 0) {
            wantArrow += -err; // below it: push the signal gate down instead
            wantGain = 0;
        }
        // err == 0: aligned, leave both pads alone (resetting gainPad here was
        // what made the solver oscillate between aligned and 50 px off)
        if (wantArrow < arrowMin)
            wantArrow = arrowMin;
        if (wantGain < 0)
            wantGain = 0;

        const bool converged = wantLeft == flowPads_[0] &&
                               wantPede == flowPads_[1] &&
                               wantArrow == flowPads_[3] &&
                               wantGain == flowPads_[2];
        flowPads_[0] = wantLeft;
        flowPads_[1] = wantPede;
        flowPads_[2] = wantGain;
        flowPads_[3] = wantArrow;
        if (!converged && getenv("JF_TIMING"))
            printf("[flow] pass %d: left %d pede %d gain %d gateArrow %d | "
                   "scMid %d sigMid %d pedeTop %d gainTop %d\n",
                   flowPass_, wantLeft, wantPede, wantGain, wantArrow, scMid,
                   sigMid, pedeTop, gainTop);
        if (!converged) {
            // re-measure on the next event-loop turn: within one call the
            // widget positions are not guaranteed to have settled yet, and
            // solving from stale numbers compounds the corrections
            flowPass_++;
            if (flowPass_ < 12 && !flowQueued_) {
                flowQueued_ = true;
                QTimer::singleShot(0, this, &MainWindow::layoutFlow);
            }
        } else {
            flowPass_ = 0;
        }
    }
    leftPad_->setFixedHeight(flowPads_[0]);
    pedePad_->setFixedHeight(flowPads_[1]);
    gainPad_->setFixedHeight(flowPads_[2]);
    if (gatePad_)
        gatePad_->setFixedHeight(flowPads_[3]);
    if (QLayout *l = centralWidget()->layout())
        l->activate();

    // place the strip over the gap between the two columns, spanning from the
    // top of the first box to the bottom of the last one
    if (strip_ && scGroup_ && sigGroup_ && pedeGroup_ && gainGroup_) {
        const int x0 = scGroup_->mapTo(centralWidget(), QPoint(0, 0)).x() +
                       scGroup_->width() + 1;
        const int x1 =
            pedeGroup_->mapTo(centralWidget(), QPoint(0, 0)).x() - 1;
        int top = scGroup_->mapTo(centralWidget(), QPoint(0, 0)).y();
        int bot = pedeGroup_->mapTo(centralWidget(), QPoint(0, 0)).y() +
                  pedeGroup_->height();
        const int gainBot = gainGroup_->mapTo(centralWidget(), QPoint(0, 0)).y() +
                            gainGroup_->height();
        if (gainBot > bot)
            bot = gainBot;
        if (roiGroup_) {
            const int roiBot =
                roiGroup_->mapTo(centralWidget(), QPoint(0, 0)).y() +
                roiGroup_->height();
            if (roiBot > bot)
                bot = roiBot;
        }
        if (x1 > x0 && bot > top)
            strip_->setGeometry(x0, top - 6, x1 - x0, bot - top + 12);
        strip_->raise();
        strip_->show();
    }

    // hand the strip its connectors, in its own coordinates
    if (strip_ && scGroup_ && sigGroup_ && pedeGroup_ && gainGroup_ &&
        pmapGroup_ && displayGroup_) {
        auto yIn = [&](QWidget *w, int y) {
            return w->mapTo(centralWidget(), QPoint(0, 0)).y() + y;
        };
        const int stripTop =
            strip_->mapTo(centralWidget(), QPoint(0, 0)).y();
        // The signal gate does not change the data, so nothing enters or
        // leaves its box: the flow runs from the previous stage to the Gain and
        // simply passes the gate, which is drawn as a funnel on that line (the
        // gate panel only points at the funnel - see FlowStrip::drawFunnel).
        const int funnelY =
            yIn(sigGroup_, sigGroup_->height() / 2) - stripTop;
        FlowStrip::Path a, b, c;
        // 1. storage cell (middle) -> Pedestal (middle): enter from the left
        a.yFrom = yIn(scGroup_, scGroup_->height() / 2) - stripTop;
        a.yTo = yIn(pedeGroup_, pedeGroup_->height() / 2) - stripTop;
        a.dir = +1;
        a.cls = FlowPedestal; // into the Pedestal
        // Pedestal (bottom) -> the funnel, travelling leftwards (no head: it
        // merges into the funnel)
        b.yFrom = yIn(pedeGroup_, pedeGroup_->height()) - stripTop;
        b.yTo = funnelY;
        b.dir = -1;
        b.x = FlowStrip::kFunnelX;
        b.head = false;
        // the funnel -> Gain (top): the flow continues after the gate
        c.yFrom = funnelY;
        c.yTo = yIn(gainGroup_, 0) - stripTop;
        c.dir = +1;
        c.x = FlowStrip::kFunnelX;
        c.startAtJog = true;
        // Gain -> ROI: a rail just left of the right column, so it does not
        // have to cross the Display panel.  Frames are the ROI-cropped
        // product, so this is the frames-mode input of the ROI.
        FlowStrip::Path d;
        d.yFrom = yIn(gainGroup_, gainGroup_->height() / 2) - stripTop;
        d.yTo = roiGroup_ ? (yIn(roiGroup_, roiGroup_->height() / 2) - stripTop)
                          : 0;
        d.dir = 2;
        d.on = roiGroup_ != nullptr;
        d.cls = FlowData; // corrected when setSaveMode() knows the mode
        // the gate block -> Pedestal tracking: the tracker lives off the frames
        // the gate *drops*, so it leaves the gate block's top-right corner
        FlowStrip::Path e;
        e.yFrom = yIn(sigGroup_, 6) - stripTop;
        e.yTo = pedeTrackGroup_
                    ? (yIn(pedeTrackGroup_, pedeTrackGroup_->height() / 2) -
                       stripTop)
                    : 0;
        e.dir = +1;
        e.on = pedeTrackGroup_ != nullptr;
        e.cls = FlowPedestal; // the tracker is the pedestal's own feedback
        // Gain -> Virtual photon counting: the calibration reaches the counting
        // (its threshold is keV while the gain correction is on).  The map is
        // built in the left column, so the connector travels leftwards.
        FlowStrip::Path f;
        f.yFrom = yIn(gainGroup_, gainGroup_->height() - 8) - stripTop;
        f.yTo = pmapGroup_ ? (yIn(pmapGroup_, 24) - stripTop) : 0;
        f.dir = -1;
        f.on = pmapGroup_ != nullptr;
        f.cls = FlowNormal; // flipped by setSaveMode()
        // Virtual photon counting -> Display: the map is shown (in its own
        // window), so this one travels rightwards into the Display.
        FlowStrip::Path g;
        g.yFrom = pmapGroup_
                      ? (yIn(pmapGroup_, pmapGroup_->height() - 8) - stripTop)
                      : 0;
        g.yTo = displayGroup_
                    ? (yIn(displayGroup_, displayGroup_->height() / 2) - stripTop)
                    : 0;
        g.dir = +1;
        g.on = pmapGroup_ != nullptr && displayGroup_ != nullptr;
        // counting -> ROI: with the maps as the saved product this is the
        // input of the ROI (they are cropped to it on the way out)
        FlowStrip::Path i;
        i.yFrom = yIn(pmapGroup_, pmapGroup_->height() - 8) - stripTop;
        i.yTo = roiGroup_ ? (yIn(roiGroup_, roiGroup_->height() / 2) - stripTop)
                          : 0;
        i.dir = +1;
        i.cls = FlowData;
        i.on = roiGroup_ != nullptr;
        // raw mode: the storage cell feeds the funnel directly, so the flow
        // does not have to pass through the (greyed) Pedestal box.  Both
        // feeders are always drawn; setStages() decides which one is live.
        FlowStrip::Path h;
        h.yFrom = yIn(scGroup_, scGroup_->height()) - stripTop;
        h.yTo = funnelY;
        h.dir = +1;
        h.x = FlowStrip::kFunnelX;
        h.head = false; // merges into the funnel
        strip_->setPaths(a, b, c, d, e, f, g, h, i, funnelY);
        bool dp, sp, go, pm;
        flowStageStates(dp, sp, go, pm);
        strip_->setStages(dp, sp, go, pm);
    }
    inLayout = false;

    if (getenv("JF_TIMING")) {
        printf("[flow] pads: gates %d pede %d gain %d gateArrow %d (passes "
               "%d)\n",
               flowPads_[0], flowPads_[1], flowPads_[2], flowPads_[3],
               flowPass_);
        auto rect = [&](QWidget *w) {
            return w ? QString("%1,%2 %3x%4")
                           .arg(w->mapTo(centralWidget(), QPoint(0, 0)).x())
                           .arg(w->mapTo(centralWidget(), QPoint(0, 0)).y())
                           .arg(w->width())
                           .arg(w->height())
                           .toStdString()
                     : std::string("-");
        };
        printf("[rect] sc %s | signal %s | pede %s | track %s | gain %s | "
               "roi %s\n",
               rect(scGroup_).c_str(), rect(sigGroup_).c_str(),
               rect(pedeGroup_).c_str(), rect(pedeTrackGroup_).c_str(),
               rect(gainGroup_).c_str(), rect(roiGroup_).c_str());
        printf("[rect] gainToggle %s | keVchk %s | noPede %s | saveStart %s | "
               "selectRoi %s | saveRoi %s | quit %s | track %s | pmap %s\n",
               rect(btnGainToggle).c_str(), rect(kevChk_).c_str(),
               rect(btnNoPede_).c_str(), rect(btnSaveStart).c_str(),
               rect(btnSelectRoi_).c_str(), rect(btnSaveRoi).c_str(),
               rect(btnQuit).c_str(), rect(btnPedeTrack_).c_str(),
               rect(pmapGroup_).c_str());
        printf("[rect] unsetRoi %s | selectRoi %s | thr %s | minPix %s | "
               "zoomCoord %s\n",
               rect(btnUnsetRoi_).c_str(), rect(btnSelectRoi_).c_str(),
               rect(thrSpin_).c_str(), rect(minPixSpin_).c_str(),
               rect(zoomCoordChk_).c_str());
        printf("[rect] saveFramesRadio %s | savePhotonRadio %s | "
               "saveCorrected %s\n",
               rect(saveModeFrames_).c_str(), rect(saveModePhoton_).c_str(),
               rect(saveCorrected_).c_str());
    }
    // The pads and the alignment moves push the panels around *after* the first
    // paint; repaint the column area so the space they leave behind does not
    // keep the old pixels (that is what left a piece of the "Virtual photon
    // counting" title floating next to the Save box).
    if (centralWidget())
        centralWidget()->update();
}

// Tells the overlay which boxes are in the chain right now: a stage that is
// switched off is skipped, so the arrow steps over it.
// What the ROI panel shows: the ROI the gate is using and how many are saved.
void MainWindow::updateRoiLabel() {
    if (!roiListVal_)
        return;
    QString act = "no ROI selected";
    if (pipeline_.gate().roiSet()) {
        int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        pipeline_.gate().roiSnapshot(x0, y0, x1, y1);
        act = QString("active %1x%2 at %3,%4")
                  .arg(x1 - x0)
                  .arg(y1 - y0)
                  .arg(x0)
                  .arg(y0);
    }
    roiListVal_->setText(QString("%1 | gate: %2 | %3 saved")
                             .arg(act)
                             .arg(pipeline_.gate().roiActive() ? "ROI only"
                                                              : "full frame")
                             .arg(roiListLoad().size()));
}

// One place that says what the keV conversion is using.
void MainWindow::updateGainMapLabel() {
    if (!gainMapVal_)
        return;
    if (!gain_flag.load())
        gainMapVal_->setText("gain off: PH spectra in ADU");
    else if (gainMapLoaded())
        gainMapVal_->setText(QFileInfo(QString::fromStdString(g_cfg.gainMapFile))
                                 .fileName());
    else if (kevChk_ && kevChk_->isChecked())
        gainMapVal_->setText(
            "no gain map selected - using nominal values (40 ADU/keV)");
    else
        gainMapVal_->setText("no gain map loaded : using defaults");
}

void MainWindow::updateFlow() {
    const bool pede = PedestalStore::instance().loaded() && !gNoPede.load();
    const bool gain = gain_flag.load();
    const bool scAll = !scCombo_ || scCombo_->currentIndex() <= 0;
    const bool gateOn = !gateChk_ || gateChk_->isChecked();
    // The arrows are static: the chain is always drawn the same way, and a
    // stage that is not in use is greyed out instead, so the bypass is obvious
    // without the arrows moving.
    // the stage's own switch stays black: Pede/Raw, Gate enabled and the gain
    // button are how a bypassed stage is switched back on
    greyStage(pedeGroup_, !pede, {btnNoPede_});
    // the Gain panel follows the gain toggle alone; the keV checkbox keeps its
    // tick for next time but is greyed while the gain is off, and the plots
    // fall back to ADU
    greyStage(gainGroup_, !gain, {btnGainToggle});
    if (kevChk_)
        kevChk_->setEnabled(gain);
    if (gainStateChk_)
        gainStateChk_->setChecked(gain); // read-only indicator, no signals
    // a gate that is switched off greys out exactly like a bypassed stage
    // Greyed when it cannot be *selected* (the tracker needs the signal gate),
    // NOT when it is merely inactive: the label under the button says whether
    // tracking is on, so the box must not go grey just because it is off.
    greyStage(pedeTrackGroup_, !gateOn);
    greyStage(scGroup_, scAll, {scCombo_}); // storage-cell filter == ALL
    greyStage(sigGroup_, !gateOn, {gateChk_}); // signal gate off == passthrough
    // photon counting needs a pedestal (it thresholds the pede-subtracted
    // ADC), so the whole virtual-photon-counting box is a bypassed stage when
    // the pedestal is out of the chain - the display map goes empty with it
    greyStage(pmapGroup_, !pede);
    // pedestal tracking only makes sense with the signal gate on: enable the
    // button here, but do not flip it (the gate's own handler does that when it
    // is switched off, so --pede-tracking survives start-up)
    if (btnPedeTrack_)
        btnPedeTrack_->setEnabled(gateOn);
    if (pedeTrackVal_) {
        const PedeTrackStats ts = pedeTrackerStats();
        if (!ts.on)
            pedeTrackVal_->setText("off");
        else
            pedeTrackVal_->setText(
                QString("on: %1 samples, delta %2 ADU, %3 updates")
                    .arg(static_cast<qulonglong>(ts.samples))
                    .arg(ts.maxDiff, 0, 'f', 1)
                    .arg(static_cast<qulonglong>(ts.updates)));
    }
    // the gate threshold is applied to the raw 14-bit value when no pedestal
    // is in play, so say so instead of claiming "pede-subtracted"
    if (thrLabel_) {
        const bool raw = gNoPede.load() || !PedestalStore::instance().loaded();
        thrLabel_->setText(raw ? "Threshold (raw ADC - no pede):"
                               : "Threshold (ADC, pede-subtracted):");
    }
    // The data path (orange) follows the save mode: frames are written from the
    // ROI, photon maps from the virtual photon counter.  Only the colours
    // change - the connectors themselves never move.
    {
        // only the branch that actually feeds the file is drawn, and it is
        // orange - the frames come from the ROI, the maps from the counting
        bool pmap = false, savePede = false;
        bool dPede, gOn;
        flowStageStates(dPede, savePede, gOn, pmap);
        // the file is always entered through the ROI: the maps are cropped to
        // it on the way out, so this arrow never moves and is always orange
        if (arrRoiDown_) {
            setFlowArrowClass(arrRoiDown_, FlowData);
            arrRoiDown_->setVisible(true);
        }
        if (strip_)
            strip_->setStages(dPede, savePede, gOn, pmap);
    }
    updateGainMapLabel();
    updateRoiLabel();
    if (noPedeVal_) {
        if (gNoPede.load())
            noPedeVal_->setText("no pede: raw ADC");
        else if (PedestalStore::instance().loaded())
            noPedeVal_->setText("pede subtracted");
        else
            noPedeVal_->setText("no pedestal loaded");
    }
}

void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    flowPass_ = 0;
    layoutFlow();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    if (!flowQueued_)
        flowPass_ = 0; // a fresh solve for the new geometry
    layoutFlow();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (!confirmQuit()) {
        event->ignore(); // the operator changed their mind
        return;
    }
    stop();
    closeSideWindows();
    event->accept();
    // ...and end the program even if a side window somehow refused to close
    QApplication::quit();
}

// ---------------------------------------------------------------------------
// periodic GUI work
// ---------------------------------------------------------------------------
void MainWindow::processImages() {
    updateReceiverRoiOverlay(); // cheap when the ROI did not change
    if (goout.load() == 0)
        return;
    std::lock_guard<std::mutex> lock(pipeline_.images().mtx);
    if (!pipeline_.images().ready)
        return;
    // ADC and gain are always the raw 1024x512 layout (ROI selection on them
    // stays in raw data coordinates, also in a JFSTRX build)
    // Hot pixels: in the raw view they are replaced by the average pedestal of
    // their 3x3 neighbourhood (so they look like background), in the
    // pedestal-corrected view by 0 - they carry no signal.
    const float *adcSrc = pipeline_.images().adc.data();
    if (pipeline_.gate().hotPixels() > 0) {
        const auto hmask = hotMaskSnapshot();
        if (hmask && hmask->size() == static_cast<size_t>(NCH)) {
            if (adcScratch_.size() != static_cast<size_t>(NCH))
                adcScratch_.assign(static_cast<size_t>(NCH), 0.0f);
            std::memcpy(adcScratch_.data(), adcSrc, NCH * sizeof(float));
            const std::shared_ptr<const PedeSet> peds =
                gNoPede.load() ? PedestalStore::instance().get() : nullptr;
            for (size_t i = 0; i < static_cast<size_t>(NCH); i++) {
                if (!(*hmask)[i])
                    continue;
                float rep = 0.0f;
                if (peds && peds->g0.size() >= static_cast<size_t>(NCH)) {
                    const int x = static_cast<int>(i % kRawWidth);
                    const int y = static_cast<int>(i / kRawWidth);
                    double sum = 0.0;
                    int cnt = 0;
                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            const int xx = x + dx, yy = y + dy;
                            if (xx < 0 || xx >= kRawWidth || yy < 0 ||
                                yy >= kRawHeight)
                                continue;
                            sum += peds->g0[static_cast<size_t>(yy) * kRawWidth +
                                            xx];
                            cnt++;
                        }
                    }
                    rep = cnt ? static_cast<float>(sum / cnt) : 0.0f;
                }
                adcScratch_[i] = rep;
            }
            adcSrc = adcScratch_.data();
        }
    }
    m_adcView->setImageCopy(adcSrc, kRawWidth, kRawHeight);
    if (m_photonView && pipeline_.images().photonReady &&
        pipeline_.images().photonSeq != pmapSeq_ &&
        !pipeline_.images().photon.empty()) {
        pmapSeq_ = pipeline_.images().photonSeq;
        const float *pmapSrc = pipeline_.images().photon.data();
        if (pipeline_.gate().hotPixels() > 0) {
            const auto hmask = hotMaskSnapshot();
            if (hmask && hmask->size() == static_cast<size_t>(NCH)) {
                if (photonScratch_.size() != static_cast<size_t>(NCH))
                    photonScratch_.assign(static_cast<size_t>(NCH), 0.0f);
                std::memcpy(photonScratch_.data(), pmapSrc,
                            NCH * sizeof(float));
                for (size_t i = 0; i < static_cast<size_t>(NCH); i++)
                    if ((*hmask)[i])
                        photonScratch_[i] = 0.0f;
                pmapSrc = photonScratch_.data();
            }
        }
        m_photonView->setImageCopy(pmapSrc, kRawWidth, 512);
        m_photonView->update();
    }
    m_gainView->setImageCopy(pipeline_.images().gain.data(), kRawWidth,
                             kRawHeight);
#ifdef JFSTRX
    if (pipeline_.images().panelsReady) {
        for (int p = 0; p < kNumAdcPanels; p++) {
            if (m_adcPanelView[p] && !pipeline_.images().panels[p].empty())
                m_adcPanelView[p]->setImageCopy(
                    pipeline_.images().panels[p].data(), kAdcPanelW[p],
                    kAdcPanelH[p]);
        }
    }
#endif
    pipeline_.images().ready = false;
}

// Mirrors the receiver ROI ("rx_roi" in the stream header) onto the ADC image
// as the RED box, next to the yellow gate ROI.  The ADC image is the raw
// 1024x512 layout, so this is always a plain rectangle in raw coordinates.
void MainWindow::updateReceiverRoiOverlay() {
    if (!m_adcView)
        return;
    const int xmin = rxRoiXmin.load();
    const int xmax = rxRoiXmax.load();
    const int ymin = rxRoiYmin.load();
    const int ymax = rxRoiYmax.load();
    if (!rxRoiValid.load() || xmax <= xmin || ymax <= ymin) {
        if (m_rxRoiShown) {
            m_adcView->setReceiverRoiRect(0, 0, 0, 0, false);
            m_rxRoiShown = false;
        }
        return;
    }
    if (m_rxRoiShown && xmin == m_rxRoiLast[0] && xmax == m_rxRoiLast[1] &&
        ymin == m_rxRoiLast[2] && ymax == m_rxRoiLast[3])
        return;
    m_adcView->setReceiverRoiRect(xmin, ymin, xmax + 1, ymax + 1, true);
    m_rxRoiShown = true;
    m_rxRoiLast[0] = xmin;
    m_rxRoiLast[1] = xmax;
    m_rxRoiLast[2] = ymin;
    m_rxRoiLast[3] = ymax;
}

void MainWindow::updateStatus() {
    const GateEngine::Counters s = pipeline_.gate().snapshot();
    const QLocale loc;

    frameLabel->setText(
        QString("Frames: display %1 | in stream %2 | gap %3")
            .arg(loc.toString(static_cast<qulonglong>(pipeline_.displayFrames())))
            .arg(loc.toString(static_cast<qulonglong>(s.framesIn)))
            .arg(loc.toString(static_cast<qulonglong>(s.framesIn -
                                                      s.framesPass))));

    // Stream state comes from ZMQ socket monitoring: an idle-but-alive
    // receiver is reported as connected, never as "stopped".
    QString streamTxt;
    for (size_t k = 0; k < pipeline_.portCount(); k++) {
        double since = 0.0;
        const Pipeline::StreamState st =
            pipeline_.streamState(static_cast<int>(k), &since);
        QString one;
        switch (st) {
        case Pipeline::StreamState::Disconnected:
            one = "disconnected";
            break;
        case Pipeline::StreamState::Connecting:
            one = "waiting...";
            break;
        case Pipeline::StreamState::Connected:
            one = "connected";
            break;
        case Pipeline::StreamState::Idle:
            one = QString("connected (no frames for %1 s)").arg(since, 0, 'f', 1);
            break;
        case Pipeline::StreamState::EndOfAcq:
            one = "connected (end of acquisition)";
            break;
        }
        const int w = pipeline_.portWidth(static_cast<int>(k));
        const int h = pipeline_.portHeight(static_cast<int>(k));
        const QString geo =
            (w > 0 && h > 0) ? QString(" [%1x%2]").arg(w).arg(h) : QString();
        if (k)
            streamTxt += "\n";
        streamTxt += QString("%1: %2%3  (%4 frames, last #%5)")
                         .arg(pipeline_.portList()[k])
                         .arg(one)
                         .arg(geo)
                         .arg(pipeline_.portFrames(static_cast<int>(k)))
                         .arg(pipeline_.portLastFrame(static_cast<int>(k)));
    }
    streamVal_->setText(streamTxt);

    // frame-index alignment: delta between the newest frames of each
    // interface, plus what had to be discarded to stay aligned
    if (pipeline_.portCount() > 1) {
        const qlonglong f0 = static_cast<qlonglong>(
            pipeline_.portLastFrame(0));
        const qlonglong f1 = static_cast<qlonglong>(
            pipeline_.portLastFrame(1));
        alignVal_->setText(
            QString("delta %1 | aligned %2 | resync drops %3 | behind drops "
                    "%4 | unpaired %5")
                .arg(f1 - f0)
                .arg(loc.toString(static_cast<qulonglong>(s.framesIn)))
                .arg(loc.toString(
                    static_cast<qulonglong>(pipeline_.resyncDrops())))
                .arg(loc.toString(
                    static_cast<qulonglong>(pipeline_.behindDrops())))
                .arg(loc.toString(
                    static_cast<qulonglong>(pipeline_.incompleteFrames()))));
        alignVal_->setVisible(true);
    } else {
        alignVal_->setVisible(false);
    }
    if (s.pedeLoaded)
        pedeVal_->setText(QString("loaded (%1)%2")
                              .arg(QString::fromStdString(g_cfg.pedeFile))
                              .arg(gPedeTracked.load() ? " [tracked]"
                                                       : ""));
    else
        pedeVal_->setText("not loaded");

    displayDropVal_->setText(
        loc.toString(static_cast<qulonglong>(pipeline_.displayDropped())));

    const QString err = QString::fromStdString(s.lastError);
    errVal_->setText(err.isEmpty() ? QString() : QString("Last error: %1").arg(err));

    framesInCumVal_->setText(loc.toString(static_cast<qulonglong>(s.framesIn)));
    // "Current" used to show the cumulative counters (framesInCur was never
    // reset), so both columns always agreed.  Derive the per-interval delta
    // from consecutive snapshots instead.
    static uint64_t pIn = 0, pPass = 0, pDrop = 0;
    const uint64_t dIn = s.framesIn - pIn;
    const uint64_t dPass = s.framesPass - pPass;
    const uint64_t dDrop = s.framesDrop - pDrop;
    pIn = s.framesIn;
    pPass = s.framesPass;
    pDrop = s.framesDrop;
    framesInCurVal_->setText(loc.toString(static_cast<qulonglong>(dIn)));
    framesPassCumVal_->setText(
        loc.toString(static_cast<qulonglong>(s.framesPass)));
    framesPassCurVal_->setText(loc.toString(static_cast<qulonglong>(dPass)));
    framesDropCumVal_->setText(
        loc.toString(static_cast<qulonglong>(s.framesDrop)));
    framesDropCurVal_->setText(loc.toString(static_cast<qulonglong>(dDrop)));
    updateFlow(); // the pedestal/gain state may have changed since last time

    scDropCumVal_->setText(
        loc.toString(static_cast<qulonglong>(s.framesScDrop)));
    scDropCurVal_->setText(
        loc.toString(static_cast<qulonglong>(s.framesScDropCur)));
    if (scLastVal_)
        scLastVal_->setText(s.lastSc < 0
                                ? QString("last frame: ? (no SC in header)")
                                : QString("last frame: SC %1").arg(s.lastSc));
    pixelsAboveCumVal_->setText(
        loc.toString(static_cast<qulonglong>(s.pixelsAbove)));
    pixelsAboveCurVal_->setText(
        loc.toString(static_cast<qulonglong>(s.pixelsAboveCur)));
    rateInVal_->setText(QString("%1 fps / %2 MB/s")
                            .arg(loc.toString(s.rateIn, 'f', 1))
                            .arg(loc.toString(s.rateInMBs, 'f', 1)));
    ratePassVal_->setText(QString("%1 fps / %2 MB/s")
                              .arg(loc.toString(s.ratePass, 'f', 1))
                              .arg(loc.toString(s.ratePassMBs, 'f', 1)));
    dropFracVal_->setText(loc.toString(s.dropFrac, 'f', 3));

    if (pipeline_.pedeAcquireActive())
        statusLabel->setText(
            QString("Status: acquiring pede... %1 frames")
                .arg(pipeline_.pedeAcquireProgress()));

    // ROI save progress + auto-stop detection (budget reached)
    if (pipeline_.saver().running()) {
        if (m_saveWasRunning != true) {
            m_saveWasRunning = true;
            if (btnSaveStart)
                btnSaveStart->setChecked(true);
        }
        updateSaveLabel();
    } else if (m_saveWasRunning) {
        m_saveWasRunning = false;
        if (btnSaveStart)
            btnSaveStart->setChecked(false);
        applySaveLock(false); // the run ended (user, budget or error)
        updateFlow();
        updateSaveLabel();
        const RoiSaver::Status st = pipeline_.saver().status();
        statusLabel->setText(QString("Status: ROI save stopped (%1), %2 saved")
                                 .arg(QString::fromStdString(st.stopReason))
                                 .arg(st.saved));
    }

    // gate ROI, for the headless line
    char roiTxt[64];
    if (pipeline_.gate().roiActive()) {
        int rx0, ry0, rx1, ry1;
        pipeline_.gate().roiSnapshot(rx0, ry0, rx1, ry1);
        snprintf(roiTxt, sizeof(roiTxt), "%dx%d@%d,%d", rx1 - rx0, ry1 - ry0,
                 rx0, ry0);
    } else {
        snprintf(roiTxt, sizeof(roiTxt), "off");
    }

    // ROI save state, for the headless line
    char saveTxt[96];
    {
        const RoiSaver::Status ss = pipeline_.saver().status();
        // the first number is whatever this mode writes: frames or maps
        const unsigned long long n =
            (unsigned long long)(ss.photonMode ? ss.pmaps : ss.saved);
        if (ss.running && ss.photonMode)
            snprintf(saveTxt, sizeof(saveTxt), "%llumap/%llu", n,
                     (unsigned long long)ss.dropped);
        else if (ss.running)
            snprintf(saveTxt, sizeof(saveTxt), "%llu/%llu", n,
                     (unsigned long long)ss.dropped);
        else if (!ss.h5Path.empty())
            snprintf(saveTxt, sizeof(saveTxt), "done:%llu%s/%llu", n,
                     ss.photonMode ? "map" : "",
                     (unsigned long long)ss.dropped);
        else
            snprintf(saveTxt, sizeof(saveTxt), "off");
    }

    // dual-interface alignment audit: out-of-step flushes and the current
    // load-shedding step (keep 1 in N pairs), once per second
    if (hotVal_)
        hotVal_->setText(
            QString("hot: %1 px")
                .arg(static_cast<qulonglong>(pipeline_.gate().hotPixels())));

    if (g_cfg.printCounters && pipeline_.portCount() > 1) {
        static int atick = 0;
        if (++atick >= 2) {
            atick = 0;
            // Sustained-rate audit: what fraction of the last 120 seconds
            // ran within 5 % of the best assembled rate seen, i.e. "2 kHz for
            // 99 % of the time" expressed as a number.
            static double best = 0.0;
            static int good = 0, total = 0;
            const double r = s.rateIn;
            if (r > best)
                best = r;
            const bool ok = best > 0.0 && r >= 0.95 * best;
            if (ok)
                good++;
            total++;
            if (total > 120) {
                total--; 
                if (!ok) good--;
            }
            printf("[align] keep=1/%d shed=%llu rate=%.0f/s best=%.0f/s "
                   "within95=%.0f%% (%d/%d s) hot=%llupx/%lluf\n",
                   g_keepDiv.load(),
                   static_cast<unsigned long long>(pipeline_.shedFrames()), r,
                   best, total ? 100.0 * good / total : 0.0, good, total,
                   static_cast<unsigned long long>(
                       pipeline_.gate().hotPixels()),
                   static_cast<unsigned long long>(
                       pipeline_.pmapFrames()));
            fflush(stdout);
        }
    }

    // optional headless monitoring: the counter line the old CLI printed
    if (g_cfg.printCounters) {
        static int tick = 0;
        if (++tick >= 2) { // updateStatus() runs every 500 ms
            tick = 0;
            printf("in=%llu pass=%llu drop=%llu inrate=%.1f passrate=%.1f "
                   "dropfrac=%.3f px=%llu display=%llu displaydrop=%llu "
                   "stream=%s resync=%llu behind=%llu incomplete=%llu "
                   "ports=%s workers=%d roi=%s save=%s gainmap=%s sc=%s "
                   "scdrop=%llu pthr=%g%s kev=%s ptrack=%s shown=%llu\n",
                   static_cast<unsigned long long>(s.framesIn),
                   static_cast<unsigned long long>(s.framesPass),
                   static_cast<unsigned long long>(s.framesDrop), s.rateIn,
                   s.ratePass, s.dropFrac,
                   static_cast<unsigned long long>(s.pixelsAbove),
                   static_cast<unsigned long long>(pipeline_.displayFrames()),
                   static_cast<unsigned long long>(pipeline_.displayDropped()),
                   pipeline_.streamStateName(),
                   static_cast<unsigned long long>(pipeline_.resyncDrops()),
                   static_cast<unsigned long long>(pipeline_.behindDrops()),
                   static_cast<unsigned long long>(
                       pipeline_.incompleteFrames()),
                   pipeline_.streamStateSummary().c_str(),
                   g_cfg.analysisWorkers, roiTxt, saveTxt,
                   gainMapLoaded() ? "loaded" : "off",
                   s.scFilter < 0 ? "all" : std::to_string(s.scFilter).c_str(),
                   static_cast<unsigned long long>(s.framesScDrop),
                   static_cast<double>(gain_flag.load() ? g_pheneThresholdKev
                                                        : g_pheneThreshold),
                   gain_flag.load() ? "keV" : "ADU",
                   (kevSpectra.load() && gain_flag.load())
                       ? (gainMapLoaded() ? "loaded" : "nominal")
                       : "off",
                   gPedeTracking.load()
                       ? (pedeTrackerStats().updates > 0 ? "on-upd" : "on")
                       : "off",
                   static_cast<unsigned long long>(pipeline_.imagesShown()));
            fflush(stdout);
        }
    }
}

void MainWindow::checkSignals() {
    if (g_sigCount == 0)
        return;
    if (g_sigCount > 1)
        std::_Exit(0);
    printf("[shutdown] signal received, stopping threads\n");
    fflush(stdout);
    statusLabel->setText("Status: stopping...");
    stop();
    QApplication::quit();
}

// ---------------------------------------------------------------------------
// actions
// ---------------------------------------------------------------------------
void MainWindow::toggleStartPause() {
    if (goout.load() == 0) {
        goout = 1;
        statusLabel->setText("Status: running");
    } else {
        goout = 0;
        statusLabel->setText("Status: paused");
    }
}

void MainWindow::faster() {
    int v = int(HDraw_every.load() * 0.8f);
    if (v < 1)
        v = 1;
    HDraw_every = v;
    cout << endl << "Drawing every " << HDraw_every.load() << " frames" << endl;
    updateDrawLabel();
}

void MainWindow::slower() {
    int cur = HDraw_every.load();
    int v = int(cur * 1.25f);
    if (v <= cur)
        v = cur + 1; // avoid 1->1
    HDraw_every = v;
    cout << endl << "Drawing every " << HDraw_every.load() << " frames" << endl;
    updateDrawLabel();
}

// "Apply gain": divide the pixels by their ADU/keV (map if loaded, otherwise
// the nominal 40) so the keV spectra are physically meaningful.  The plots show
// keV only while this is pushed and "keV PH" is ticked.
void MainWindow::toggleGain() {
    if (btnGainToggle)
        btnGainToggle->setChecked(!btnGainToggle->isChecked());
}

void MainWindow::setApplyGain(bool on) {
    // The photon-counting threshold is in ADU with the gain off and in keV with
    // it on: keep the value that is on screen in its own unit and show the
    // other one, so toggling the gain never destroys either number.
    if (photonThrSpin_) {
        const double shown = photonThrSpin_->value();
        if (on)
            g_pheneThreshold = static_cast<float>(shown); // it was ADU
        else
            g_pheneThresholdKev = static_cast<float>(shown); // it was keV
        const QSignalBlocker block(photonThrSpin_); // no handler round trip
        photonThrSpin_->setDecimals(on ? 1 : 0);
        photonThrSpin_->setSingleStep(on ? 0.5 : 10.0);
        photonThrSpin_->setValue(static_cast<double>(
            on ? g_pheneThresholdKev : g_pheneThreshold));
        if (photonThrLabel_)
            photonThrLabel_->setText(on ? "photon threshold (keV):"
                                        : "photon threshold (ADU):");
    }
    gain_flag = on;
    // Toggling the gain mid-save changes the unit of the photon counting, so
    // the maps written from here on use the other one while the JSON keeps the
    // value it recorded at save start.  Say it out loud instead of leaving a
    // file with two incompatible scales.
    if (gSaveMode.load() == 1 && pipeline_.saver().status().running) {
        printf("[gain] warning: the photon-counting unit is now %s while a "
               "photon-map save is running - maps written from here on use the "
               "new unit, the JSON keeps the one from save start\n",
               on ? "keV" : "ADU");
        fflush(stdout);
    }
    updateFlow(); // the arrows bypass the Gain box when it is off
    printf("[gain] apply gain %s (ADU/keV: %s)\n", on ? "ON" : "off",
           on ? (gainMapLoaded() ? "from the gain map" : "nominal 40") : "-");
    fflush(stdout);
    statusLabel->setText(on ? (gainMapLoaded()
                                   ? "Status: apply gain on (per-pixel map)"
                                   : "Status: apply gain on (nominal 40 ADU/keV)")
                            : "Status: apply gain off (raw ADC)");
}

void MainWindow::bwPalette1() {
    RootCmd c;
    c.type = RootCmdType::PaletteBW1;
    pushRootCmd(c);
    m_adcView->setPaletteLut(paletteLut(Palette::BW1)); // same colours as ROOT
    if (m_adcScale_)
        m_adcScale_->setPaletteChoice(1);
    statusLabel->setText("Status: BW palette");
}

void MainWindow::bwPalette2() {
    RootCmd c;
    c.type = RootCmdType::PaletteBW2;
    pushRootCmd(c);
    m_adcView->setPaletteLut(paletteLut(Palette::BW2));
    if (m_adcScale_)
        m_adcScale_->setPaletteChoice(2);
    statusLabel->setText("Status: BW palette 2");
}

void MainWindow::colPal() {
    RootCmd c;
    c.type = RootCmdType::PaletteColor;
    pushRootCmd(c);
    m_adcView->setPaletteLut(paletteLut(Palette::Color));
    statusLabel->setText("Status: colour palette");
}

// The ADC widget works in DISPLAY coordinates (row 0 = top of the widget),
// while every ROOT plot and the gate use DATA coordinates (row 0 = bottom,
// (0,0) = bottom-left).  This is the single place where the two meet: the
// gain view follows in display coordinates, the ROOT photon map gets the
// range converted to data rows AND framed on pixel edges (the histogram has
// one bin per pixel centred on the integer coordinate, so showing display rows
// [y1,y2) means the data range [ny-y2-0.5, ny-y1-0.5]).
// A window was zoomed.  With "zoom coordination" ticked (the default) every
// other window and the ROOT plots follow the rectangle; unticked, each window
// is on its own and nothing is propagated.
void MainWindow::zoomTo(FastImageView *src, double x1, double y1, double x2,
                        double y2) {
    if (x2 - x1 < kMinViewport || y2 - y1 < kMinViewport)
        return; // never push a degenerate range to the ROOT canvases
    if (!zoomCoord_)
        return;
    printf("[zoom] coordinated from %s\n",
           src == m_adcView    ? "ADC"
           : src == m_gainView ? "gain"
           : src == m_photonView ? "photon"
                                 : "?");
    fflush(stdout);
    for (FastImageView *v : {m_adcView, m_gainView, m_photonView})
        if (v && v != src)
            v->setViewport(x1, y1, x2, y2);
    const double ny =
        (m_adcView && m_adcView->imageHeight() > 0)
            ? static_cast<double>(m_adcView->imageHeight())
            : 512.0;
    RootCmd c;
    c.type = RootCmdType::Zoom;
    c.a = x1 - 0.5;
    c.b = ny - y2 - 0.5;
    c.c = x2 - 0.5;
    c.d = ny - y1 - 0.5;
    pushRootCmdCoalesced(c);
}

// The gate ROI is captured from the ADC viewport when the button is pressed
// and then stays fixed, so the operator can keep panning/zooming the image.
// The viewport is in DISPLAY coordinates (row 0 at the top) while the gate
// works in DATA rows (row 0 at the bottom), hence the vertical flip.
void MainWindow::toggleGateRoi(bool on) {
    if (on && m_applyingRoi)
        return; // ROI was set directly by applyRoi(); do not re-capture

    if (!on) { // stop restricting, but keep the current ROI
        pipeline_.gate().setRoiOnly(false);
        pipeline_.gate().resetCounters();
        printf("[roi] gate over the full frame (current ROI kept)\n");
        fflush(stdout);
        statusLabel->setText(
            "Status: gate over the full frame (ROI kept as the current one)");
        updateRoiLabel();
        return;
    }
    if (!pipeline_.gate().roiSet()) {
        QMessageBox::warning(
            this, "ROI only",
            "No ROI selected.\n\nUse \"Select ROI (ADC zoom)\" in the ROI "
            "panel (zoom the ADC image first), or pick one with "
            "\"Load ROI...\", then press ROI only again.");
        btnGateRoi->setChecked(false); // leave it unselected
        return;
    }
    pipeline_.gate().setRoiOnly(true);
    pipeline_.gate().resetCounters();
    int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    pipeline_.gate().roiSnapshot(x0, y0, x1, y1);
    printf("[roi] gate restricted to the current ROI x=%d..%d y=%d..%d\n", x0,
           x1, y0, y1);
    fflush(stdout);
    statusLabel->setText(QString("Status: gate restricted to the current ROI "
                                 "%1x%2 at %3,%4")
                             .arg(x1 - x0)
                             .arg(y1 - y0)
                             .arg(x0)
                             .arg(y0));
    updateRoiLabel();
}

// Toggles the in-image ROI editor.  It is seeded from the current ROI when
// there is one, otherwise from the ADC zoom (the old behaviour) so that the
// rectangle always starts somewhere sensible.  Selection only: restricting the
// gate stays the "ROI only" button's business.
void MainWindow::setRoiEditMode(bool on) {
    if (!m_adcView)
        return;
    if (!on) {
        m_adcView->beginRoiEdit(false);
        statusLabel->setText("Status: ROI editor closed");
        return;
    }
    const int imgW = m_adcView->imageWidth() > 0 ? m_adcView->imageWidth()
                                                 : kRawWidth;
    const int imgH = m_adcView->imageHeight() > 0 ? m_adcView->imageHeight()
                                                  : kRawHeight;
    int x0 = 0, y0 = 0, x1 = imgW, y1 = imgH;
    if (pipeline_.gate().roiSet()) {
        pipeline_.gate().roiSnapshot(x0, y0, x1, y1);
    } else if (m_adcView->hasImage()) {
        double vx = 0, vy = 0, vw = 0, vh = 0;
        m_adcView->viewport(vx, vy, vw, vh);
        x0 = static_cast<int>(std::floor(vx));
        x1 = static_cast<int>(std::ceil(vx + vw));
        y0 = imgH - static_cast<int>(std::ceil(vy + vh));
        y1 = imgH - static_cast<int>(std::floor(vy));
        x0 = std::max(0, std::min(imgW - 2, x0));
        y0 = std::max(0, std::min(imgH - 2, y0));
        x1 = std::max(x0 + 2, std::min(imgW, x1));
        y1 = std::max(y0 + 2, std::min(imgH, y1));
    }
    m_adcView->setOverlayRect(x0, y0, x1, y1, true);
    m_adcView->beginRoiEdit(true);
    m_adcView->setFocus();
    statusLabel->setText(
        "Status: ROI editor - drag the yellow rectangle to move it, its edges "
        "or corners to resize, empty space to draw a new one; press \"Select "
        "ROI\" again when done");
}

// Selects the ADC zoom as the current ROI.  Selection only: whether the gate
// is restricted to it is the "ROI only" button's business.
void MainWindow::captureRoi() {
    if (!m_adcView || !m_adcView->hasImage()) {
        QMessageBox::information(
            this, "Select ROI",
            "No image yet - wait for frames and zoom the ADC image first.");
        return;
    }
    double vx = 0, vy = 0, vw = 0, vh = 0;
    m_adcView->viewport(vx, vy, vw, vh);
    const int imgW = m_adcView->imageWidth();
    const int imgH = m_adcView->imageHeight();

    int x0 = static_cast<int>(std::floor(vx));
    int x1 = static_cast<int>(std::ceil(vx + vw));
    const int dy0 = static_cast<int>(std::floor(vy));
    const int dy1 = static_cast<int>(std::ceil(vy + vh));
    // display row d  <->  data row (imgH - 1 - d)
    int y0 = imgH - dy1;
    int y1 = imgH - dy0;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > imgW) x1 = imgW;
    if (y1 > imgH) y1 = imgH;

    pipeline_.gate().setRoi(x0, y0, x1, y1);
    if (!pipeline_.gate().roiSet()) { // too small to be useful
        QMessageBox::information(this, "Select ROI",
                                 "The selected region is too small.");
        return;
    }
    if (m_adcView)
        m_adcView->setOverlayRect(x0, y0, x1, y1, true);
    if (gateRoiVal_)
        gateRoiVal_->setText(QString("%1x%2 at %3,%4 (data rows)")
                                 .arg(x1 - x0)
                                 .arg(y1 - y0)
                                 .arg(x0)
                                 .arg(y0));
    printf("[roi] current ROI x=%d..%d y=%d..%d (%dx%d px, data rows, row 0 = "
           "bottom) selected from the ADC zoom\n",
           x0, x1, y0, y1, x1 - x0, y1 - y0);
    fflush(stdout);
    updateRoiLabel();
    statusLabel->setText(
        QString("Status: ROI %1x%2 at %3,%4 is the current ROI%5")
            .arg(x1 - x0)
            .arg(y1 - y0)
            .arg(x0)
            .arg(y0)
            .arg(pipeline_.gate().roiActive()
                     ? " (gate restricted to it)"
                     : " - press \"ROI only\" to restrict the gate"));
}

// Adds the active gate ROI to the list file in the program folder.
void MainWindow::saveRoi() {
    if (!pipeline_.gate().roiSet()) {
        QMessageBox::information(
            this, "Save ROI",
            "No ROI selected.\n\nZoom the ADC image and press "
            "\"Select ROI (ADC zoom)\" first (or load one from the list).");
        return;
    }
    int x0, y0, x1, y1;
    pipeline_.gate().roiSnapshot(x0, y0, x1, y1);
    const int w = x1 - x0, h = y1 - y0;
    if (!roiListUpsert(x0, y0, w, h)) {
        QMessageBox::warning(this, "Save ROI",
                             QString("Could not write %1")
                                 .arg(QString::fromStdString(g_cfg.roiFile)));
        return;
    }
    const size_t n = roiListLoad().size();
    updateRoiLabel();
    statusLabel->setText(QString("Status: ROI %1x%2 at %3,%4 saved (%5 in list)")
                             .arg(w).arg(h).arg(x0).arg(y0).arg(n));
}
// Applies a ROI to the gate without capturing the ADC viewport, and makes
// sure the "ROI only" button reflects that the gate is restricted.
void MainWindow::applyRoi(int x, int y, int w, int h, const QString &src,
                          bool restrict) {
    pipeline_.gate().setRoi(x, y, x + w, y + h);
    if (!pipeline_.gate().roiSet()) {
        QMessageBox::information(this, "Load ROI",
                                 "That entry is too small to use.");
        return;
    }
    // -1 = keep the current restriction, 0/1 = force it off/on
    if (restrict)
        pipeline_.gate().setRoiOnly(true);
    pipeline_.gate().resetCounters();
    if (m_adcView)
        m_adcView->setOverlayRect(x, y, x + w, y + h, true);
    printf("[roi] current ROI x=%d..%d y=%d..%d (%dx%d px, data rows, row 0 = "
           "bottom) applied from %s\n",
           x, x + w, y, y + h, w, h, src.toLocal8Bit().constData());
    fflush(stdout);
    if (gateRoiVal_)
        gateRoiVal_->setText(QString("%1x%2 at %3,%4 (data rows)")
                                 .arg(w).arg(h).arg(x).arg(y));
    updateRoiLabel();
    statusLabel->setText(
        QString("Status: ROI %1x%2 at %3,%4 is the current ROI (from %5)%6")
            .arg(w).arg(h).arg(x).arg(y).arg(src)
            .arg(pipeline_.gate().roiActive()
                     ? " - gate restricted to it"
                     : " - press \"ROI only\" to restrict the gate"));
}

// List of saved ROIs, most recently saved/loaded first.
void MainWindow::loadRoiDialog() {
    std::vector<RoiEntry> list = roiListLoad();
    const bool listEmpty = list.empty();

    QDialog dialog(this);
    dialog.setWindowTitle("Load gate ROI");
    dialog.resize(520, 320);
    auto *lay = new QVBoxLayout(&dialog);
    auto *table = new QTableWidget(static_cast<int>(list.size()), 5, &dialog);
    table->setHorizontalHeaderLabels(
        QStringList() << "X" << "Y" << "width" << "height" << "last used");
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    const QLocale loc;
    for (size_t i = 0; i < list.size(); i++) {
        const RoiEntry &e = list[i];
        const QString when = QDateTime::fromMSecsSinceEpoch(e.usedMs)
                                 .toString("yyyy-MM-dd HH:mm:ss");
        const QString cells[5] = {
            loc.toString(e.x), loc.toString(e.y), loc.toString(e.w),
            loc.toString(e.h), when};
        for (int c = 0; c < 5; c++)
            table->setItem(static_cast<int>(i), c,
                           new QTableWidgetItem(cells[c]));
    }
    table->resizeColumnsToContents();
    table->selectRow(0); // newest first, pre-selected
    lay->addWidget(table);
    // the same convention as the ADC image and every ROOT plot
    auto *hint = new QLabel(
        listEmpty
            ? QString("The list is empty.  Use \"Edit as new...\" to type a "
                      "ROI, or \"Save ROI\" in the ROI panel to add the current "
                      "one (entries are kept in %1).")
                  .arg(QString::fromStdString(g_cfg.roiFile))
            : QString("X/Y are data coordinates: x grows to the right, y grows "
                      "upwards and (0,0) is the bottom-left pixel."),
        &dialog);
    hint->setWordWrap(true);
    lay->addWidget(hint);

    auto *btns = new QDialogButtonBox(&dialog);
    auto *applyBtn = btns->addButton("Apply", QDialogButtonBox::AcceptRole);
    auto *delBtn = btns->addButton("Delete", QDialogButtonBox::DestructiveRole);
    auto *editBtn =
        btns->addButton("Edit as new...", QDialogButtonBox::ActionRole);
    editBtn->setToolTip(
        "Type the four numbers of a ROI (prefilled from the selected entry, or\n"
        "from the current ROI), press Apply: the rectangle becomes the current\n"
        "ROI and is added to the list as a new entry.");
    btns->addButton(QDialogButtonBox::Cancel);
    if (listEmpty) { // nothing to apply or delete, but editing still works
        applyBtn->setEnabled(false);
        delBtn->setEnabled(false);
    }
    lay->addWidget(btns);
    connect(btns, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(table, &QTableWidget::itemDoubleClicked, &dialog,
            [&dialog](QTableWidgetItem *) { dialog.accept(); });

    bool editedChosen = false; // the numbers dialog applied the ROI itself
    connect(editBtn, &QPushButton::clicked, &dialog, [&]() {
        const int imgW = (m_adcView && m_adcView->imageWidth() > 0)
                             ? m_adcView->imageWidth()
                             : kRawWidth;
        const int imgH = (m_adcView && m_adcView->imageHeight() > 0)
                             ? m_adcView->imageHeight()
                             : kRawHeight;
        int x = 0, y = 0, w = imgW, h = imgH;
        const int r = table->currentRow();
        if (r >= 0 && r < static_cast<int>(list.size())) {
            x = list[r].x;
            y = list[r].y;
            w = list[r].w;
            h = list[r].h;
        } else if (pipeline_.gate().roiSet()) {
            int a = 0, b = 0, c = 0, d = 0;
            pipeline_.gate().roiSnapshot(a, b, c, d);
            x = a;
            y = b;
            w = c - a;
            h = d - b;
        }
        QDialog ed(&dialog);
        ed.setWindowTitle("Edit ROI (applied as a new ROI)");
        auto *form = new QFormLayout(&ed);
        auto mk = [&](int v, int lo, int hi) {
            auto *sb = new QSpinBox(&ed);
            sb->setRange(lo, hi);
            sb->setValue(std::max(lo, std::min(hi, v)));
            return sb;
        };
        auto *sx = mk(x, 0, imgW - 1);
        auto *sy = mk(y, 0, imgH - 1);
        auto *sw = mk(w, 1, imgW);
        auto *sh = mk(h, 1, imgH);
        form->addRow("X (data, from the left):", sx);
        form->addRow("Y (data, from the bottom):", sy);
        form->addRow("Width:", sw);
        form->addRow("Height:", sh);
        auto *eb = new QDialogButtonBox(&ed);
        // A QDialogButtonBox only emits accepted() for AcceptRole buttons, so an
        // Apply (ApplyRole) button must be wired to accept() explicitly -
        // otherwise clicking it did nothing at all.
        auto *applyEd = eb->addButton("Apply", QDialogButtonBox::ApplyRole);
        eb->addButton(QDialogButtonBox::Cancel);
        form->addRow(eb);
        connect(applyEd, &QPushButton::clicked, &ed, &QDialog::accept);
        connect(eb, &QDialogButtonBox::rejected, &ed, &QDialog::reject);
        // keep the rectangle inside the image while the numbers are typed
        connect(sx, QOverload<int>::of(&QSpinBox::valueChanged), &ed,
                [=](int v) { sw->setMaximum(std::max(1, imgW - v)); });
        connect(sy, QOverload<int>::of(&QSpinBox::valueChanged), &ed,
                [=](int v) { sh->setMaximum(std::max(1, imgH - v)); });
        sw->setMaximum(std::max(1, imgW - sx->value()));
        sh->setMaximum(std::max(1, imgH - sy->value()));
        if (ed.exec() != QDialog::Accepted)
            return;
        const int nx = sx->value(), ny = sy->value();
        const int nw = std::min(sw->value(), imgW - nx);
        const int nh = std::min(sh->value(), imgH - ny);
        if (nw < 2 || nh < 2) {
            QMessageBox::information(this, "Edit ROI",
                                     "That rectangle is too small to use.");
            return;
        }
        applyRoi(nx, ny, nw, nh, "edited numbers", false);
        // it is a NEW entry: store it in the list (this also refreshes the
        // timestamp, so it comes back first next time)
        if (!roiListUpsert(nx, ny, nw, nh))
            QMessageBox::warning(
                this, "Edit ROI",
                QString("The ROI was applied but could not be written to %1")
                    .arg(QString::fromStdString(g_cfg.roiFile)));
        updateRoiLabel();
        editedChosen = true;
        dialog.accept();
    });
    bool deleted = false;
    connect(delBtn, &QPushButton::clicked, &dialog, [&]() {
        const int r = table->currentRow();
        if (r < 0 || r >= static_cast<int>(list.size()))
            return;
        const RoiEntry &e = list[r];
        if (!roiListRemove(e.x, e.y, e.w, e.h)) {
            QMessageBox::warning(this, "Load ROI",
                                 QString("Could not write %1")
                                     .arg(QString::fromStdString(
                                         g_cfg.roiFile)));
            return;
        }
        deleted = true;
        table->removeRow(r); // keep the dialog open for further edits
    });

    if (dialog.exec() != QDialog::Accepted) {
        if (deleted)
            statusLabel->setText("Status: ROI entry deleted");
        return;
    }
    if (editedChosen)
        return; // already applied by the "Edit as new..." dialog
    const int row = table->currentRow();
    if (row < 0 || row >= static_cast<int>(list.size()))
        return;
    const RoiEntry e = list[row];
    // refresh the timestamp so it comes back first next time
    roiListUpsert(e.x, e.y, e.w, e.h);
    applyRoi(e.x, e.y, e.w, e.h, "list");
}

void MainWindow::resetHis() {
    requestHistoReset();
    statusLabel->setText("Status: histograms reset");
}

void MainWindow::resetAxis() {
    RootCmd c;
    c.type = RootCmdType::ResetAxes;
    pushRootCmd(c);
    // The ADC view is the master of the zoom, so unzooming only the ROOT
    // clones would leave the images (and, through the next drag, the photon
    // map) still zoomed: reset the widget viewports as well.
    if (m_adcView)
        m_adcView->resetView(); // propagates to the gain view + photon map
    statusLabel->setText("Status: axes reset");
}

// Starts/stops writing the gate-passed ROI frames to HDF5.
void MainWindow::toggleRoiSave(bool on) {
    RoiSaver &sv = pipeline_.saver();
    if (!on) {
        if (sv.running()) {
            sv.requestStop("user");
            statusLabel->setText("Status: finishing ROI save (flushing)...");
            applySaveLock(false); // the run is over, settings are free again
            updateFlow();
            updateSaveLabel();
        }
        return;
    }
    if (sv.running()) {
        btnSaveStart->setChecked(false);
        return;
    }
    // No gate ROI is fine: roiSnapshot() then reports the full frame, so the
    // save writes the whole image instead of refusing to start.

    RoiSaver::StartInfo info;
    int x0, y0, x1, y1;
    pipeline_.gate().roiSnapshot(x0, y0, x1, y1);
    info.x = x0;
    info.y = y0;
    info.w = x1 - x0;
    info.h = y1 - y0;
    info.maxFrames = static_cast<uint64_t>(saveFrames_->value());
    info.photonMode = saveModePhoton_ && saveModePhoton_->isChecked();
    info.photonEvery = pmapSaveSpin_ ? pmapSaveSpin_->value() : 2000;
    info.photonThreshold = gain_flag.load() ? g_pheneThresholdKev
                                           : g_pheneThreshold;
    info.photonKev = gain_flag.load();
    gGateChangedDuringSave.store(false); // fresh run, fresh note
    pmapSaveReset(); // start the first saved map from scratch
    info.dir = saveDirEdit_->text().trimmed().toStdString();
    if (info.dir.empty())
        info.dir = ".";
    info.baseName = saveBaseEdit_->text().trimmed().toStdString();
    if (info.baseName.empty())
        info.baseName = "jfroi";

    // pedestal in the same ROI (gain 0): slice 0 in raw mode, the
    // "/pedestal data" dataset in corrected mode
    std::shared_ptr<const PedeSet> peds = PedestalStore::instance().get();
    info.corrected = saveCorrected_->isChecked() && !gNoPede.load();
    if (info.corrected && !peds) {
        QMessageBox::information(
            this, "ROI save",
            "Pedestal-corrected saving needs a pedestal.\n\nRead or acquire "
            "one first, or switch the option off to save raw data.");
        btnSaveStart->setChecked(false);
        return;
    }
    // the correction itself needs the full-frame pedestal set
    info.peds = peds;
    // Virtual photon counting thresholds the PEDESTAL-SUBTRACTED ADC, so a map
    // without a pedestal would be empty: refuse the save instead of writing
    // empty maps (same policy as "Pedestal corrected" above).
    if (info.photonMode && (!peds || gNoPede.load())) {
        const char *why = peds ? "\"No pede\" is on" : "no pedestal is loaded";
        printf("[save] refusing virtual photon counting: %s, and the photon "
               "map thresholds the pedestal-subtracted ADC\n", why);
        fflush(stdout);
        statusLabel->setText(
            QString("Status: virtual photon counting needs a pedestal (%1)")
                .arg(QString::fromLatin1(why)));
        // a scripted start (--start-save) must not wait on a modal dialog
        if (!g_cfg.startSave) {
            QMessageBox::warning(
                this, "Virtual photon counting",
                QString("Virtual photon counting thresholds the "
                        "pedestal-subtracted ADC, but %1.\n\nLoad or acquire "
                        "a pedestal and clear \"No pede\" first (or save "
                        "frames instead).")
                    .arg(QString::fromLatin1(why)));
        }
        btnSaveStart->setChecked(false);
        updateSaveLabel();
        return;
    }
    info.useGain = gain_flag.load();
    info.g1off = G1Poffset.load();
    info.g2off = G2Poffset.load();
    if (peds) {
        // the ROI is in raw data coordinates (the ADC image is unmapped), and
        // g0 is the raw 1024x512 pedestal even in a JFSTRX build
        info.pedestal.resize(static_cast<size_t>(info.w) * info.h, 0.0f);
        for (int y = 0; y < info.h; y++) {
            const int sy = info.y + y;
            for (int x = 0; x < info.w; x++) {
                const int sx = info.x + x;
                if (sy >= 0 && sy < kRawHeight && sx >= 0 && sx < kRawWidth)
                    info.pedestal[static_cast<size_t>(y) * info.w + x] =
                        peds->g0[static_cast<size_t>(sy) * kRawWidth + sx];
            }
        }
    }

    // gate criteria in force at the start
    const GateEngine::Counters g0 = pipeline_.gate().snapshot();
    // read from the engine: the operator can change these while running, and
    // g_cfg keeps the startup snapshot
    info.gate.enabled = pipeline_.gate().enabled();
    info.gate.threshold = pipeline_.gate().threshold();
    info.gate.minPixels = pipeline_.gate().minPixels();
    info.gate.contiguous = pipeline_.gate().contiguous();
    info.gate.roiActive = true;
    info.gate.noPede = gNoPede.load();
    info.gate.roiX = x0;
    info.gate.roiY = y0;
    info.gate.roiW = info.w;
    info.gate.roiH = info.h;
    info.gateAtEnd = g0;
    {
        char buf[256];
        snprintf(buf, sizeof(buf),
                 "{\"enabled\":%s,\"threshold\":%.3f,\"min_pixels\":%llu,"
                 "\"contiguous\":%s,\"storage_cell\":%d,\"no_pede\":%s,"
                 "\"hot_suppress\":%s,\"hot_threshold\":%.2f,"
                 "\"hot_pixels\":%llu}",
                 g_cfg.gateEnabled ? "true" : "false",
                 static_cast<double>(g_cfg.threshold),
                 static_cast<unsigned long long>(g_cfg.minPixels),
                 g_cfg.contiguous ? "true" : "false",
                 pipeline_.gate().scFilter(),
                 gNoPede.load() ? "true" : "false",
                 pipeline_.gate().hotSuppress() ? "true" : "false",
                 pipeline_.hotThreshold(),
                 static_cast<unsigned long long>(pipeline_.gate().hotPixels()));
        info.gateJson = buf;
    }
    // exact end-of-run statistics, refreshed by the writer when it finishes
    info.statsProvider = [this](GateEngine::Counters &g, RunStats &st,
                                std::vector<StreamInfo> &ports) {
        g = pipeline_.gate().snapshot();
        st.resync = pipeline_.resyncDrops();
        st.behind = pipeline_.behindDrops();
        st.unpaired = pipeline_.incompleteFrames();
        st.displayFrames = pipeline_.displayFrames();
        st.displayDropped = pipeline_.displayDropped();
        ports.clear();
        for (size_t i = 0; i < pipeline_.portCount(); i++) {
            StreamInfo si;
            si.port = pipeline_.portList()[i];
            si.state = pipeline_.streamStateName(
                pipeline_.streamState(static_cast<int>(i)));
            si.frames = pipeline_.portFrames(static_cast<int>(i));
            si.lastFrame = pipeline_.portLastFrame(static_cast<int>(i));
            ports.push_back(si);
        }
    };

    std::string err;
    if (!sv.start(info, err)) {
        QMessageBox::warning(this, "ROI save",
                             QString("Could not start: %1")
                                 .arg(QString::fromStdString(err)));
        btnSaveStart->setChecked(false);
        return;
    }
    // remember the destination, unless it is still the start-up default: that
    // one is not a "previous" destination and would only clutter the list
    if (info.dir == saveDefaultDir_.toStdString() &&
        info.baseName == saveDefaultBase_.toStdString()) {
        printf("[save] %s/%s is the default destination, not added to the "
               "previous-destinations list\n",
               info.dir.c_str(), info.baseName.c_str());
        fflush(stdout);
    } else {
        saveDestListUpsert(info.dir, info.baseName);
    }
    // keep the CLI defaults in sync so later runs start from the last choice
    g_cfg.h5Dir = info.dir;
    g_cfg.h5Base = info.baseName;
    const RoiSaver::Status st = sv.status();
    statusLabel->setText(QString("Status: saving to %1")
                             .arg(QString::fromStdString(st.h5Path)));
    applySaveLock(true); // nothing that changes the file from here on
    updateSaveLabel();
}

void MainWindow::browseSaveDir() {
    const QString dir = QFileDialog::getExistingDirectory(
        this, "Directory for the ROI save",
        saveDirEdit_->text().isEmpty() ? QDir::currentPath()
                                       : saveDirEdit_->text());
    if (!dir.isEmpty())
        saveDirEdit_->setText(dir);
}

// List of destinations used before, most recent first.
void MainWindow::saveDestDialog() {
    std::vector<SaveDest> list = saveDestListLoad();
    if (list.empty()) {
        QMessageBox::information(
            this, "Previous destinations",
            QString("No destination has been used yet.\n\nThey are recorded "
                    "when a save starts; the list lives in %1.")
                .arg(QString::fromStdString(g_cfg.saveListFile)));
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Previous save destinations");
    dialog.resize(560, 300);
    auto *lay = new QVBoxLayout(&dialog);
    auto *table = new QTableWidget(static_cast<int>(list.size()), 3, &dialog);
    table->setHorizontalHeaderLabels(
        QStringList() << "directory" << "base name" << "last used");
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    const QLocale loc;
    for (size_t i = 0; i < list.size(); i++) {
        const QString when = QDateTime::fromMSecsSinceEpoch(list[i].usedMs)
                                 .toString("yyyy-MM-dd HH:mm:ss");
        table->setItem(static_cast<int>(i), 0,
                       new QTableWidgetItem(
                           QString::fromStdString(list[i].dir)));
        table->setItem(static_cast<int>(i), 1,
                       new QTableWidgetItem(
                           QString::fromStdString(list[i].base)));
        table->setItem(static_cast<int>(i), 2, new QTableWidgetItem(when));
    }
    table->resizeColumnsToContents();
    table->selectRow(0);
    lay->addWidget(table);

    auto *btns = new QDialogButtonBox(&dialog);
    btns->addButton("Use", QDialogButtonBox::AcceptRole);
    auto *delBtn = btns->addButton("Delete", QDialogButtonBox::DestructiveRole);
    btns->addButton(QDialogButtonBox::Cancel);
    lay->addWidget(btns);
    connect(btns, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(table, &QTableWidget::itemDoubleClicked, &dialog,
            [&dialog](QTableWidgetItem *) { dialog.accept(); });
    bool deleted = false;
    connect(delBtn, &QPushButton::clicked, &dialog, [&]() {
        const int r = table->currentRow();
        if (r < 0 || r >= static_cast<int>(list.size()))
            return;
        saveDestListRemove(list[r].dir, list[r].base);
        deleted = true;
        table->removeRow(r);
    });

    if (dialog.exec() != QDialog::Accepted) {
        if (deleted)
            statusLabel->setText("Status: destination removed from the list");
        return;
    }
    const int row = table->currentRow();
    if (row < 0 || row >= static_cast<int>(list.size()))
        return;
    saveDirEdit_->setText(QString::fromStdString(list[row].dir));
    saveBaseEdit_->setText(QString::fromStdString(list[row].base));
    statusLabel->setText(
        QString("Status: destination set to %1 / %2")
            .arg(QString::fromStdString(list[row].dir))
            .arg(QString::fromStdString(list[row].base)));
}

void MainWindow::updateSaveLabel() {
    if (!saveVal_)
        return;
    const RoiSaver::Status st = pipeline_.saver().status();
    // in photon-map mode the frames counter never moves, so report the maps
    const qulonglong n = st.photonMode ? st.pmaps : st.saved;
    const QString what = st.photonMode ? (n == 1 ? "map" : "maps")
                                       : (n == 1 ? "frame" : "frames");
    // only mention the queue when something was actually dropped: "0 dropped
    // (queue full)" claimed a problem that is not there
    const QString dropNote = st.dropped
                                 ? QString(" (writer behind: queue full)")
                                 : QString(" (nothing lost)");
    if (st.running) {
        saveVal_->setText(QString("%1\n%2 %3 written, %4 dropped%5")
                              .arg(QString::fromStdString(st.h5Path))
                              .arg(n)
                              .arg(what)
                              .arg(st.dropped)
                              .arg(dropNote));
    } else if (st.h5Path.empty()) {
        saveVal_->setText("idle");
    } else {
        saveVal_->setText(
            QString("%1\n%2 %3 written, %4 dropped - stopped (%5)")
                .arg(QString::fromStdString(st.h5Path))
                .arg(n)
                .arg(what)
                .arg(st.dropped)
                .arg(QString::fromStdString(st.stopReason)));
    }
}

void MainWindow::acquirePede() {
    if (!pipeline_.running()) {
        QMessageBox::information(this, "Acquire pedestal",
                                 "The acquisition pipeline is not running.");
        return;
    }
    if (pipeline_.pedeAcquireActive())
        return;
    pipeline_.requestPedeAcquire(g_cfg.pedeFrames);
    statusLabel->setText("Status: acquiring pede...");
}

void MainWindow::savePede() {
    std::string werr;
    if (pedeSaveFile(g_cfg.pedeFile, werr)) {
        printf("[pede] saved %s\n", g_cfg.pedeFile.c_str());
        statusLabel->setText(QString("Status: pede saved to %1")
                                 .arg(QString::fromStdString(g_cfg.pedeFile)));
    } else {
        fprintf(stderr, "[pede] save failed: %s\n", werr.c_str());
        pipeline_.gate().setLastError("savePedestal: " + werr);
        QMessageBox::warning(this, "Save Pedestal",
                             QString::fromStdString(werr));
    }
}

void MainWindow::readPede() {
    std::string perr;
    if (pedeLoadFile(g_cfg.pedeFile, perr)) {
        printf("[pede] read %s\n", g_cfg.pedeFile.c_str());
        pipeline_.gate().setLastError("");
        statusLabel->setText("Status: pede read");
    } else {
        fprintf(stderr, "[pede] read failed: %s\n", perr.c_str());
        pipeline_.gate().setLastError("loadPedestal: " + perr);
        QMessageBox::warning(this, "Read Pedestal",
                             QString::fromStdString(perr));
    }
}

void MainWindow::loadAllPede() {
    // The original only printed "not implemented".  It is genuinely useful in
    // the merged program (it is the only way to load a full G0/G1/G2 set from
    // an arbitrary path), so it opens a file chooser now.
    const QString path = QFileDialog::getOpenFileName(
        this, "Load pedestal file", QDir::currentPath(),
        "Pedestal files (*.dat *.txt);;All files (*)");
    if (path.isEmpty())
        return;
    std::string perr;
    if (pedeLoadFile(path.toStdString(), perr)) {
        cout << "[pede] loaded " << path.toStdString() << endl;
        pipeline_.gate().setLastError("");
        statusLabel->setText(QString("Status: pede loaded from %1").arg(path));
    } else {
        fprintf(stderr, "[pede] load failed: %s\n", perr.c_str());
        pipeline_.gate().setLastError("loadPedestal: " + perr);
        QMessageBox::warning(this, "Load Pedestal",
                             QString::fromStdString(perr));
    }
}

// The live value range of the main plot: what its colour scale is using right
// now, whatever set it (the defaults, this dialog, or a drag on the ROOT
// canvas).  Read from the displayed histogram, so the dialog always opens on
// the values the operator is actually looking at.
// The ADC image (the "main plot") keeps its own colour scale, which the
// operator can also drag on the colour bar: read what the widget is using.
void MainWindow::liveAdcImageRange(int &lo, int &hi) const {
    if (m_adcView) {
        double a = 0, b = 0;
        m_adcView->colorRange(a, b);
        lo = static_cast<int>(std::floor(a));
        hi = static_cast<int>(std::ceil(b));
        if (lo != hi)
            return;
    }
    lo = adcmin.load();
    hi = adcmax.load();
}

// The ROOT 2D map (canvas A5) keeps its colour scale in min/max.
void MainWindow::liveMapRange(int &lo, int &hi) const {
    if (m_photonView) {
        double a = 0, b = 0;
        m_photonView->colorRange(a, b);
        lo = static_cast<int>(a);
        hi = static_cast<int>(b);
        if (lo != hi)
            return;
    }
    lo = pmmin.load();
    hi = pmmax.load();
}

// Same for the shared x window of the spectra.
void MainWindow::liveChipRange(int &lo, int &hi) const {
    if (dispChip[0] && dispChip[0]->GetXaxis()) {
        TAxis *ax = dispChip[0]->GetXaxis();
        lo = static_cast<int>(std::floor(ax->GetBinLowEdge(ax->GetFirst())));
        hi = static_cast<int>(std::ceil(ax->GetBinUpEdge(ax->GetLast())));
        if (lo != hi)
            return;
    }
    lo = chipxmin.load();
    hi = chipxmax.load();
}

// Calibration popup: pick (or change) the gain-map file used for the keV
// spectra.  Everything else keeps working in ADU.
void MainWindow::loadGainMap() {
    const QString start =
        g_cfg.gainMapFile.empty() ? QDir::homePath()
                                  : QString::fromStdString(g_cfg.gainMapFile);
    const QString path = QFileDialog::getOpenFileName(
        this, "Select gain map (g0,g1,g2,hg0,hg1,hg2 - 1024x512 doubles)", start,
        "Gain maps (*.bin *.dat *.raw);;All files (*)");
    if (path.isEmpty())
        return;
    std::string err;
    if (!gainMapLoadFile(path.toStdString(), err)) {
        fprintf(stderr, "[gain] %s\n", err.c_str());
        QMessageBox::warning(this, "Gain map", QString::fromStdString(err));
        return;
    }
    g_cfg.gainMapFile = path.toStdString();
    const QString base = QFileInfo(path).fileName();
    if (kevChk_)
        kevChk_->setChecked(true); // show what was just calibrated
    updateFlow();
    statusLabel->setText(
        QString("Status: gain map %1 loaded - PH spectra also in keV")
            .arg(base));
}

void MainWindow::showRangeDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("Set Ranges");
    auto *form = new QFormLayout(&dialog);

    int imgLo = 0, imgHi = 0, mapLo = 0, mapHi = 0, chipLo = 0, chipHi = 0;
    liveAdcImageRange(imgLo, imgHi);
    liveMapRange(mapLo, mapHi);
    liveChipRange(chipLo, chipHi);

    auto *adcMinEdit = new QSpinBox(&dialog);
    adcMinEdit->setRange(-100000, 100000);
    adcMinEdit->setValue(imgLo);
    auto *adcMaxEdit = new QSpinBox(&dialog);
    adcMaxEdit->setRange(-100000, 100000);
    adcMaxEdit->setValue(imgHi);
    auto *pmMinEdit = new QSpinBox(&dialog);
    pmMinEdit->setRange(-100000, 100000);
    pmMinEdit->setValue(mapLo);
    auto *pmMaxEdit = new QSpinBox(&dialog);
    pmMaxEdit->setRange(-100000, 100000);
    pmMaxEdit->setValue(mapHi);
    auto *chMinEdit = new QSpinBox(&dialog);
    chMinEdit->setRange(-100000, 100000);
    chMinEdit->setValue(chipLo);
    auto *chMaxEdit = new QSpinBox(&dialog);
    chMaxEdit->setRange(-100000, 100000);
    chMaxEdit->setValue(chipHi);

    form->addRow("ADC min (ADC image):", adcMinEdit);
    form->addRow("ADC max (ADC image):", adcMaxEdit);
    form->addRow("P.Map min (ROOT map):", pmMinEdit);
    form->addRow("P.Map max (ROOT map):", pmMaxEdit);
    form->addRow("PH min (spectra):", chMinEdit);
    form->addRow("PH max (spectra):", chMaxEdit);
    auto *hint = new QLabel(
        "Each field opens on the scale that display is using now; nothing "
        "changes unless you edit it.",
        &dialog);
    hint->setWordWrap(true);
    form->addRow(hint);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        setRanges(adcMinEdit->value(), adcMaxEdit->value(), pmMinEdit->value(),
                  pmMaxEdit->value(), chMinEdit->value(), chMaxEdit->value());
    }
}

void MainWindow::setRanges(int amin, int amax, int pmin, int pmax, int cmin,
                           int cmax) {
    int imgLo = 0, imgHi = 0, mapLo = 0, mapHi = 0, chipLo = 0, chipHi = 0;
    liveAdcImageRange(imgLo, imgHi);
    liveMapRange(mapLo, mapHi);
    liveChipRange(chipLo, chipHi);

    // the ADC pair is the colour scale of the ADC (main) image - a Qt widget,
    // so it is applied straight away - and the P.Map pair the ROOT 2D map,
    // which must be changed on the ROOT thread
    adcmin = amin;
    adcmax = amax;
    pmmin = pmin;
    pmmax = pmax;
    chipxmin = cmin;
    chipxmax = cmax;
    if ((amin != imgLo || amax != imgHi) && m_adcView) {
        m_adcView->setColorRange(amin, amax);
        if (m_adcScale_)
            m_adcScale_->syncFromView();
        cout << "Ranges set: ADC image [" << amin << "," << amax << "]"
             << endl;
    }
    if (pmin != mapLo || pmax != mapHi) {
        fixranges = true;
        RootCmd c;
        c.type = RootCmdType::SetMapRange;
        c.a = pmin;
        c.b = pmax;
        pushRootCmd(c);
        cout << "Ranges set: ROOT map z [" << pmin << "," << pmax << "]"
             << endl;
    }
    if (cmin != chipLo || cmax != chipHi) {
        RootCmd c;
        c.type = RootCmdType::SetChipRange;
        c.a = cmin;
        c.b = cmax;
        pushRootCmd(c);
        cout << "Ranges set: PH x [" << cmin << "," << cmax << "]" << endl;
    }
    forceRootRedraw();
}

void MainWindow::showRateDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("Update Rates");
    auto *form = new QFormLayout(&dialog);

    auto *mainRateSpin = new QSpinBox(&dialog);
    mainRateSpin->setRange(1, 10000);
    mainRateSpin->setValue(HDraw_every.load());
    form->addRow("Main image (frames):", mainRateSpin);

    auto *rootMsSpin = new QSpinBox(&dialog);
    rootMsSpin->setRange(50, 60000);
    rootMsSpin->setSingleStep(100);
    rootMsSpin->setValue(rootUpdateMs.load());
    form->addRow("ROOT update (ms):", rootMsSpin);

    auto *chipSpin = new QSpinBox(&dialog);
    chipSpin->setRange(1, 100);
    chipSpin->setValue(chipUpdateEvery.load());
    chipSpin->setToolTip(
        "Redraw the chip-spectrum canvas (A4) every N-th ROOT redraw.  It is\n"
        "the most expensive canvas, so a larger N keeps the other canvases\n"
        "responsive.");
    form->addRow("Chip spectra every N redraws:", chipSpin);

    auto *effLabel = new QLabel(
        QString("ROOT redraws every %1 ms when frames arrived")
            .arg(rootUpdateMs.load()),
        &dialog);
    effLabel->setWordWrap(true);
    form->addRow(effLabel);
    connect(rootMsSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            [effLabel](int ms) {
                effLabel->setText(
                    QString("ROOT redraws every %1 ms when frames arrived")
                        .arg(ms));
            });

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        HDraw_every = mainRateSpin->value();
        rootUpdateMs = rootMsSpin->value();
        chipUpdateEvery = chipSpin->value();
        updateDrawLabel();
        cout << "Rates: main=" << HDraw_every.load()
             << " rootMs=" << rootUpdateMs.load() << endl;
    }
}

void MainWindow::updateDrawLabel() {
    if (drawLabel)
        drawLabel->setText(QString("Main: every %1 frames | ROOT: every %2 ms")
                               .arg(HDraw_every.load())
                               .arg(rootUpdateMs.load()));
}

// The ADC / gain / photon (and, under JFSTRX, the strixel) windows are separate
// top-level widgets, and the ROOT canvases live in the ROOT thread: closing the
// control window used to leave them on screen, and with them the process alive
// (Qt only quits on the *last* window).  Close them all, then leave the loop.
// The display chain and the saved data do not have to agree about the
// pedestal: the display follows the Pede/Raw toggle, while the file is
// corrected only when "Pedestal corrected" is ticked - and the photon maps
// always need the pedestal (their save is refused without it).
void MainWindow::flowStageStates(bool &displayPede, bool &savePede, bool &gateOn,
                                 bool &pmap) const {
    displayPede = PedestalStore::instance().loaded() && !gNoPede.load();
    pmap = (gSaveMode.load() == 1);
    savePede = pmap || ((saveCorrected_ && saveCorrected_->isChecked()) &&
                        !gNoPede.load());
    gateOn = !gateChk_ || gateChk_->isChecked();
}

void MainWindow::closeSideWindows() {
    QWidget *wins[] = {m_adcWin, m_gainWin, m_photonWin,
#ifdef JFSTRX
                       m_adcPanelWin,
#endif
                       nullptr};
    for (QWidget *w : wins) {
        if (!w)
            continue;
        w->hide();
        w->close();
    }
}

// While a save is running everything that would change what lands in the file is
// locked: its JSON describes the settings at save start, so a quiet change
// halfway would leave a file nobody can interpret.  Display-only controls (and
// the pedestal tracker, which is meant to follow drift during a run) stay live;
// the gate threshold and min-pixels stay live but ask first.
void MainWindow::applySaveLock(bool saving) {
    auto lockGroup = [&](QGroupBox *g, std::initializer_list<QWidget *> keep) {
        if (!g)
            return;
        for (QObject *c : g->children()) {
            auto *w = qobject_cast<QWidget *>(c);
            if (!w)
                continue;
            bool kept = false;
            for (QWidget *k : keep)
                if (k == w)
                    kept = true;
            w->setEnabled(!saving || kept);
        }
    };
    lockGroup(saveGroup_, {btnSaveStart, saveVal_}); // Start/Stop must stay live
    lockGroup(pedeGroup_, {});      // manual pede work changes the saved data
    lockGroup(gainGroup_, {kevChk_, gainStateChk_}); // keV PH is display only
    lockGroup(scGroup_, {});        // the cell filter changes which frames pass
    lockGroup(sigGroup_, {thrSpin_, minPixSpin_});   // the two that ask first
    lockGroup(pmapGroup_, {pmapCumulative_, pmapEverySpin_}); // display-only pair
    if (btnResetCounters)
        btnResetCounters->setEnabled(!saving);
    for (QWidget *w : {btnSelectRoi_, btnUnsetRoi_, btnLoadRoi})
        if (w)
            w->setEnabled(!saving); // they move the ROI, i.e. the saved crop
    // the photon threshold scales the counts that are written in photon-map
    // mode, so it is locked there and stays live while frames are saved
    if (photonThrSpin_)
        photonThrSpin_->setEnabled(!(saving && gSaveMode.load() == 1));
    if (saveCorrected_) // only meaningful for the frames mode
        saveCorrected_->setEnabled(!(gSaveMode.load() == 1) && !saving);
}

bool MainWindow::confirmGateChangeDuringRun(const QString &what) {
    if (!pipeline_.saver().status().running)
        return true;
    // With the gate off (passthrough) the threshold and min-pixels decide
    // nothing about the saved frames, so there is nothing to confirm.
    if (!pipeline_.gate().enabled())
        return true;
    const QMessageBox::StandardButton a = QMessageBox::question(
        this, "Gate setting",
        QString("Changing settings during a run, are you sure?\n\n%1\n\nThe "
                "file being written keeps the gate criterion it started with; a "
                "note is added to its JSON sidecar.")
            .arg(what),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (a != QMessageBox::Yes) {
        printf("[save] gate change refused during the run (%s)\n",
               what.toLocal8Bit().constData());
        fflush(stdout);
        return false;
    }
    gGateChangedDuringSave.store(true);
    printf("[save] gate setting changed during the run: %s (noted in the JSON)\n",
           what.toLocal8Bit().constData());
    fflush(stdout);
    return true;
}

// "Unset ROI": drop the current rectangle, so the gate, the ROI spectra and the
// saved data are back to the full frame.  The ROI list on disk is untouched.
void MainWindow::unsetRoi() {
    if (!pipeline_.gate().roiSet()) {
        statusLabel->setText("Status: no ROI set (already the full frame)");
        return;
    }
    // release the restriction first, so its own message stays truthful
    if (btnGateRoi && btnGateRoi->isChecked())
        btnGateRoi->setChecked(false);
    pipeline_.gate().clearRoi();
    pipeline_.gate().resetCounters();
    if (btnSelectRoi_ && btnSelectRoi_->isChecked())
        btnSelectRoi_->setChecked(false); // leaves the editor
    if (m_adcView)
        m_adcView->setOverlayRect(0, 0, 0, 0, false);
    if (gateRoiVal_)
        gateRoiVal_->setText("full frame");
    printf("[roi] ROI unset: full frame\n");
    fflush(stdout);
    updateRoiLabel();
    statusLabel->setText("Status: ROI unset (full frame)");
}

void MainWindow::quitApp() {
    if (!confirmQuit())
        return;
    stop();
    closeSideWindows();
    QApplication::quit();
}

// ---------------------------------------------------------------------------
// command line
// ---------------------------------------------------------------------------
void printUsage(const char *prog) {
    printf("Usage: %s <rx_host> [rx_port] [gate_threshold] [min_pixels]\n"
           "          [pede_file] [--pub PORT] [--contig] [--no-gate]\n"
           "          a port may carry its slab placement:\n"
           "          30026+30027      order of the list = bottom, top\n"
           "          30027:top        one interface, upper half\n"
           "          30026:bottom     one interface, lower half\n"
           "          [--pede-frames N]\n"
           "\n"
           "  rx_host      slsReceiver host\n"
           "  rx_port      slsReceiver zmq port, or a '+' list for the\n"
           "               multi-interface (N UDP interfaces) mode, ordered\n"
           "               bottom -> top:  30001+30002  = (bot,top)\n"
           "               (default %d)\n"
           "  gate_threshold  event-finder gate threshold, pede-subtracted ADC\n"
           "               (default %d)\n"
           "  min_pixels   gate: pass when >= N pixels are above threshold\n"
           "               (default 5)\n"
           "  pede_file    pedestal file                 (default %s)\n"
           "\n"
           "  The photon-map threshold (ADU, default %d) is a control in the\n"
           "  Display panel, not an argument; --raw-map shows raw ADC there\n"
           "  instead of photon counts.\n"
           "\n"
           "  --pub PORT       additionally republish gated frames on PORT\n"
           "  --contig         enable the contiguity proxy (r = N/2+1)\n"
           "  --hot-suppress   learn and suppress hot pixels in the gate\n"
           "  --hot-frac P     a pixel is hot above P % of the frames\n"
           "                   (default 10)\n"
           "  --gate           enable the gate (it is OFF by default)\n"
           "  --no-gate        disable the gate (same as the default)\n"
           "  --raw-map        show raw ADC in the main map instead of\n"
           "                   photon counts (threshold unused then)\n"
           "  --save-budget N  stop after N saved frames, or N saved photon\n"
           "                   maps in virtual photon counting mode\n"
           "  --apply-gain     start with the gain correction on (the photon\n"
           "                   threshold then means keV, default 4 keV)\n"
           "  --start-save     start saving at startup (same as pressing\n"
           "                   the Save button); combine with --save-photon\n"
           "  --save-photon    write the virtual photon counting maps instead\n"
           "                   of the frames (see the Save panel)\n"
           "  --pmap-every N   display cadence of the photon map (default 2000)\n"
           "  --pmap-save-every N\n"
           "                   saved map cadence, minimum 200 (default 2000)\n"
           "  --pede-tracking  follow the pedestal drift from the frames the\n"
           "                   signal gate drops (see the Pedestal panel)\n"
           "  --no-pede        ignore the pedestal everywhere (same as the\n"
           "                   No pede button); this is the STARTUP DEFAULT,\n"
           "                   so the gate threshold applies to raw 14-bit ADC\n"
           "  --use-pede       start with pedestal subtraction enabled\n"
           "  --stats          print the counter line on stdout every second\n"
           "  --decimate N     process 1 frame in N (N = 2, 4 or 8); the frames\n"
           "                   are dropped by detector frame number, so both\n"
           "                   interfaces drop the same ones\n"
           "  --auto-decimate  let the pipeline decimate on a real analysis\n"
           "                   backlog (off by default: --decimate is manual)\n"
           "  --acquire-pede   average --pede-frames frames into a live\n"
           "                   pedestal as soon as acquisition starts\n"
           "  --pede-frames N  frames averaged by \"Acquire Pede\" (default %d)\n"
           "  --analysis-workers N\n"
           "                   threads for the per-pixel analysis loops\n"
           "                   (default: auto = cores/4, capped at 8; 1 =\n"
           "                   single threaded). Scales across cores with no\n"
           "                   extra memory and bit-identical results.\n"
           "  --h5-dir DIR     directory for ROI-save HDF5 + JSON (default .)\n"
           "  --h5-base NAME   file-name base, file is <base>_<stamp>_f0_0.h5\n"
           "                   (default jfroi)\n"
           "  --gainmap PATH   gain-map file (6 x 1024x512 doubles: g0 g1 g2\n"
           "                   hg0 hg1 hg2).  g0 is ADU/keV and is used to\n"
           "                   fill a second set of PH spectra in keV; the ADC\n"
           "                   image, the photon map and the HDF5 writer stay\n"
           "                   in ADU.  Can also be loaded from the GUI.\n"
           "  --roi-file PATH  gate-ROI list file (default ./roi_list.dat)\n"
           "  --sc N           pass only frames from storage cell N (0..15,\n"
           "                   bits 8-11 of detSpec3; 'all' = no SC gating).\n"
           "                   Checked before the signal gate, so other cells\n"
           "                   cost no CPU.  The GUI has the same selector.\n"
           "  --gate-roi X Y W H\n"
           "                   start with a gate ROI (data rows: x to the\n"
           "                   right, y upwards, (0,0) = bottom-left; same\n"
           "                   convention as the ROI list and the ROOT plots)\n"
           "  --rx-hwm N       receive high-water mark per interface (default\n"
           "                   %d); smaller = lower latency, more drops\n"
           "\n"
           "Run chain: slsReceiver -> %s  (no external event finder, no\n"
           "intermediate zmq hop: the gate and the display share one process)\n",
           prog, DEFAULT_RX_PORT,
           static_cast<int>(PHOTON_THRESHOLD_DEFAULT), PEDEFNAME,
           static_cast<int>(PHOTON_THRESHOLD_DEFAULT), DEFAULT_N_PEDE_FRAMES,
           1000, prog);
}

bool parseCommandLine(int argc, char **argv, Config &cfg, bool &help,
                      std::string &error) {
    help = false;
    std::vector<std::string> pos;
    for (int i = 1; i < argc; i++) {
        const std::string a = argv[i];
        if (a == "-h" || a == "--help") {
            help = true;
            return true;
        } else if (a == "--pub") {
            if (i + 1 >= argc) {
                error = "--pub needs a port";
                return false;
            }
            cfg.pubPort = std::atoi(argv[++i]);
        } else if (a == "--contig") {
            cfg.contiguous = true;
        } else if (a == "--hot-suppress") {
            cfg.hotSuppress = true;
        } else if (a == "--hot-frac") {
            cfg.hotFraction = std::atof(argv[++i]) / 100.0;
        } else if (a == "--pmap-every") {
            if (i + 1 >= argc) {
                error = "--pmap-every needs a count";
                return false;
            }
            const int v = std::atoi(argv[++i]);
            gPmapEvery = v > 0 ? v : 1;
        } else if (a == "--pmap-save-every") {
            if (i + 1 >= argc) {
                error = "--pmap-save-every needs a count (>= 200)";
                return false;
            }
            const int v = std::atoi(argv[++i]);
            gPmapSaveEvery = v < 200 ? 200 : v;
        } else if (a == "--save-photon") {
            gSaveMode = 1; // same as the Save panel's radio button
        } else if (a == "--start-save") {
            cfg.startSave = true; // start saving as soon as we are up
        } else if (a == "--apply-gain") {
            // the gain stage is the only one without a CLI mirror; needed for
            // scripted keV runs (and it makes the keV photon threshold testable)
            cfg.applyGain = true;
        } else if (a == "--save-budget") {
            if (i + 1 >= argc) {
                error = "--save-budget needs a count";
                return false;
            }
            cfg.saveBudget = static_cast<uint64_t>(std::atoll(argv[++i]));
        } else if (a == "--pede-tracking") {
            gPedeTracking = true; // same as the Pedestal panel button
        } else if (a == "--no-pede") {
            // same as the "Pede/Raw toggle" button: no pedestal subtraction
            // anywhere, so the gate threshold applies to the raw 14-bit value
            gNoPede = true;
        } else if (a == "--use-pede") {
            // re-enable pedestal subtraction (the startup default is --no-pede)
            gNoPede = false;
        } else if (a == "--raw-map") {
            cfg.rawMap = true; // his3000 shows raw ADC instead of counts
        } else if (a == "--no-gate") {
            cfg.gateEnabled = false;
        } else if (a == "--gate") {
            cfg.gateEnabled = true; // the gate is off by default
        } else if (a == "--decimate") {
            const int v = std::atoi(argv[++i]);
            cfg.decimate = (v == 2 || v == 4 || v == 8) ? v : 1;
            g_keepDiv.store(cfg.decimate); // frames are dropped from now on
        } else if (a == "--auto-decimate") {
            cfg.autoDecimate = true;
            g_autoDecimate.store(true);
        } else if (a == "--stats") {
            cfg.printCounters = true;
        } else if (a == "--acquire-pede") {
            cfg.acquirePede = true;
        } else if (a == "--h5-base") {
            if (i + 1 >= argc) {
                error = "--h5-base needs a name";
                return false;
            }
            cfg.h5Base = argv[++i];
        } else if (a == "--h5-dir") {
            if (i + 1 >= argc) {
                error = "--h5-dir needs a path";
                return false;
            }
            cfg.h5Dir = argv[++i];
        } else if (a == "--gainmap") {
            if (i + 1 >= argc) {
                error = "--gainmap needs a file";
                return false;
            }
            cfg.gainMapFile = argv[++i];
            cfg.gainMapExplicit = true;
        } else if (a == "--roi-file") {
            if (i + 1 >= argc) {
                error = "--roi-file needs a path";
                return false;
            }
            cfg.roiFile = argv[++i];
        } else if (a == "--sc") {
            if (i + 1 >= argc) {
                error = "--sc needs a value (0..15, or all)";
                return false;
            }
            const std::string v = argv[++i];
            if (v == "all" || v == "ALL") {
                cfg.scFilter = -1;
            } else {
                const int n = std::atoi(v.c_str());
                if (n < 0 || n > 15) {
                    error = "--sc must be 0..15 or all";
                    return false;
                }
                cfg.scFilter = n;
            }
        } else if (a == "--gate-roi") {
            // X Y W H in data rows (row 0 = bottom), same as the ROI list
            if (i + 4 >= argc) {
                error = "--gate-roi needs X Y W H";
                return false;
            }
            cfg.gateRoiX = std::atoi(argv[++i]);
            cfg.gateRoiY = std::atoi(argv[++i]);
            cfg.gateRoiW = std::atoi(argv[++i]);
            cfg.gateRoiH = std::atoi(argv[++i]);
            if (cfg.gateRoiW < 1 || cfg.gateRoiH < 1) {
                error = "--gate-roi needs a positive width and height";
                return false;
            }
        } else if (a == "--analysis-workers") {
            if (i + 1 >= argc) {
                error = "--analysis-workers needs a count";
                return false;
            }
            const int n = std::atoi(argv[++i]);
            if (n < 1 || n > 32) {
                error = "--analysis-workers must be 1..32";
                return false;
            }
            cfg.analysisWorkers = n;
        } else if (a == "--rx-hwm") {
            if (i + 1 >= argc) {
                error = "--rx-hwm needs a value";
                return false;
            }
            const int n = std::atoi(argv[++i]);
            if (n < 1) {
                error = "--rx-hwm must be >= 1";
                return false;
            }
            cfg.rxHwm = n;
        } else if (a == "--pede-frames") {
            if (i + 1 >= argc) {
                error = "--pede-frames needs a count";
                return false;
            }
            const int n = std::atoi(argv[++i]);
            if (n < 1) {
                error = "--pede-frames must be >= 1";
                return false;
            }
            cfg.pedeFrames = n;
        } else if (a == "--pede") {
            if (i + 1 >= argc) {
                error = "--pede needs a file";
                return false;
            }
            cfg.pedeFile = argv[++i];
            cfg.pedeExplicit = true;
        } else if (!a.empty() && a[0] == '-' && a.size() > 1 &&
                   !isdigit(static_cast<unsigned char>(a[1]))) {
            error = "unknown option: " + a;
            return false;
        } else {
            pos.push_back(a);
        }
    }

    if (pos.empty()) {
        error = "need at least <rx_host>";
        return false;
    }
    cfg.rxHost = pos[0];
    // the photon-map threshold is a Display control (ADU, default 150); the
    // raw-ADC map is still available with --raw-map
    cfg.dophotonmap = !cfg.rawMap;
    cfg.threshold = static_cast<float>(cfg.phene / 2); // default gate threshold
    if (pos.size() >= 2) {
        // "30001" (single interface) or "30001+30002" (multi-interface,
        // ordered BOTTOM -> TOP in the assembled image)
        cfg.rxPorts.clear();
        std::string spec = pos[1];
        size_t start = 0;
        while (start <= spec.size()) {
            const size_t plus = spec.find('+', start);
            const std::string tok =
                spec.substr(start, (plus == std::string::npos)
                                       ? std::string::npos
                                       : plus - start);
            if (tok.empty()) {
                error = "bad rx_port list '" + spec + "'";
                return false;
            }
            // "30027:top" / "30026:bottom" place a single interface's slab
            // inside the module without needing its partner.
            std::string num = tok;
            int slab = 0;
            const size_t colon = tok.find(':');
            if (colon != std::string::npos) {
                num = tok.substr(0, colon);
                const std::string where = tok.substr(colon + 1);
                if (where == "top")
                    slab = 2;
                else if (where == "bottom")
                    slab = 1;
                else {
                    error = "bad placement '" + where + "' in '" + spec +
                            "' (use :top or :bottom)";
                    return false;
                }
            }
            const int port = std::atoi(num.c_str());
            if (port <= 0 || port > 65535) {
                error = "bad rx_port '" + tok + "' in '" + spec + "'";
                return false;
            }
            cfg.rxPorts.push_back(port);
            cfg.rxSlab.push_back(slab);
            if (plus == std::string::npos)
                break;
            start = plus + 1;
        }
        if (cfg.rxPorts.empty() ||
            cfg.rxPorts.size() > static_cast<size_t>(JF_MAX_PORTS)) {
            error = "rx_port list must have 1.." +
                    std::to_string(JF_MAX_PORTS) + " entries";
            return false;
        }
    }
    if (pos.size() >= 3)
        cfg.threshold = static_cast<float>(std::atof(pos[2].c_str()));
    if (pos.size() >= 4)
        cfg.minPixels = std::strtoull(pos[3].c_str(), nullptr, 10);
    if (pos.size() >= 5) {
        cfg.pedeFile = pos[4];
        cfg.pedeExplicit = true;
    }
    if (cfg.rxHost.empty()) {
        error = "rx_host is empty";
        return false;
    }
    if (cfg.analysisWorkers == 0) { // auto
        unsigned hw = std::thread::hardware_concurrency();
        if (hw == 0)
            hw = 4;
        cfg.analysisWorkers =
            static_cast<int>(std::min(8u, std::max(1u, hw / 4)));
    }
    return true;
}

// ---------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    Config cfg;
    bool help = false;
    std::string err;
    if (!parseCommandLine(argc, argv, cfg, help, err)) {
        if (!err.empty())
            fprintf(stderr, "error: %s\n", err.c_str());
        printUsage(argv[0]);
        return help ? 0 : 2;
    }
    if (help) {
        printUsage(argv[0]);
        return 0;
    }

    QApplication app(argc, argv);
    g_cfg = cfg;
    g_pheneThreshold = PHOTON_THRESHOLD_DEFAULT; // photon counting, ADU
    g_pheneThresholdKev = PHOTON_THRESHOLD_KEV_DEFAULT; // same, keV (gain on)
    cfg.phene = static_cast<int>(2 * PHOTON_THRESHOLD_DEFAULT);
    dophotonmap.store(cfg.dophotonmap);

    printf("=== Online JF display + event finder (single process) ===\n");
    {
        std::string ports;
        for (size_t k = 0; k < cfg.rxPorts.size(); k++) {
            if (k)
                ports += "+";
            ports += std::to_string(cfg.rxPorts[k]);
        }
        printf("input : %s:%s (subscriber)%s\n", cfg.rxHost.c_str(),
               ports.c_str(),
               cfg.rxPorts.size() > 1
                   ? "  [multi-interface: slabs stacked bottom -> top]"
                   : "");
    }
    printf("photon map: threshold %.0f ADU, %.1f keV while the gain correction "
           "is on (Virtual photon counting panel)%s\n",
           static_cast<double>(g_pheneThreshold),
           static_cast<double>(PHOTON_THRESHOLD_KEV_DEFAULT),
           cfg.dophotonmap ? "" : " [raw ADC map]");
    printf("gate  : %s, threshold %.1f, min_pixels %llu%s\n",
           cfg.gateEnabled ? "enabled" : "PASSTHROUGH", cfg.threshold,
           static_cast<unsigned long long>(cfg.minPixels),
           cfg.contiguous ? ", contiguous" : "");
    printf("pede  : %s\n", cfg.pedeFile.c_str());
    printf("threads: analysis workers %d\n", cfg.analysisWorkers);
    if (cfg.pubPort > 0)
        printf("pub   : gated frames republished on port %d\n", cfg.pubPort);

    MainWindow window;
    window.show();
    window.start();
    return app.exec();
}
