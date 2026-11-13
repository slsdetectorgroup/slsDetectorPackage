Plotting in slsDetectorGui
==========================

The detector GUI used to draw every plot with the bundled **Qwt 6.1.5**
library.  That dependency has been removed together with its build rules and
sources.  All plots in `slsDetectorGui` are now painted by a small,
self-contained stack that needs only **Qt Widgets / Gui / Core** (plus the
Qt `Concurrent` module for the acquisition thread).  It needs **Qt 6
(>= 6.2)**.  There is no ROOT, Qwt, Qt Charts or OpenGL dependency.

The stack lives in `slsDetectorGui/slsDetectorPlotting` and is compiled once
into the static library `slsDetectorPlotting`, which is linked by the GUI
executable, by the QtTest suite and by the demo application.

Components
----------

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Component
     - Responsibility
   * - `sls::plot` (SlsQtPlotCommon)
     - Shared data: the 256-entry colour lookup tables of all palettes, the
       1/2/5 tick and label formatting helpers, `logClamp()` and the dark
       `PlotTheme` used by the 1D, 2D and histogram painting.
   * - `SlsQtFastImage`
     - Fast 2D image view with its own axes, grid, tick labels, pixel outlines
       and value labels, ROI overlays and wheel/pan zoom.  Also the canvas of
       `SlsQt2DPlot`.
   * - `SlsQtScaleHisto`
     - Thin 256-bin histogram strip with two draggable range markers.
   * - `SlsQtScalePanel`
     - The strip plus palette chooser and image/histogram lin-log switches.
       The colour min/max are edited in the Plot tab; the panel synchronises
       its range with the image view in both directions.
   * - `SlsQt2DPlot` / `SlsQt2DHist`
     - The GUI 2D detector plot and its plain data container: z range, lin/log
       z, bilinear interpolation, contour lines and ROI boxes.  The z-axis
       (colour scale) title is shown as a compact caption row directly above the
       scale panel, where the old qwt colour bar used to have it.
   * - `SlsQt1DPlot` / `SlsQtH1D`
     - The GUI 1D plot widget and histogram/curve data class: auto range,
       lin/log axes, zoom/pan, legend, ROI boxes and gain plots.

Rendering rules
---------------

These rules are what make the image look like the old detector display while
staying fast enough for online frames:

* Data coordinates are **image pixel edge coordinates**: `x` to the right,
  `y` up, row 0 at the bottom, pixel `(ix, iy)` covering
  `[ix, ix+1) x [iy, iy+1)`.  The frame is flipped exactly once when the
  `QImage` is composed, never anywhere else.
* One cached `Format_RGB32` image is composed per update by writing whole
  scanlines through the 256-entry palette LUT; there is no `setPixel` in the
  hot loop and the image is only reallocated when the frame geometry changes.
  Repaints of the same frame reuse the cache.
* Log palettes clamp values `<= 0` to `1e-2` before `log10`, so empty or
  negative (pedestal corrected) pixels stay visible instead of becoming
  `-inf`.
* The histogram of the scale panel is binned over the window the strip
  actually displays - the colour range widened by half a range on either side,
  in log space when the image is logarithmic - and values outside that window
  are skipped instead of piling up in the edge bins.  It is recomputed at most
  every ~300 ms.
* The image, the overlays and the contours are clipped to the plot rectangle,
  so a zoomed frame never paints over the axes or the margins.
* Axis ticks are 1/2/5 x 10^n steps chosen for at most ~8 grid lines; labels
  drop trailing zeros and adapt their decimals to the step.  Pixel outlines
  appear when one data pixel is at least ~16 screen pixels wide, in-pixel
  values only when they fit (~50 screen pixels) and use the integer format for
  photon/gain maps.
* Wheel zoom keeps the data point under the cursor fixed (factor 1.18) and the
  viewport is clamped to the frame and to a minimum size.
* Overlays (gate ROI, receiver ROIs, contour lines) are specified in data
  coordinates and follow zoom and pan.

Scale panel freeze rule
-----------------------

Dragging one of the two range markers of the histogram strip must not change
anything except the dashed preview line: the axis window is the one captured
when the button went down, the painted bars are a snapshot taken at the same
moment (the producer keeps feeding new frames), and the cursor-to-value mapping
uses the pressed-time window so the marker cannot chase itself.  The new range
is applied once, on release, and the strip is refreshed afterwards.

Detachable Plot settings
------------------------

The Plot settings tab can be detached into a movable/floating dock with
**View -> Detach Plot Settings**.  The tab contents are reparented between two
``QScrollArea`` widgets (the tab and the dock), so the same ``qTabPlot``
instance keeps working while the rest of the main window is busy starting or
stopping an acquisition.  Selecting the action again reattaches the tab.

Demo and tests
--------------

The demo application and the QtTest suite are built together with the GUI
when the tests are enabled::

    cmake -S slsDetectorPackage -B build -DSLS_USE_GUI=ON \
          -DSLS_USE_TESTS=ON -DCMAKE_BUILD_TYPE=Release
    cmake --build build --target plot_demo plotting_test -j

    # standalone demo / offline benchmark (1024x512 synthetic frame, 20 Hz)
    QT_QPA_PLATFORM=offscreen ./build/bin/plot_demo --frames 100
    QT_QPA_PLATFORM=offscreen ./build/bin/plot_demo --shot 2d-zoom-roi --png shot.png

    # QtTest suite (also registered with CTest)
    QT_QPA_PLATFORM=offscreen ./build/bin/plotting_test

`plot_demo` prints the unthrottled render throughput and the measured timer
frame rate; `plotting_test` covers the palettes and LUTs, tick/label helpers,
the FastImageView histogram window, log clamping, zero-white, cursor-fixed
zoom, viewport clamping, clipping (checked on rendered pixels), buffer
ownership, ROI overlays, the three-way histogram freeze rule, the scale panel
re-entrancy, 1D and 2D behaviour and the 1024x512 >= 20 Hz acceptance
benchmark.

Removing Qwt
------------

`libs/qwt` is gone from the source tree and the build; `slsDetectorGui` no
longer finds or links Qwt, and no source under `slsDetectorPlotting` includes
a `qwt_*` header.  The package metadata and documentation refer to the Qt
runtime packages instead (`qt6-qtbase` on RPM based systems, `qt6-base-dev`
on Debian/Ubuntu, `qt6-main` in conda).
