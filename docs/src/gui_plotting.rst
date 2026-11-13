Plotting in slsDetectorGui
==========================

The plots of `slsDetectorGui` are painted with plain Qt widgets. The code is
in `slsDetectorGui/slsDetectorPlotting` and needs only the Widgets, Gui and
Core modules of **Qt 6 (>= 6.2)**. It is built as the static library
`slsDetectorPlotting`, which the GUI, the tests and the demo link.

Using the 2D image
------------------

* The wheel, or a pinch on a touchpad, zooms around the cursor. Dragging
  moves the image and a double click shows all of it again.
* Zoomed in far enough, the pixels are outlined and their values are written
  into them. Integers are written out in full, other values with five
  significant digits.
* Position and value of the pixel under the cursor are shown in a corner of
  the image.
* The strip below the image is a histogram of the image. The band between the
  two markers is the range of the colour scale, drag a marker to change it.
  A range picked this way is kept until a z range is set in the Plot tab or
  the size of the image changes.
* The boxes below the strip select the palette and switch the colour scale
  and the height of the histogram bars between linear and logarithmic.

The 1D plot zooms in on a rectangle drawn with the left button, zooms out one
step with a right click and to the full range with Ctrl + right click, and is
moved with the middle button.

The Plot settings tab can be detached with **View -> Detach Plot Settings**,
so that it stays at hand while the other tabs are used.

Components
----------

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Component
     - Responsibility
   * - `sls::plot` (SlsQtPlotCommon)
     - Palettes, axis ticks and colours shared by all plots.
   * - `SlsQtFastImage`
     - The image with its axes, zoom, pixel labels, cursor readout and
       overlays (ROI boxes and contour lines).
   * - `SlsQtScaleHisto`, `SlsQtScalePanel`
     - The histogram strip with the two markers, and the strip together with
       the palette and lin/log boxes.
   * - `SlsQt2DPlot`, `SlsQt2DHist`
     - The 2D plot of the GUI: an image with a scale panel, and the data with
       its automatic z range. Also draws the gain plot.
   * - `SlsQt1DPlot`, `SlsQtH1D`
     - The 1D plot of the GUI and one curve of it.

How the image is drawn
----------------------

* Pixel `(ix, iy)` covers `[ix, ix+1) x [iy, iy+1)`, y points up and row 0 is
  at the bottom. The rows are flipped once, when the data is turned into a
  `QImage`.
* The data goes through a 256 entry colour table into one `QImage` per frame,
  which is scaled when painting. Zooming and panning do not touch the data.
* With a logarithmic colour scale, values `<= 0` are shown as `1e-2`, so that
  empty or negative (pedestal corrected) pixels do not disappear.
* The histogram covers the range of the colour scale plus half of it on both
  sides and is updated a few times per second.
* A view chosen with the mouse always fills the plot area. Axis ranges from
  the Plot tab are shown exactly, with empty borders where their shape differs
  from the plot area. The pixels are always square.

Tests and demo
--------------

Both are built together with the GUI when the tests are enabled::

    cmake -S slsDetectorPackage -B build -DSLS_USE_GUI=ON -DSLS_USE_TESTS=ON
    cmake --build build --target plotting_test plot_demo -j

    # tests of the plotting widgets, also run by ctest
    QT_QPA_PLATFORM=offscreen ./build/bin/plotting_test

    # the plots of the GUI with synthetic frames, no detector needed
    ./build/bin/plot_demo
    ./build/bin/plot_demo --help
