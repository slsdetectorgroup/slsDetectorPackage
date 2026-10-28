# jf_online_display_gui — online Jungfrau display **+** event finder in one process

A rewrite that merges the two separate programs

| old | role |
|---|---|
| `jf_filter.cpp` + `jf_event_finder_cli/gui.cpp` | ZMQ subscriber → pedestal subtract → threshold gate → ZMQ publisher |
| `onlinedisp_zmq.cpp` | Qt5 GUI + ROOT plots, subscribed to the filter's output |

into a **single multithreaded program**. The intermediate ZMQ hop between the
filter and the display is gone: one subscriber reads `slsReceiver`, the gate
decides pass/drop on every frame, and only passing frames reach the display and
the analysis.

It also supports slsReceiver's **multi-UDP-interface ("threads") mode**, where
the module is streamed as several ZMQ ports — see
[Multi-interface mode](#multi-interface-mode-two-udp-interfaces).

The originals in the parent directory are untouched.

---

## Build

Two equivalent builds exist: **CMake** (same as the rest of the package) and the
self-contained Makefile.  Both produce the same two programs from the same
sources.

```sh
# CMake (out-of-tree, nothing is written into the source directory)
cmake -S . -B cmake-build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build -j                 # both variants + tests
./cmake-build/jf_online_display_gui          # normal
./cmake-build/jf_online_display_gui_JFSTRX   # LGAD strip-xel variant
./cmake-build/gate_contig_test               # gate unit test (ctest also runs it)
./cmake-build/jf_feeder                      # synthetic slsReceiver
#   -DJF_BUILD_TESTS=OFF skips the test and the feeder
#   -DJF_ZMQ_STATIC=/path/libzmq.a overrides the ZMQ library
#   (Qt5, ROOT and HDF5 are found with find_package; without a usable
#    static libzmq the build falls back to the system one via pkg-config)

# Makefile (identical binaries, in-tree objects)
make                 # -> ./jf_online_display_gui      (Qt5 + ROOT + static libzmq)
make                 # also builds ./jf_online_display_gui_JFSTRX, the
                     # LGAD strip-xel variant of the same sources
                     # (-DJFSTRX; `make normal` / `make jfstrx` build one)
make feeder          # -> ./test/jf_feeder      (synthetic slsReceiver, testing)
make clean
make jfstrx          # optional -DJFSTRX (LGAD strip-xel) build
```

Deps: system ROOT (`root-config`), Qt5 (`pkg-config`), HDF5 (`libhdf5_cpp`,
`libhdf5`), rapidjson headers, and the prebuilt `libzmq.a` from
`../../../build/_deps/libzmq-build/lib/`.

## Run

```sh
./jf_online_display_gui <rx_host> [rx_port] [gate_threshold] [min_pixels] [pede_file] \
                 [--pub PORT] [--contig] [--gate|--no-gate] [--stats] \
                 [--use-pede|--no-pede] [--acquire-pede] [--pede-frames N]
```

The positional arguments are the union of the two originals:

| arg | meaning |
|---|---|
| `rx_host` / `rx_port` | slsReceiver stream (default port 30001). `rx_port` may be a `+` list for the multi-interface mode: `30001+30002` |
| `gate_threshold` | event-finder gate threshold, pede-subtracted ADC (default **150**) |
| `min_pixels` | gate: pass when ≥ N pixels are above threshold |
| `pede_file` | pedestal file (default `./current_pede.dat`) |

Options: `--pub PORT` additionally republishes gated frames (header+data, same
format as the old filter) for third-party subscribers; `--contig` enables the
contiguity proxy; the **gate is off by default** (`--gate` enables it,
`--no-gate` is the default transparent passthrough); **pedestal subtraction is
off by default** (`--no-pede` is the default "raw ADC" mode, `--use-pede`
starts with subtraction enabled); `--stats` prints
the old CLI counter line once per second; `--acquire-pede` averages
`--pede-frames N` frames into a live pedestal as soon as acquisition starts;
`--gate-roi X Y W H` starts with a gate ROI already set (data rows: y = 0 is the
bottom row, the same convention as the ROI list and every plot - see
[One coordinate system](#one-coordinate-system-image-and-root));
`--sc N` passes only frames from storage cell N (0..15, bits 8-11 of
`detSpec3`; `--sc all` for no cell gating); `--raw-map` shows raw ADC in the
main map instead of photon counts.  The **photon-map threshold** (ADU, default
150) is no longer an argument: it is the control in the *Virtual photon
counting* panel, and the `--stats` line reports it with its unit (`pthr=150ADU`
or `pthr=4keV`), together with `kev=off|loaded|nominal`.  `--apply-gain` starts
with the gain correction on, so a scripted run can use the keV threshold.

Saving and the photon-map display can also be pre-set, which is what the
headless checks use: `--pmap-every N` (photon-map display cadence, default
2000), `--pmap-save-every N` (saved-map cadence, minimum 200, default 2000),
`--save-photon` (write the virtual photon counting maps instead of the frames),
`--save-budget N` (stop after N saved frames, or N saved maps in photon mode)
`--start-save` (press the Save button at startup) and `--apply-gain` (press the
Gain button).  Each of them does exactly what the corresponding control does, so
a run can be driven from a script.

```sh
# was:  jf_event_finder_gui <rx_ip> 30001 30011 ./current_pede.dat
# then: LD_LIBRARY_PATH=/usr/lib64/root ./onlinedisp_zmq <energy> <filter_ip> 30011
# now:
./jf_online_display_gui <rx_ip> 30001 100 5 ./current_pede.dat
```

## Threads

| thread | owns / does |
|---|---|
| main / Qt | `QApplication::exec`, all widgets, button + shortcut handlers |
| **acq** | `zmq_poll` over every interface, receive, frame pairing/assembly, gate decision + counters + rates, optional republish, live pedestal acquisition, hands the frame to the mailbox |
| **analysis** | pedestal subtraction, ADC/gain images, photon counting, and the **only** thread that touches the source ROOT histograms (per-frame work accumulates into plain arrays; the histograms are synced at the redraw cadence) |
| **ROOT** | canvas painting, `gSystem->ProcessEvents`, the user command queue, paints the display clones |

Why the gate is not done in the analysis thread: the gate must count **every**
received frame, while the display is allowed to fall behind. A single-slot
"latest frame wins" mailbox decouples them — dropping an old frame there is the
merged equivalent of the ZMQ receive-HWM drops of the old chain, and the gate
counters stay exact.

All interfaces are read by the **one** acq thread via `zmq_poll`, so pairing
needs no extra synchronisation.

## Multi-interface mode (two UDP interfaces)

`slsReceiver` with N UDP interfaces ("threads") publishes **one ZMQ stream per
interface on consecutive ports**, each carrying one Y-slab of the module:

* `DetectorImpl.cpp` → `portnum = getClientStreamingPort() + (iSocket % numUDPInterfaces)`
* `JungfrauData::UpdateImageSize()` → `nPixelsX = 256*4 = 1024`,
  `nPixelsY = (256*2)/numUDPInterfaces` → **256 rows per interface for N=2**
* `GetPortGeometry()` for JUNGFRAU → `portGeometry.y = numUDPInterfaces`
  (`numPorts = {1,2}`), i.e. the split is along **Y**

So for one Jungfrau module split in two, each port streams 1024×256
(524288 B) and the two stack into 1024×512:

```sh
# slsReceiver started with 2 interfaces -> ports 30001 (bottom) and 30002 (top)
./jf_online_display_gui <rx_ip> 30001+30002 100 5 ./current_pede.dat
```

* **Order = image order.** The list is read bottom → top, the first port
  occupying the lowest rows. To swap the halves, reverse the list
  (`30002+30001`) — no extra flag.
* **Assembly.** Each slab's geometry comes from its own JSON header
  (`shape`, `size`), and the slabs are copied into the full 1024×512 frame.
  An interface that streams a *full* 1024×512 buffer instead is handled too:
  only its own slab is taken out of it.
* **Pairing and alignment.** Every socket is drained continuously - nothing is
  ever held back, so the receiver's buffer stays empty - and the newest slab of
  each interface is paired. If their frame numbers differ the pair is dropped,
  so an assembled frame is always stitched from a single detector frame (see
  [Keeping the interfaces aligned](#keeping-the-interfaces-aligned)). If no
  pair succeeds for 500 ms the interfaces are permanently offset (one of them
  keeps skipping): the newest slabs are then paired anyway to keep the stream
  alive, and strict pairing returns as soon as the numbers match again. A dead
  interface never stops the others; it is reported, not waited for.
* **Everything downstream is unchanged**: gate, images, photon map, T vs B /
  L vs R, ROOT fills and the pedestal layout all operate on the assembled
  1024×512 frame, so the existing `current_pede.dat` stays valid.
* **Status** is per interface in the GUI and in `--stats`
  (`stream=connected ports=connected,idle`), each with its own
  `zmq_socket_monitor` connection state.
* **Start-up checks** are printed once per interface, which is how you confirm
  the orientation on real hardware:

  ```
  [stream] port 30001: 1024x256 (524288 B) -> rows 0..255 of 1024x512
  [stream] port 30002: 1024x256 (524288 B) -> rows 256..511 of 1024x512
  [frames] first assembled frame: port 30001 -> row 0 = 100, port 30002 -> row 256 = 150
  ```

  If the second line shows the *top* half arriving on the first port, reverse
  the port list.
* **`--pub`** republishes the assembled full frame (a synthesised header with
  `size=1048576`, `shape=[1024,512]`), so downstream consumers see the same
  single-stream format as before.
* **Limits.** This is the single-module display (1024×512). A port count that
  does not divide 512 rows evenly (or a slab height that is neither 512/N nor
  512) is refused with an explicit error instead of producing a wrong image.

## Analysis stage: deferred histogram accumulation

`TH1::Fill()` was called once per pixel per frame for the chip spectra and
again for the photon map — 0.6-1 M fills/frame, ~4-6 ms, i.e. the whole cost
of the analysis stage (measured: 239 fps without the photon map, 179 fps with).
Filling a histogram that is painted once per `rootUpdateMs` at frame rate is
pure waste, and ROOT histograms are not thread safe anyway (bin array *and*
shared statistics), so calling `Fill()` from several threads is not an option.

Instead the analysis thread accumulates each frame into plain arrays:

* `g_pmapAccum[NCH]` — photon map (and the raw-ADC map, `--raw-map`);
* `g_chipAccum[8][NBIN+2]` — chip spectra, including under/overflow;

and pushes them into `his3000` / `hchip` at the redraw cadence
(`clamp(rootUpdateMs/2, 50, 500)` ms), under `gHistoMtx`:

* the photon map is written with `SetBinContent` (absolute contents, since the
  accumulator is cumulative) — a 524288-bin copy at most 20x/s is free;
* the chip spectra are filled with the **delta** per bin, so ROOT's own
  statistics keep working; the bin index is computed directly and was verified
  against `TAxis::FindBin` over every bin edge and 20M random values;
* `SetEntries` restores the true entry count, because `Fill(x,w)` increments
  `fEntries` once per *call*, not per weight;
* "Reset His" zeroes the accumulators and the histograms together.

Verified exactly against an analytic prediction (two 1024x256 slabs of ADC 100
and 150, zero pedestal, threshold 150): every chip bin/entry and every photon-map
bin matched to the last digit.

Measured on a live receiver streaming ~1500-2000 frames/s:

| | before | after |
|---|---|---|
| analysis frames/s (chip fills only) | 239 | **538** |
| analysis frames/s (+ photon map) | 179 | **513** |
| photon-map cost | -25% | -5% |

The display still subsamples (analysis ~500 fps vs stream ~2000 fps), so
`displaydrop` remains the counter to watch — it is now ~68% instead of ~88%.
One cosmetic difference: the chip-spectrum stats box mean/RMS is computed from
bin centres rather than exact ADC values (sub-bin, <0.1% on the mean), while
entries and all bin contents are exact.

### Where the analysis time goes

`JF_TIMING=1 ./jf_online_display_gui ...` prints a per-step breakdown when the program
exits. On a live stream (~2000 fps, two interfaces, full pedestal set):

| step | us/frame | share |
|---|---|---|
| mailbox take + frame memcpy (1 MB) | 36 + 21 | 3 % |
| step 1: ADC/gain image build (524288 px) | 527 | 27 % |
| step 2: pedestal subtraction + photon counting + chip binning (524288 px) | 1294 | 67 % |
| step 3: T vs B / L vs R (1536 ROOT fills) | 16 | 1 % |
| step 4: raw-ADC map (only with `--raw-map`) | 0 | - |
| accumulator sync (amortised) | 10 | 1 % |
| step 5: image publish (every `HDraw_every` frames) | 16 | 1 % |
| **total** | **1920** | 521 fps |

(One run; a second run on the same shared machine gave 2294 us / 436 fps, so
treat these as +/-25 %. The *shape* is stable: copies ~2 %, step 1 ~30-40 %,
step 2 ~55-65 %.)

So the bulk memcpys are *not* the limit (1 %). The two per-pixel passes are
94 %, at ~1.0 ns/px (step 1) and ~2.5 ns/px (step 2) — i.e. ~3 and ~7 cycles
per pixel of scalar, branchy work (gain decode, `i%1024` / `i/1024` index
arithmetic, `chipBin`, the accumulator scatter). Memory is not the limit
either: the working set is ~15 MB and the loops use ~5-12 GB/s, while a
single-core memcpy on this machine sustains ~110 GB/s (L3-resident).
A live-acquired pedestal (no g1/g2 streams) only saves ~80 us of step 2, so
halving the pedestal traffic would buy ~4 %.

## Parallel analysis (worker threads)

Steps 1 and 2 are pure per-pixel work over disjoint ranges, and after the
accumulator change they touch no ROOT state at all (`adc_image`, `gain_image`,
`hpmapAccum` and `hchptr` are written per pixel; only the chip spectra are a
scatter, handled with small per-worker copies that the controller merges once
per frame). So they can simply run on several threads:

```
--analysis-workers N    1 = single threaded, 0/omitted = auto (cores/4, max 8)
```

The worker threads are persistent; a frame is handed over with a generation
counter (two condvar handshakes per frame, a few us). Worker 0 is the analysis
thread itself, so nothing changes for `N=1`. Step 3 (T vs B / L vs R), the
histogram sync and the image publish stay on the controller.

Measured on the live stream (~2000 fps, both interfaces, full gain paths):

| workers | analysis rate |
|---|---|
| 1 | 407-415 fps |
| 4 | 952-1220 fps |
| **8** | **1340-1376 fps** |
| 12 | 1255-1343 fps (saturated) |

The results are **bit-identical** to the single-threaded run: verified with the
deterministic two-slab setup for `N = 1, 2, 4, 8` - every chip bin, entry and
photon-map bin matched to the last digit. Integer per-pixel accumulation into
disjoint ranges plus an ordered merge makes the outcome independent of
scheduling.

Practical effect at ~2000 fps: analysis coverage goes from ~20 % to ~67 %, i.e.
`displaydrop` falls from ~80 % to ~33 %.

This also leaves the door open for the planned AFC/gain-split histograms: the
per-worker accumulators are L1/L2 resident and simply grow with the number of
gain groups, with no locking added.

## Keeping the interfaces aligned

`frameIndex` cannot be used as a shared key: `DataStreamer` computes it as
`frameNumber - firstIndex` and `firstIndex` is per-streamer, so the two
interfaces can legitimately report different `frameIndex` for the same frame.
Pairing therefore uses the **absolute `frameNumber`**.

Three rules, in order of importance:

1. **Never hold a slab (no back-pressure).** slsReceiver has its own ring
   buffer; if our ZMQ queue fills, the *receiver* skips frames on that
   interface - per interface, not per frame - and the two frame numbers are
   offset from then on. So every socket is drained to its newest message on
   every wake-up and nothing is ever kept back.
2. **Pair index-aligned or drop the pair.** The newest slab of each interface
   is paired; if the numbers differ, both are discarded (`resync`) and the next
   pair is tried. An assembled frame is therefore always stitched from one
   detector frame, and no frame is ever shown or saved half-filled.
3. **Latch when the offset is structural.** After 500 ms without a successful
   pair the newest slabs are paired anyway (counted separately and logged) so a
   permanently offset interface still streams; the first matching pair returns
   the pipeline to strict pairing.

Decimation is a manual choice: `--decimate 2|4|8` assembles one frame in N,
chosen by the shared frame number so both interfaces drop the same frames. The
automatic ladder (`--auto-decimate`) reacts only to frames actually queued in
the gate (`framesIn - framesPass - framesDrop - framesScDrop`) - counting gate
rejects as backlog used to pin it at 1/8 - and climbs back after 5 s without a
backlog. Everything is audited through `--stats` (`resync`, `behind`,
`incomplete`, `[align] keep=1/N shed=n`) and the rate-limited log messages.

## Display controls

| button | key | action |
|---|---|---|
| Start/Pause | `s` | pause/resume processing |
| Faster / Slower | `+` / `-` | main image every N *analysed* (gate-passed) frames, but never slower than one every 250 ms |
| gain correction on/off | `g` | divide by ADU/keV (gain map, else nominal 40) - the `gain corrected` tick next to it is a read-only indicator |
| BW Pal. / BW Pal. 2 | `b` / `Shift+B` | monochrome palettes |
| **Col Pal** | `c` | back to the colour palette |
| Set Ranges | `S` | ADC / photon-map / chip-X ranges |
| Update Rates | | main image cadence + ROOT redraw period |
| Reset His | `r` | clear the photon map and spectra |

The photon map itself is **not** in the control window any more: it has its own
window, `Photon Map (virtual photon counting)`, whose panel is *Virtual photon
counting* (left column, under the gates):

| control | action |
|---|---|
| cumulative (toggle to reset) | off by default: each map covers the frames since the previous publish, so the picture shows the *rate* (turn it on for a running total).  Toggling it restarts the map from scratch, so the switch doubles as the reset |
| plot every N images | publish cadence of the map, counted in **analysed frames** (default 2000); independent of the ADC image cadence |
| photon threshold | threshold of the photon map; a pixel above it counts `(value + threshold) / (2 x threshold)` photons.  It is in **ADU** while the gain correction is off (default 150) and in **keV** while it is on (default 4.0): the counting value is then the pedestal-subtracted ADC divided by the pixel's ADU/keV (gain map, else the nominal 40).  Both numbers are kept, and switching the gain swaps which one the row shows, so neither is ever overwritten.  Toggling the gain while a photon-map save runs changes the unit of the maps written from that moment on, which the run announces (`[gain] warning: the photon-counting unit is now keV ...`); the JSON keeps the value it recorded at save start |

The photon window has no ROI and no gate: the ADC image, the gain image and the
photon map are three independent windows.

**The image windows are plots.**  Each one draws the image inside a frame with
X/Y axes, tick labels *outside* the plot area and a light dotted grid inside it
(the Y labels are data rows, row 0 at the bottom, like everywhere else).  When
the zoom makes an image pixel at least **16 screen pixels** wide, every pixel is
outlined with a thin line; above **50 screen pixels** the value is written inside
each pixel - whole numbers for the photon map (counts), one decimal for the
real-valued ADC/keV images.  The colour bar that used to sit at the right of the
image is replaced by a thin **scale panel** under it (its histogram strip is
~38 px, about a tenth of the window):

* the **histogram** of the last image, one solid bar per bin - the bar is as wide
  as the bin spacing - with the mapped range highlighted;
* the two range **markers are pinned at 1/4 and 3/4** and the value axis
  rescales under them (the strip shows the selected range widened by half a
  range on each side), so **dragging a marker** moves that end of the range; the
  ends are printed next to the markers and in the spin boxes;
* a **palette** chooser with the matplotlib family that is useful for detector
  images: the original `Col (jet)` / `BW grey 1` / `BW grey 2` (also on `b` /
  `Shift+B` / `c`), `Turbo`, the perceptually uniform `Viridis`, `Plasma`,
  `Inferno`, `Magma` and `Cividis` (colour-blind friendly), `Hot`,
  `Grey (linear)`, `Grey reversed`, the diverging `Coolwarm`, `BWR`, `Seismic`
  and `RdBu` (handy for pedestal-corrected data that goes negative),
  `YlGnBu (Brewer)` and `Sunset (Tol)`.  Pick one per window; the ROOT plots
  follow the `b` / `Shift+B` / `c` keys as before;
* **two lin/log switches** - `image` for the intensity mapping (log maps
  log10(value) and caps values <= 0 at 1e-2 so empty pixels stay visible) and
  `histo` for the bar heights (log10(count + 1)); the histogram's *value* axis
  always follows the image switch;
* `min` / `max` spin boxes for the same pair as `Set Ranges...`.  Pixels with **no photons are drawn
white** - the way ROOT's `colz` leaves empty cells on the white canvas - while
every pixel above zero takes the palette colour, so a faint map stays readable
instead of drowning in the palette's lowest colour.  The colour bar keeps the
palette itself (dark blue at the bottom), exactly as the ROOT plot's did; the
ADC and gain images still map zero to the palette's first colour.
| Reset Axis | `R` | unzoom everything: the ADC and gain images *and* the ROOT plots |
| ROI only | `i` | capture the ADC zoom as the gate ROI (see below) |
| Save ROI (ROI panel) | `j` | add the active gate ROI to the ROI list |
| Unset ROI (ROI panel) | | forget the current ROI: gate, ROI spectra and the saved data go back to the full frame (the ROI list on disk is untouched) |
| Load ROI... (ROI panel) | `k` | pick a ROI from the list and apply it |
| keV PH (Gain panel) | `e` | show the PH spectra in keV (see below) |
| Start/Stop save | `n` | write gate-passed ROI frames to HDF5 (see below) |
| Acquire/Save/Read/Load Pede | `p` / `O` / `o` / `P` | pedestals |
| Quit | `q` | |

The `Surf2Z`, `ColZ` and `G1+/G1-/G2+/G2-` buttons (and their keys `u`, `U`,
`C`, `[`, `]`, `{`, `}`) were removed: in the standard build the surf/colz
commands only changed a histogram that is never drawn, and the gain offsets are
constant 0 until the AFC/gain-split work lands.

The three palette buttons (`BW Pal.`, `BW Pal. 2`, `Col Pal`) sit together in
one row of the Display group.

**ROI panel** (below the Display).  Everything about *which* ROI the gate works
on lives here:

| control | what it does |
|---|---|
| `Select ROI (ADC zoom)` | makes the region currently zoomed in the ADC image the **current ROI** |
| `Save ROI` (`j`) | adds the current ROI to `./roi_list.dat` |
| `Load ROI...` (`k`) | picks one from that list (the dialog can also delete) and makes it current |

Selecting and loading only *choose* the ROI - they do not restrict anything.  The
read-out underneath shows the current ROI, whether the gate is restricted to it,
and how many are saved.

Next to the signal gate there is still `ROI only (i)`, which switches the
restriction on: the gate then counts only inside **the current ROI**, whatever it
is at that moment (re-selecting or loading another one while it is on takes
effect immediately).  Pressing it with no ROI selected pops a warning and leaves
the button unselected; switching it off goes back to the full frame but keeps the
current ROI.

**Gain panel.** Holds `gain correction on/off (g)` - a checkable button - the
read-only `gain corrected` tick next to it, and the gain map used for the keV
spectra (see below).  The tick only *reports* the state (the button is the only
control); it is greyed together with the panel while the correction is off.

`gain correction on/off` pushed means *divide every pixel by its ADU/keV*: the
per-pixel
value from the gain map when one is loaded, or the **nominal 40 ADU/keV** for
all pixels otherwise.  That is what makes the keV spectra physical - with a
320 ADU pixel, the gain correction gives 8 keV either way, but the map corrects for the
pixel-to-pixel spread (30-55 ADU/keV in the lab file) while the nominal treats
every pixel alike.

The label under the row says exactly which calibration is in use: the gain-map
file name when one is loaded, `no gain map loaded : using defaults` when the
correction runs on the nominal 40 ADU/keV, and `gain off: PH spectra in ADU`
when the stage is bypassed.

`keV PH` then chooses what the PH canvases show: the **keV** spectra when it is
ticked *and* the gain correction is on, the **raw pedestal-subtracted ADC**
otherwise.  Pressed state, tick and greyed-out states line up with this: with
the correction off the Gain box, the `keV PH` checkbox and the `gain corrected`
tick go grey (the `keV PH` tick is kept for next time) and the plots show ADC.

The saved data is unaffected: `Save` always writes pedestal-subtracted ADC
(with the Jungfrau gain-bit conversion), never the display's keV choice.

**Pedestal tracking.** Pedestals drift slowly, so there is a `Pedestal
tracking` panel of its own, directly under `Pedestal`, with a `pedestal
tracking on/off` toggle (also `--pede-tracking`); a small up-arrow shows that
its result goes back into the Pedestal box.  The box greys while tracking
cannot be *selected* - i.e. while the signal gate is off - not while tracking is
simply inactive: whether it is on is shown by the value label under the button
(`off`, or `on: N samples, delta D ADU, U updates`).  It only makes sense with
the signal gate on - that is the button's enable condition, and turning the gate
off turns the tracking off - because it works from the frames the gate *drops*,
i.e. while the beam is off.  The rules:

| step | default |
|---|---|
| feed the average only while the signal gate drops | > 90 % of the frames reaching it |
| sample of every dropped frame | 0.2 % of its pixels, with the stride phase **rotated** each frame so every pixel is covered within 500 frames |
| per-pixel running average, pooled window | 10 M pixel samples (~19 updates/pixel, weight `NCH/10M` per sample, ~6 min at 25 fps) |
| replace the pedestal in use when the largest pixel difference **inside the gate ROI** exceeds | 5 ADU |
| rate limits | at most once per 10 s, at least once per 10 min |
| pedestal tracking image in the save file | every 10 min, while tracking is on |

Sampling happens on every dropped frame; the (ROI-wide) distance test is the only
expensive part and runs about once a second.  The 5 ADU test looks only at the
**gate ROI** - drift outside it is not worth
acting on - while the average itself is still accumulated over the whole frame.
With no ROI selected the whole frame is used.  High-gain / gain-3 pixels are
skipped when feeding the average (they live on a different scale), and the
pedestal is replaced in memory only - the file you
loaded is untouched, and loading a pedestal (or `Acquire Pede`) re-primes the
average from it.  The panel read-out shows `samples`, the current `delta` and
the number of `updates`, the `Pedestal:` line in Status gains a `[tracked]` tag,
and `--stats` carries `ptrack=off|on|on-upd`.  Each update logs

```
[pedetrack] pedestal updated #3: max delta 7.41 ADU in the ROI over 152340 pixel samples (drift > 5 ADU)
```

While tracking is on, the save file gets a time series: `/pedestal tracking`
(float32, one 512x1024 image per 10 min, in the ROI shape) with the matching
`/pedestal tracking frame`, so the drift during a run is recoverable.  The two
datasets are created (empty) for every save because SWMR mode cannot create
objects after it starts; a baseline image is always written when a save starts,
and further images only while tracking is on.

In `Pedestal corrected` mode the saved frames follow the tracker too: every
frame is corrected with the pedestal **in use when it was captured**, exactly
like the display and the gate, so a live-tracked pedestal is in the file and
not just in the display.  A save that spans several pedestal updates therefore
uses several pedestals, documented by `/pedestal tracking` and
`/pedestal tracking frame`; the single `/pedestal data` image is the pedestal at
save start (the baseline), not a description of every slice.

**Pede/Raw.** The Pedestal panel has the `Pede/Raw toggle` (equivalently
`--no-pede` / `--use-pede` on the command line), and raw ADC is the **startup
default** - the button starts pressed; `--use-pede` starts with subtraction
enabled instead.  Raw mode
ignores the loaded pedestal *everywhere* -
display, photon map, spectra, event finder and the ROI save all work on raw ADC
- and the Pedestal panel, the *Virtual photon counting* box (photon counting
thresholds the pedestal-subtracted ADC, so the map goes empty with it) and the
flow box grey out.  The state is recorded in
the saved file's `gate` JSON as `"no_pede"`, and the panel says `pede subtracted`
/ `no pede: raw ADC` next to the button.

Note what that does to the **signal gate**: with no pedestal in play it compares
the threshold against the raw 14-bit value, `frame[i] & 0x3fff` (and skips the
gain-bit scaling), so the threshold is in raw ADC units - the gate row label
changes from `Threshold (ADC, pede-subtracted)` to
`Threshold (raw ADC - no pede)` to make that visible.  The same happens with no
pedestal file loaded at all, since that is the same code path.  Measured with a
200 ADU pedestal, a 120 ADU threshold and halves at 3000/200 ADU: 262144
pixels/frame with the pedestal, 524288 with `--no-pede` (the 200 ADU half stops
being pushed below the threshold) - i.e. the raw half really is counted.  (The gain-1/gain-3
pedestal offsets still have no controls; the panel is the natural home for them
if the gain-split work needs them back.)

**Display cadence.** The ADC/Gain image windows are refreshed on the frames the
gate passes, decimated by the `Faster (+)` / `Slower (-)` counter (default every
20).  The counter alone would freeze the image when the gate passes rarely - at
0.1 Hz and `every=20` that is one image every 200 s - so a frame is also
displayed when the previous one is older than `DISPLAY_FLOOR_MS` (250 ms).
Under that floor every passed frame is shown, and above it the decimation is
untouched: at a 2 Hz pass rate `shown/pass = 1.00`, at 200 Hz `shown/pass = 0.05`
(1/20).  The floor only takes over when `pass rate / every` drops below 4 Hz.
`--stats` reports `shown=`, the images actually handed to the view, next to
`pass=` so the ratio is auditable.  The spectra, the photon map and the ROOT
repaint timer are unaffected by this: they follow every passed frame.

**Ranges.** `Set Ranges (S)` sets the axes explicitly instead of letting ROOT
auto-scale them.  It shows the ranges that are **in force right now** - read
back from the displayed histograms, not from stored numbers - so it opens on
what you are looking at (including a scale you dragged on the ROOT canvas), and
**nothing is applied unless you edit it**:

| field | applies to | default |
|---|---|---|
| ADC min / max | the colour scale of the **ADC image** window - the Qt "main plot", also draggable on its colour bar | -200 / 1000 |
| P.Map min / max | the colour scale of the ROOT 2D map (canvas `A5`) | 0 / 256 |
| PH min / max | the shared x window of the spectra (`A4` and the ROI spectrum `A7`) | -200 / 3000 |

One pair per display, and **each field opens on the scale that display is
using right now** (read back from the widget / the histogram, so it also picks
up a drag on the image colour bar or on the ROOT canvas).  Only the values you
actually edit are applied, and each goes to its own display: the ADC pair is a
Qt widget and repaints immediately, the ROOT pair is applied once by the ROOT
thread.  Because it is applied once, a scale you then change on the ROOT canvas
is not reset by the next redraw (`TH1::Reset()` does not touch it either).
`Reset Axis (R)` releases the fixed ranges and the ROOT axes auto-scale again;
a double-click on the ADC image colour bar restores its last applied ADC range.

ROOT snaps a requested window to the nearest bin edges, which is why the
`-200 / 3000` default reads back as `-212 / 3010` on the 500-bin spectrum axis.
`JF_TIMING=1` prints the ranges actually in force on exit.

**The control window is a flow chart.** The left column is the gate flow (1.
Storage cell, then 2. Signal gate), the right column carries the rest of the
chain (Pedestal, then Gain, then Display - the display shows the result, so it
comes after the processing), the ROI-save panel spans the full width under both,
and Status (left) and Counters (right) sit across the bottom.

The arrows are **static** - the chain is always drawn the same way - and a
stage that is not in use is **greyed out** (title, text and background), so the
bypass is obvious without anything moving.  Greyed means:

| panel | greyed while | its own switch (kept readable) |
|---|---|---|
| 1. Storage cell | the filter is `ALL` (no cell gating) | the cell combo |
| 2. Signal gate | `Gate enabled` is off (passthrough) | `Gate enabled` |
| Pedestal | no pedestal is loaded, or `Pede/Raw` is on raw | `Pede/Raw toggle` |
| Pedestal tracking | the signal gate is off, so it cannot be selected (inactive-but-selectable stays black; the value label below shows on/off) | `pedestal tracking on/off` |
| Gain | the gain correction is off | `gain correction on/off` |
| Virtual photon counting | no pedestal in play (the map needs `g0`) | - |
| ROI | never (the ROI list is independent of the gate) | - |

Greying is a **look, not an enablement**: a greyed panel is still clickable, and
the control that switches the stage back on keeps its normal text colour (only
the box, its title and the other contents dim), so a bypassed stage never looks
stuck.  The `keV PH` checkbox is the deliberate exception: it is disabled *and*
greyed while the gain is off (its tick is remembered for next time).

```
1. Storage cell --+                       Pedestal
                  |                          ^  |
      (down)      +--> Pedestal --+          |  +--> Pedestal tracking
                  |              |       (drops feed the average)
   (raw: bypass)  |              v
                  +----------> [funnel] <-- 2. Signal gate  (the gate CONTROLS
                  |              |                           the funnel; it does
                  |              |                           not modify data)
                  |              +--> Gain --+--> Display
                  |                   |      ^       ^
                  |                   |      |       |
                  |     Virtual photon counting -----+  (its own window)
                  |                   ^
                  |                   +-- calibration: the threshold is keV
                  |                       while the gain is on
                  |
                  |                   +----> ROI --> Save
```

The gate is drawn as a **funnel on the line**, not as a stage the flow enters and
leaves: nothing goes in or out of the gate box, and a short blue arrow from the
gate panel into the funnel's side shows what does the gating.  With the gate off
the funnel and that arrow grey out.  Two feeders reach the funnel, and they do
**not** have to agree, because the display and the file can use the pedestal
differently:

* the **display** chain uses the pedestal when `Pede/Raw` is on "pede subtracted"
  and one is loaded (the same condition that greys the Pedestal panel);
* the **saved** data uses it when `virtual photon counting` is the save mode (the
  maps always need `g0`) **or** when `save frames` is selected with
  `Pedestal corrected` ticked.  So in the default `save frames` + raw setting the
  file takes the *bypass* straight from the storage cell while a corrected
  display still goes through the Pedestal.

Both feeders can therefore be live at once, and the colour says what each one
carries:

| colour | meaning |
|---|---|
| **orange** | everything the saved data travels through, entry point to the Save panel: `save frames` = storage cell -> (Pedestal or bypass) -> funnel -> Gain -> ROI -> Save; `virtual photon counting` = ... -> funnel -> Gain -> counting -> Save.  The ROI is only there for the file, so its input is always orange too - from the **Gain** when frames are written, from the **counting** when the maps are (they are cropped to it on the way out) - and the arrow into the Save panel always leaves the ROI, never the counting panel |
| **light blue** | the branches that only feed the display: storage cell -> Pedestal and Pedestal -> funnel while the file bypasses them, Gain -> Display, Gain -> counting, counting -> Display, and Gain -> ROI when the maps are the saved product |
| **lighter blue** | the pedestal tracking pair only: the gate block -> tracking input and the tracking -> Pedestal feedback |
| **pale grey** | the gate funnel while the gate is off (the symbol greys instead of vanishing) |

Nothing is drawn greyed-out any more: a feeder that carries nothing is simply not
there, and the Save arrow of the mode that is *not* selected is not there either.
The tracking input leaves the **top-right corner of the gate block**.  (The flow
strip's tooltip repeats the legend.)

The `Pedestal tracking` panel sits directly under `Pedestal` with a small `^`
arrow back up into it: it is fed by the frames the *signal gate drops* (the `-->`
from the gate) and its result is written back into the Pedestal box above it.

Those connectors are drawn (orthogonal lines with a filled head) in the narrow
strip between the two columns, which is why the boxes are padded to line up.
The two remaining vertical arrows (Gain -> Display, ROI -> Save) are arrow
glyphs in the layout; the gap between the two gates is plain empty space, since
the path between them goes through the Pedestal and is drawn by the strip.
Each connector turns near the box it enters, so their vertical parts never line
up into one long line down the strip.  Nothing
moves or flickers: the geometry is solved once at start-up (and again on
resize) and the bypass is shown only by greying the stage.

`Quit (q)` sits at the top right, out of the gate and arrow area, the ROI-save
panel spans the full width above the Status/Counters strip, and the boxes are
lined up so the flow is readable: the middle of the storage-cell
box sits on the top edge of the Pedestal, and the Gain starts at the middle of
the signal gate (both measured and padded at start-up, so it survives a
different font or DPI).

The event finder panel holds two separate gates:

| panel | what it keeps | cost |
|---|---|---|
| **1. Storage cell (header)** | only frames acquired in the selected storage cell (dropdown `ALL` or `0`..`15`) | a header field - filtered frames touch no pixel at all |
| **2. Signal gate (pixel counting)** | only frames with `min_pixels` pixels above the threshold (and, optionally, clustered inside the gate ROI) | the full-frame scan |

The storage cell is **bits 8-11 of `detector specific 3`** in the frame header
(0..15, 15 = the default cell), so the signal gate and the analysis only ever
see frames that already survived it - selecting a cell cuts the CPU load to
roughly 1/16 with a uniformly cycling acquisition.  Each gate can be off
independently: `ALL` switches the storage-cell gate off, the `Gate enabled`
checkbox switches the signal gate off, and the four combinations are just those
two controls (no gating, cell only, signal only, both).

`JF_TIMING=1` (or the GUI) reports the cell of the last frame seen
("last frame: SC n"), which is the quickest way to confirm the decoding on a
new setup; `scdrop` counts the frames the cell gate rejected
(`frames passed + frames dropped + frames dropped (SC) = frames in`).
`--sc N` (`N` = 0..15, or `all`) selects the cell at start-up.

**Gate ROI.** `ROI only (i)` sits next to "Gate enabled" and is a latching
button. Pressing it captures the region currently shown in the ADC image and
restricts the event finder to it: from then on only pixels inside that
rectangle are counted for the gate (threshold, min-pixels and the contiguity
test all use it). The rectangle is a **snapshot** - panning or zooming the ADC
image afterwards does not move it, so you can inspect other areas while the
gate keeps looking at the same place. It is outlined with a thin yellow box on
the ADC image, the captured size/position is shown next to the button, and
`--stats` reports it as `roi=WxH@x,y` (`roi=off` when inactive). Pressing the
button again releases it (full frame). Capturing a new ROI is un-toggle then
toggle. The receiver's own ROI (from the stream header) is a separate **red**
box on the same image - see "One coordinate system" below.

Saved ROIs live in **one list, backed by one small file** in the program
folder (`./roi_list.dat`, override with `--roi-file PATH`). `Save ROI` adds the
active rectangle (or refreshes its timestamp if it is already there), `Load
ROI...` shows the list - **most recently saved or loaded first** - with the X/Y
coordinates, width, height and when it was last used, and the chosen entry is
applied and moves to the top. The dialog also has a `Delete` button. The list
is capped at 200 entries, dropping the least recently used, so the file stays a
few kB indefinitely, and it is rewritten on every change (a crash cannot lose
it).

A ROI can also be set before the program is started, which is handy for
scripts and for headless runs:

```
./jf_online_display_gui 1000 <rx_host> 30001+30002 --gate-roi 446 114 20 11
```

### One coordinate system, image and ROOT

The operator only ever thinks in **data coordinates**: `x` grows to the right,
`y` grows **upwards**, and **(0,0) is the bottom-left pixel** of the module.
Every part of the program uses them:

| | convention |
|---|---|
| ADC/gain image | draws data row 0 at the *bottom* of the widget (the image is flipped once when it is turned into a `QImage`) |
| pixel read-out (hover) | reports data coordinates, so the number matches the plots |
| gate ROI, ROI list, `--gate-roi`, HDF5 crop | data coordinates, row 0 at the bottom |
| ROOT photon map / spectra | data coordinates, so y=`0` is at the bottom of the plot |

The two places where the flip has to be applied are the ADC overlay (data ->
screen) and the zoom that is forwarded to ROOT (screen -> data); both are done
in one place each, so the ADC box, the ROOT box and the gate rectangle are
always the *same* rectangle. A bottom ROI in the ADC image is a bottom box in
ROOT.

**Receiver ROI (red box).** Independent of the gate ROI, the receiver puts its
own ROI in every stream header as `"rx_roi": [xmin, xmax, ymin, ymax]` in raw
port coordinates. It is drawn as a **red** box on the ADC image, next to the
yellow gate ROI. It is parsed from `rx_roi`; the old `detSpec1`
reinterpretation was a hack (`detSpec1` is the bunch id) and is gone.

**The ROI has the shape of the ADC window.** The visible region of the ADC
image takes the shape of the window it is shown in, so resizing the window to
be tall makes the region - and therefore the captured ROI - narrow and tall
(the shorter side grows, so nothing you were looking at disappears; the
whole-frame view is left alone so it still captures the full sensor). ROOT then
draws that window with square pixels, so the yellow box has the same shape it
has on the ADC image instead of being stretched across the canvas: the axis
range is widened (never narrowed) around the ROI until it matches the shape of
the pad frame, which also leaves a margin of context around the box.

### JFSTRX (LGAD strip-xel) build

`make jfstrx` (adds `-DJFSTRX`) compiles in the strixel decoder
(`jungfrauLGADStrixelsData_new.h`). Two things change:

* the **ADC and gain windows stay the raw 1024x512 layout**, exactly as in a
  normal build, so the gate-ROI selection and the hover read-out keep working in
  raw data coordinates, and the receiver's **red `rx_roi` box** is a plain raw
  rectangle;
* a separate window, **"ADC Strixel Panels (JFSTRX)"**, opens with the six ADC
  groups arranged like the original ROOT ADC canvas: left column chip1
  g3/g2/g1, right column chip6 g1/g2/g3. It is display only (no ROI, no
  receiver box) and shows `raw - pede`, the same `dataMap`/`value` mapping the
  original `onlinedisp_zmq` filled its ROOT panels with.

The panels are drawn with the **rectangular pixel aspect of the strixel
groups**: the mapped pixel is `X = 75*(75/Y)` wide and `Y` tall, with
`Y = 25 / 15 / 18.75 um`, i.e. 9:1, 25:1 and 16:1. `FastImageView` therefore
scales x and y independently for these panels (the ADC and gain windows keep
square pixels).

The six panels are fed from separate group buffers, not from one tall
`335x1922` image (that geometry is larger than `NCH = 524288`, so writing it
into the raw `adc_image`/`gain_image` overflowed them). The ROOT photon map and
the T-vs-B / L-vs-R scatters also keep the raw 1024x512 layout.

### keV spectra from a gain map (`keV PH` checkbox)

The PH spectra can be calibrated to **keV**.  Press **`Gain map (keV)...`** in
the **Gain** panel to pick the file (or start with `--gainmap PATH`); the file
is

```
6 images of 1024x512 doubles, in this order:
  g0, g1, g2, hg0, hg1, hg2          (24 MB)
```

which is exactly what a `gainMaps_<module>_<...>.bin` from the lab holds.  In
normal mode **g0** is the per-pixel gain in **ADU/keV** (the module's file has
30-55 ADU/keV, mean ~42, and hg0 is the high-gain map), so

```
keV = pedestal-corrected ADC / g0[pixel]
```

**Both sets of spectra are always accumulated** - 8 chip spectra plus the
gate-ROI spectrum, in ADU *and* in keV - but only one of them is on screen,
selected by the **`keV PH`** checkbox next to the gain-map button (loading a map
ticks it for you).  The keV view needs the **gain stage active**
 (`gain correction on/off`, or `g`): while the gain is off the checkbox keeps its tick for next time but is
greyed, and the plots show ADU:

| `keV PH` | canvases `A4` (chips) and `A7` (ROI) show |
|---|---|
| unticked | the ADU spectra, x axis in ADU (e.g. -200..3000) |
| ticked, map loaded | the keV spectra, x axis in keV (the same window / 40) |
| ticked, no map | the same, using the **nominal 40 ADU/keV**; the panel says `no gain map selected - using nominal values (40 ADU/keV)` |

With the nominal constant the keV set is the ADU set divided by exactly 40, so
the two histograms have identical bin contents (the `[root] keV entries` line
reports `kev/adu=1.0000`), which is a quick way to check the conversion is
wired up right.  Nothing is converted at all while the gain is off (the Gain
panel then says `gain off: PH spectra in ADU`), so switching the gain back on
resumes the keV spectra exactly where the tick left them.

It is the *same two canvas windows*, redrawn from the other set - so nothing
goes stale, and the pad titles/stat boxes name the histogram in use (`hchip0`
vs `hchip0_keV`, `hroi` vs `hroi_keV`), which is how you can tell which one you
are looking at.

The keV axes are the ADU axes divided by the nominal **40 ADU/keV**, so a PH
window from `Set Ranges` covers the same physical region in both sets
(`-200..3000 ADU` = `-5..75 keV`), the bin indices are identical, and switching
does not move the window.  Both sets are filled from the same pixels with the
same border trim, so their entry counts are identical and the keV mean is the
ADU mean divided by the gain; with a g0 of 40 the ratio is exactly 40.

Loading a gain map is optional and only the PH spectra are affected: the ADC
image, the photon map and the HDF5 writer keep working in ADU.  Without a map
`--stats` reports `gainmap=off`, the checkbox stays disabled and the keV
accumulators are never touched (`gainmap=loaded` and matching entries once it
is there).  The two spectrum canvases are painted one per ROOT tick, only the
selected set, so lowering "Chip spectra every N redraws" in *Update Rates*
refreshes them more often.

### ROI spectrum (canvas `A7`)

`A7` holds **one** spectrum of the gate-ROI pixels: same binning, same border
trim and the same x-axis as the chip spectra, but summed over the whole
selection instead of split per chip, and drawn with the normal ROOT histogram
rendering (the chip pads use a polyline instead, because painting eight pads
with the default costs ~1.7 s per redraw).  `A9` is the same spectrum in keV
once a gain map is loaded, see below. Its entries are therefore directly
comparable with the *sum* of the eight pads of `A4` (equal when the ROI covers
the full frame). When no ROI is set it stays empty - there is nothing selected
- and the window title always names the rectangle it was filled with (or says
that there is no ROI). A ROI that is merely *selected* fills it; `ROI only` is
a separate control and only decides whether the gate is restricted to the ROI.

**Palettes are shared.** One palette definition per palette feeds both the ROOT
gradient colour table and the 256-entry lookup table of the fast ADC widget, so
the photon map and the ADC image always show the same colours. `b`/`B` switch
both to the monochrome ramps, `c` switches both back to colour; the gain image
keeps its own fixed 0..4 range colouring.

**Zoom is master/slave.** The ADC image is the master: dragging (live, not only
on release) or wheel-zooming it moves the gain image viewport immediately and
repaints the ROOT photon map (converted to data rows on the way, see above). `Reset Axis` (or double-clicking the image)
undoes it everywhere at once; a degenerate viewport is never pushed to ROOT, so
pressing it before the first frame is harmless. The queued zoom commands are *coalesced*, so a
continuous drag never floods the ROOT thread - only the newest viewport is
drawn. Double-click resets the view (and the photon map with it). Zooming *out*
keeps growing both directions until the whole frame is shown (the
window-shape matching that keeps the captured ROI in the window shape applies
while zooming in and on resize, but it must not stop a zoom-out in the longer
direction).



* ROOT graphics only ever run on the ROOT thread; other threads post `RootCmd`
  entries (`ResetAxes`, `Surf`, `Colz`, palettes, zoom, chip range).
* Source histograms are touched only by the analysis thread; the ROOT thread
  paints clones whose bins are copied once per redraw. `copyBins()` is
  deliberately lock-free (the originals accepted a torn read during a redraw).
  **"Reset His" is performed by the analysis thread** at the top of its next
  frame, where `gHistoMtx` is already held (with a non-blocking `try_lock`
  fallback on the ROOT thread when the stream is idle). The ROOT thread must
  never *wait* for `gHistoMtx`: under a heavy stream it could starve, which made
  the reset appear to take tens of seconds. Commands and forced redraws wake the
  ROOT thread immediately, so they do not wait for the `ROOT update (ms)` tick.
* `T vs B` / `L vs R` stay windowed per redraw.  The photon map is no longer a
  ROOT histogram (it has its own Qt window, see *Virtual photon counting*);
  "Reset His" clears the spectra and the map accumulator alike.
* ADC zoom propagates to the gain view and, through the coalesced command, to
  the photon-map window.

## ROOT canvas responsiveness

The ROOT thread owns every canvas and also runs the ROOT event loop, so a slow
redraw makes the plots feel dead. `JF_TIMING=1` prints what the thread spends
per redraw:

```
[root] loops=15 redraws=14 | per-redraw us: copy=5726 plot2d=345119
       plot1d=216270 | per-loop us: cmd=0 pe=248
```

Two things dominated and are fixed:

* the chip spectra were drawn with the ROOT default (a marker plus error bars
  per bin) - about 2000 X11 operations per pad, 8 pads, which cost **1.7 s**
  per redraw and starved `ProcessEvents` for ~2 s at a time. They are now
  drawn as one polyline per pad (`Draw("L")`), which is **0.22 s**, and can be
  redrawn every N-th redraw (`Update Rates...` -> "Chip spectra every N
  redraws", default 4) instead of every second.
* `ProcessEvents` now runs *inside* a redraw (between the 2D canvases and
  between chip pads), so the canvases stay interactive even while a redraw is
  in progress rather than only between redraws.

The photon map is a Qt window now, so it costs one `QImage` fill per publish
(no ROOT redraw at all): the ROOT thread no longer pays for it, and the map can
be published on its own cadence without slowing the canvases down.  Raise the
ROOT redraw period in `Update Rates...` if the remaining canvases are still too
expensive, and note that over X11 forwarding each canvas repaint is a network
round trip, so the absolute numbers there will be larger.

A continuous zoom drag no longer queues a repaint per mouse move either: the
axis ranges are updated for every event but the expensive `colz` repaint is
skipped while a newer viewport is already queued.

## Saving gate-passed ROI frames (HDF5)

`Save` in the control window writes to one HDF5 file (plus a JSON sidecar)
either the **active gate ROI** of every frame that passes the gate, or the
**virtual photon counting maps**.  A gate ROI is *not* required: with no ROI
selected the full frame is saved (the ROI panel says
`no ROI selected | gate: full frame`).  **The ROI applies to both modes**: a
saved map is the gate-ROI window of the full-frame map, cropped in the same
data coordinates as `/data` (same `x`/`y`/`w`/`h`), so a ROI of 400x280 gives
`/virtual photon counting` the shape `{nmaps, 280, 400}`; with no ROI the maps
are `{nmaps, 512, 1024}`.  That is a write-side crop - the counting itself is
always full-frame.

* `save frames` / `virtual photon counting` - what the run writes.  The mode is
  recorded in the JSON (`save_mode`).
* `Number of frames to be saved` - the run stops by itself once that many
  frames are in the file (`0` = unlimited, stop with the button).  In virtual
  photon counting mode the label changes to *Number of photon maps to be saved*
  and the same number budgets the **maps**.
* `photon map every N frames` (virtual photon counting only, minimum **200**) -
  how many analysed frames go into one saved map.  Independent of the display
  cadence in the *Virtual photon counting* panel: the display can run every 200
  frames while the file takes one map every 2000, or the other way round.  The
  maps are never cumulative in the file, and each one stores the frame number
  and timestamp of the **first gate-passed frame** of its window, so a map can
  be matched to the frames recorded around it.
* **A pedestal is required.**  Photon counting thresholds the
  *pedestal-subtracted* ADC, so virtual photon counting **refuses to start**
  without an active pedestal (`No pede` on, or no pedestal file): the button
  drops back and the status line says
  `virtual photon counting needs a pedestal (...)`, instead of writing a file
  full of zeros.  Load or acquire a pedestal and clear `No pede`, or save
  frames.  (The JSON records `photon_maps.pedestal_active`, `true` for every
  map that does get written.)
* `Directory` / `Base name` - where the files go and what they are called;
  `...` browses for the directory. The file is
  `<dir>/<base>_<YYYYmmdd_HHMMSS>_f0_0.h5` (and the JSON the same with
  `.json`), so re-using a base never overwrites an earlier run, and the
  directory is created if needed.
* `Previous destinations...` - the list of directories and bases used before,
  **most recent first**, with `Use` and `Delete`. A destination is recorded
  (and moved to the top) every time a save actually starts, **except** when it is
  still the start-up default (`--h5-dir`/`--h5-base`, i.e. `./` + `jfroi` with
  no options): the default is not a "previous" destination and would only
  clutter the list on every run. The list is one
  small file in the program folder (`./save_dest_list.dat`), capped at 100
  entries.
* `Pedestal corrected` - sits under the `save frames` radio, because it only
  applies to that mode: the photon maps are always pedestal-subtracted.  It is
  greyed while `virtual photon counting` is selected.  Off by default (raw
  data). With it on, `/data`
  contains pedestal-corrected **float32** values and no pedestal slice, and the
  pedestal image goes to the separate `/pedestal data` dataset instead. It
  needs a loaded pedestal (the run refuses to start without one). Each frame is
  corrected with the pedestal in force when it was captured, so a live-tracked
  pedestal is followed (see "Pedestal tracking" above).
* `Start/Stop save (n)` latches; it also releases when the budget is reached, and
  the status line shows the file, how many frames were saved and how many were
  dropped. Stopping flushes the queue instead of discarding it.
* **Periodic checkpoint** (`checkpointMs`, default 30 s; 0 disables): while a
  save runs, the writer shrinks the over-allocated datasets back to the frames
  actually written, flushes the HDF5, and rewrites the JSON sidecar atomically
  (`.part` -> rename, see below). Every checkpoint boundary therefore leaves a
  self-consistent pair on disk, so the run can be checked without stopping it,
  and a crash loses at most one interval.

The status line spells out what the counters mean:
`N frames written, M dropped (nothing lost)` is the healthy case, and
`... M dropped (writer behind: queue full)` means the writer could not take M
frames/maps because its queue was full (at most **4096 items or 256 MB** are
buffered) - i.e. the disk was slower than the incoming stream. The note only
appears when something was actually dropped; the gate counters keep counting
those frames, they are simply not in the file.

**A running save locks the settings that would change the file.**  While the
writer runs, the save panel (except `Start/Stop save`), the storage-cell filter,
the manual pedestal work, the gain correction and the ROI buttons are disabled -
their JSON sidecar describes the settings at save start, so a quiet change
halfway would make it a lie.  The *display* controls stay live (palettes, ranges,
update rates, `Reset His/Axis`, `keV PH`, the cumulative switch and the map
cadence), and the pedestal tracker keeps following drift as it should.  The
gate's **threshold** and **min pixels** also stay live, but while **`Gate
enabled` is ticked** changing either one asks first (`Changing settings during a
run, are you sure?`); if you decline, the old value is put back, and if you
accept, the change is applied *and* the file's JSON gets
`gate_settings_modified_during_run: true` plus a `gate_settings_note` saying the
`gate` block is the criterion from save start.  With the gate off (passthrough)
those two decide nothing about the saved frames, so they change silently and
nothing is added to the JSON.

**Quitting asks first, and flushes a running save.**  `Quit (q)`, closing the
window, or the shortcut all raise a confirmation; if a ROI save is still
running the dialog says how many frames are written and where, and states that
it will be flushed.  Only *Yes* quits: the writer drains its queue, closes the
HDF5 file (extending it to exactly the frames written), writes the JSON sidecar
with `stop_reason = "program exit"`, and only then do the threads stop.
**Every window goes with it**: the ADC, gain and photon-map windows are
separate top-levels and the ROOT canvases live in the ROOT thread, so quitting
closes them explicitly - closing only the control window used to leave them (and
the process) behind.  `No`
(or `Escape`) leaves everything running untouched.  `Ctrl-C`/`SIGTERM` cannot
ask (a signal is not a dialog), so they flush the save and exit; a second
`Ctrl-C` quits immediately.

Each mode ends by itself when its budget is reached (`stop_reason`
`frame budget reached` / `photon-map budget reached`), leaving a closed,
`complete: true` file and JSON behind - the useful way to run a save
unattended.  `--save-budget N` sets the budget without the GUI.

Saves can be run back to back: a save that ends by itself (frame budget
reached, `Start/Stop save` pressed again, or an I/O error) leaves a finished writer
thread behind, and the next start joins it before spawning a new one.  (A
`std::thread` that is still joinable may not be overwritten - doing so calls
`std::terminate`, which is what aborted the program the second time a file was
started.)  The acq, analysis, worker and ROOT threads are also wrapped so that
an exception is reported and stops that stage instead of calling
`std::terminate`; the GUI keeps running and shows the message in "Last error".

### File format (slsReceiver-compatible)

The layout is the one written by `slsReceiverSoftware/src/HDF5DataFile.cpp`, so
existing receiver readers work unchanged. The file is **v110 format** (SWMR, see
"Crash recovery and mid-run inspection"), so readers need HDF5 >= 1.10:

```
<dir>/jfroi_<YYYYmmdd_HHMMSS>_f0_0.h5
  /data                                  rank 3, U16LE, {nframes, h, w},
                                         unlimited, chunk {1, h, w}
  /frame number                          U64, one value per frame
  /exp length or sub exposure time       U32
  /packets caught                        U32
  /detector specific 1                   U64
  /timestamp                             U64
  /mod id, /row, /column                 U16
  /detector specific 2                   U16
  /detector specific 3                   U32
  /detector specific 4                   U16
  /detector type, /detector header version   U8
  /packets caught bit mask               fixed size string (zero here)
  /pedestal data                         float32, corrected mode only
  /pedestal tracking                     float32, {n, h, w} (empty unless tracked)
  /pedestal tracking frame               U64, one value per tracked image
  /virtual photon counting                float32, {nmaps, h, w}, photon mode only
  /virtual photon counting frame         U64, first frame number of each map
  /virtual photon counting timestamp     U64, its header timestamp
  attribute "version" = 7.0              (as the receiver writes it)
  attribute "roi_x/roi_y/roi_w/roi_h", "pedestal_in_frame_0", "gate"
```

The parameter datasets are filled from the streamed JSON header (frame number,
exp length, packets caught, detSpec1..4, timestamp, mod id, row, column,
detector type/version), so a reader keys on `/frame number` as usual. The
packets-caught bit mask is not part of the ZMQ header and is written zero.

Deliberate differences from a receiver file:

* `/data` is the **ROI crop**, not the whole module;
* the file is **v110 format** and written in SWMR mode, so it can be read while
  it is being written and stays valid if the writer crashes (readers need
  HDF5 >= 1.10);
* `/pedestal tracking` and `/pedestal tracking frame` are always present (empty
  when tracking is off) because SWMR cannot create objects after it starts;
* in **virtual photon counting** mode the frames are not written at all: `/data`
  keeps only its slice 0 (the pedestal) and the maps go to
  `/virtual photon counting`, with the provenance of each map in
  `/virtual photon counting frame` (first frame number of its window) and
  `/virtual photon counting timestamp`.  The maps are photon counts per pixel,
  float32, never cumulative, and cropped to the gate ROI exactly like `/data`;
* `Pedestal corrected` mode changes `/data` to float32 and drops the slice-0
  pedestal, so the layout then is `/data` (n frames), `/pedestal data`
  (one image, float32) and the same parameter datasets;
* only **gate-passed** frames are written - each keeps its **original frame
  number**, so the gaps in `/frame number` are exactly the frames the gate
  rejected;
* in raw mode **slice 0 is the gain-0 pedestal in the same ROI** (frame number
  0, flagged by the `pedestal_in_frame_0` attribute), so a reader can subtract
  it directly; with no pedestal loaded the slice is zero;
* in corrected mode the correction is the same one the display and the gate
  use (g0: `raw - pede0`, g1: `(pede1 + G1off - raw) * 30`, g3:
  `(pede2 + G2off - raw) * 340`), written as float32, so negatives and the
  gain-normalised scale survive. The pedestal applied is the one in force when
  the frame was captured, so pedestal tracking is followed slice by slice; use
  `/pedestal tracking` (with `/pedestal tracking frame`) to know which pedestal
  each slice saw. `/pedestal data` is the pedestal at save start;
* `/data` slices are the raw 16-bit words as received (ADC in bits 0-13, gain
  in bits 14-15), exactly like the receiver; slice 0 is in ADC units;
* extra self-describing root attributes: `roi_x/roi_y/roi_w/roi_h`,
  `pedestal_in_frame_0`, `gate`.

### JSON sidecar

Same base name with `.json`: ROI, the gate criteria in force, the data
encoding, start time and duration, stop reason, frames saved / dropped /
budget / first and last frame number, the full gate statistics and rates, the
alignment counters and the per-interface state.

The save mode is spelled out: `save_mode` (`frames` or
`virtual photon counting`), `photon_map_every`, `photon_threshold` and
`photon_threshold_unit` (`ADU` or `keV`), and - only when it happened -
`gate_settings_modified_during_run` with `gate_settings_note`, and in photon mode a `photon_maps`
block with `saved`, `every`, `threshold`, `threshold_unit`, `cumulative:
false`, `pedestal_active` and the dataset name.  In that mode
`frames.saved` counts the **maps** (the frames budget is unused), so a
reader does not have to know which counter applies.

The sidecar is rewritten at every checkpoint, not only at the end. Each rewrite
fills `<base>..._f0_0.json.part`, fsyncs it and renames it over the `.json`, so
a reader always sees a complete document - the previous one until the rename -
and a crash mid-write leaves only a stale `.part` behind. While the save runs
the document carries `"complete": false`; the final one, written when the writer
stops, carries `"complete": true`. The statistics are refreshed by the writer
thread just before each write, so the final document holds the true end-of-run
values.

### Crash recovery and mid-run inspection

The HDF5 is written in **SWMR** mode (Single Writer / Multiple Reader). The file
is created with `libver = latest` (v110 format), all objects are created up
front, and `H5Fstart_swmr_write` is called before the first frame. Every
checkpoint then shrinks the datasets to the frames actually written, flushes,
and rewrites the JSON atomically.

What that buys:

* **Open it mid-run.** A reader can open the live file with the SWMR read flag
  while the save continues, e.g. `h5py.File(path, "r", libver="latest",
  swmr=True)`. HDF5 guarantees a valid, consistent view at every flush; without
  SWMR a second open would be refused by the file lock.
* **Crash recovery.** SWMR guarantees the file stays valid even if the writer
  dies without closing it, so the last flushed checkpoint is readable. The JSON
  sidecar carries the matching `frames.saved` / `last_frame_number`.
* **The JSON is never torn** - it is published with a single `.part` -> rename,
  so a reader sees the previous complete document until the new one is ready.

Trade-offs:

* v110 files need HDF5 >= 1.10 to read (finished or in progress). Older analysis
  installations cannot open them.
* Because SWMR forbids creating objects after it starts, `/pedestal tracking`
  and `/pedestal tracking frame` are now created (empty) for every save; the
  series is still only filled while tracking is on.
* Tools that do not know about SWMR should read the file after the save has
  closed (then it is an ordinary v110 file), or take a copy at a checkpoint
  boundary.

### Throughput

The acq thread only copies the ROI into a queue (up to 256 MB); a separate
writer thread does the HDF5 I/O. If the gate passes frames faster than the disk
absorbs them, the queue fills and the excess is counted as
`dropped_queue_full` in the JSON and shown in the GUI - the gate counters stay
exact, but those frames are not in the file.

`--h5-dir DIR` and `--h5-base NAME` set the initial directory and base name
(defaults: the program folder and `jfroi`).

## Behaviour differences worth knowing

1. **One pedestal store.** The gate and the display share one immutable
   `PedeSet` (shared_ptr swap). File loads and live acquisition publish a new
   set, so the old `stop() → loadPedestal() → start()` contract is gone and
   loading pedestals never interrupts acquisition.
2. **Gate offsets unified.** The old filter had its own `G1offset/G2offset`
   (always 0); the gate now uses the display's live `G1Poffset/G2Poffset`.
3. **Gate threshold vs photon threshold** are separate values in one process
   (they only collided as one global named `threshold` because they lived in
   two programs). The gate one is an argument or the gate panel and is always
   ADU.  The photon-map one lives in the *Virtual photon counting* panel and
   follows the gain stage: **ADU** while the gain correction is off (default
   150) and **keV** while it is on (default 4.0) - one control, two remembered
   values, so the keV spectra and the photon counting can never disagree about
   the calibration.
4. **`Acquire Pede` follows the storage-cell filter.**  Pedestals depend on the
   storage cell, so only frames the SC filter accepts are averaged: with
   `--sc 7` (or the drop-down set to 7) a 25-frame acquisition takes frames from
   cell 7 only - measured: it completed after 365 incoming frames instead of 25
   - and the log says which cell it used
   (`[pede] installed pedestals averaged over 25 frames (storage cell 7)`).
   With the filter on `ALL` every cell contributes and the log says
   `storage cell ALL - every cell contributed`, which mixes their pedestals: set
   the filter when the pedestal is cell-dependent.  The *signal* gate is
   deliberately not applied here - the frames it drops are exactly the empty
   ones a pedestal wants.
5. **Live pedestal acquisition is asynchronous** (`p` / `--acquire-pede`); it
   averages the next N frames on the acq thread instead of blocking the GUI
   and restarting threads. It fills gain 0 only, so `hasAll` stays false — the
   same fallback behaviour as the old `loadpede()`.
5. **Keyboard shortcuts actually work, and upper case needs Shift.** `s`/`Shift+S`,
`b`/`Shift+B`, `r`/`Shift+R`, `o`/`Shift+O` and `p`/`Shift+P` are distinct
(`QKeySequence("s") == QKeySequence("S")` made the plain upper-case forms
ambiguous, so Qt activated neither). The original only *documented* `s`,
   `p`, `o`/`O`, `r`/`R`, `+`/`-`, `q`, `u`, `C`, `[`/`]`, `{`/`}`, `g`, `b`,
   `B` in the button labels; they are now real `QShortcut`s.
6. **"Load All Pede (P)" is functional** (file chooser); the original printed
   *not implemented*.
7. Leaks fixed: the ROI `TBox` is now a single reused box, and the live
   pedestal decoder object is freed.
8. `phene == 0` can no longer divide by zero in the photon-counting increment.
9. **The stream state is real.** `jf_filter` treated every 200 ms receive
   timeout as "Receiver stopped, waiting for new stream", which fired whenever
   the receiver was merely idle. The merged program monitors the ZMQ socket
   (`zmq_socket_monitor`) and derives the state from actual connect /
   disconnect / handshake events:

   | state | GUI text | `--stats` token |
   |---|---|---|
   | never connected yet | `waiting for receiver...` | `connecting` |
   | peer up, frames arriving | `connected` | `connected` |
   | peer up, no frame for > 2 s | `connected (no frames for N s)` | `idle` |
   | end-of-acquisition marker seen | `connected (end of acquisition)` | `eoa` |
   | peer gone | `disconnected` | `disconnected` |

   stdout lines are emitted only on real transitions:
   `Receiver connected` / `Receiver connected but no frames for N s (still alive)`
   / `Receiver sent end-of-acquisition, waiting for new stream` /
   `Receiver disconnected, waiting to reconnect...`.
   If `zmq_socket_monitor` is unavailable the code falls back to frame-arrival
   staleness (3 s) instead of failing.

## Quirks deliberately preserved

* `factor = 1.84` scaling of the reference rows/columns in the ADC image.
* `fgaind[]` is still never initialised to anything but 0, so the gain
  correction path multiplies gain-0 pixels by zero (pre-existing behaviour,
  kept as-is).
* `hchptr[]` is still accumulated and never displayed.
* The gate's no-pedestal path still compares raw 14-bit ADC values, while the
  display's photon counting and the saved photon maps need a pedestal (`g0`) and
  cannot be saved at all in the default `--no-pede` mode: the photon-map save
  is refused with an explanation, while the frame save (and the gate's own raw
  thresholding) keep working.
* `--no-gate` performs no pixel counting, so `pixelsAbove` stays 0.

## Testing without hardware

`test/jf_feeder.cpp` stands in for `slsReceiver`: it publishes the same
`<json header><binary frame>` pairs. Every 3rd frame carries no signal; the
others carry 40 pixels at ADC 300 over a baseline of 100.

```sh
make feeder
./test/jf_feeder tcp://*:30777 200 20 &
./jf_online_display_gui 400 127.0.0.1 30777 --gate --stats --pede /nonexistent.dat
# gate threshold 200  ->  expect pass = 2/3, drop = 1/3, px = pass*40

# virtual photon counting save, headless: a zero-filled pedestal makes the
# 40-pixel cluster count (with the real ~2000 ADU pedestal the maps are empty)
/usr/bin/python3.9 -c "import numpy as np; np.zeros(6*1024*512,'<f4').tofile('/tmp/zero_pede.dat')"
./test/jf_feeder tcp://*:30777 1500 25 &
./jf_online_display_gui 127.0.0.1 30777 --no-gate --start-save --save-photon \
    --pmap-save-every 200 --save-budget 2 --use-pede --pede /tmp/zero_pede.dat \
    --h5-dir /tmp/pmap --h5-base pmap --stats
# (`--use-pede` + a pedestal is required: without one the run refuses the
#  photon-map save and writes no file at all)
# expected: "[save] --start-save: starting the virtual photon counting writer",
# then "map 1 written (200 frames, first frame 0)", "map 2 written ... frame 201",
# and the file closes by itself (stop_reason "photon-map budget reached"):
#   /virtual photon counting        (2, 512, 1024) float32, 40 non-zero pixels,
#                                   sum ~5320 per map (40 px x 133 counts)
#   /virtual photon counting frame  [0, 201]
#   /data                           (1, 512, 1024) - the pedestal slice only

# optional republish check (passthrough so frames keep flowing):
./test/jf_feeder sub tcp://127.0.0.1:30778 10 &
./test/jf_feeder tcp://*:30777 200 25 &
./jf_online_display_gui 400 127.0.0.1 30777 --no-gate --pub 30778 --pede /nonexistent.dat

# idle-but-alive check: 4th feeder arg = silent period in seconds
./jf_online_display_gui 400 127.0.0.1 30777 --stats --pede /nonexistent.dat
./test/jf_feeder tcp://*:30777 50 25 6   # connects, is silent for 6 s, then sends
# expected: "Receiver connected" -> "connected but no frames for 2.0 s
# (still alive)" -> frames -> "end-of-acquisition" -> "disconnected"

# multi-interface check: two 1024x256 slabs on 30777 (bot, value 100) and
# 30778 (top, value 150); gate threshold 120 -> only the top slab counts
./test/jf_feeder dual 30777 200 10 4 100 150 0 &
./jf_online_display_gui 400 127.0.0.1 30777+30778 120 5 --gate --stats --pede /nonexistent.dat
# expected: px = in * 262144, mispaired=0, drop=0

# storage-cell gate: the feeder cycles SC = frame % 16 in detSpec3, so a
# selected cell must pass exactly 1/16 of the frames and nothing else
./test/jf_feeder dual 30777 1600 400 1 100 150 0 &
./jf_online_display_gui 1000 127.0.0.1 30777+30778 120 5 --gate --stats \
    --pede /nonexistent.dat --sc 3
# expected: pass ~= in/16, scdrop ~= 15*in/16, pass + scdrop = in, sc=3
# (--sc all passes everything; with --no-gate the cell gate still filters)

# gate-ROI convention + ROI spectra, same feeder (bottom slab dim, top bright):
#   a ROI on the TOP half must be the one that passes, and only hchip 4..7 of
#   the ROI spectra may be filled; the BOTTOM half must drop everything
JF_TIMING=1 ./jf_online_display_gui 400 127.0.0.1 30777+30778 120 5 --gate --stats \
    --pede /nonexistent.dat --gate-roi 0 256 1024 256
# expected: px = in * 262144, hroi/hchip = 0.5214 (chips 4..7 of A4)
JF_TIMING=1 ./jf_online_display_gui 400 127.0.0.1 30777+30778 120 5 --gate --stats \
    --pede /nonexistent.dat --gate-roi 0 0 1024 256
# expected: pass = 0, px = 0 (the dim half is at the bottom)
JF_TIMING=1 ./jf_online_display_gui 400 127.0.0.1 30777+30778 120 5 --gate --stats \
    --pede /nonexistent.dat --gate-roi 0 0 1024 512
# expected: hroi == the sum of hchip (a full-frame ROI selects every pixel)
```

A zero-filled 12 MB file works as a pedestal for these runs (the photon map
and the photon counting need `g0` to be present, so `--pede /nonexistent.dat`
leaves the photon map empty by design).

Verified end-to-end under `xvfb-run`: gate counts/rates, pixels-above, display
hand-off with zero drops, ROOT redraws, live pedestal acquisition, 12 MB
pedestal file load, optional republish to an external subscriber,
end-of-acquisition handling, connect/idle/disconnect/reconnect reporting,
graceful `SIGINT`/`SIGTERM` shutdown, and for the multi-interface mode: slab
geometry detection, bottom→top placement (and the reversed order), pairing by
frame number, straggler timeout, mispair detection/recovery, a permanently
missing interface running at full rate without freezing, dual-mode republish,
and the live/loaded pedestal paths on the assembled frame.  The ROI work was
verified against synthetic frames by counting the pixels per chip (a top-half
ROI gives `hroi/chip = 0.5214`, a bottom-half ROI passes the gate only when the
bottom half is the bright one, a full-frame ROI gives `hroi == sum(hchip)`, and
`hroi/hchip` matches rows x columns exactly), the range defaults were read back
from the live axes (`PH x -212..3010`, `main z 0..256`), the Set Ranges dialog
was driven through XTest (it opens on the live values `ADC -200..1000` /
`P.Map 0..256` / `PH -213..3010`; typing `ADC max = 300` turned the dim half of
the ADC image from blue `(0,150,221)` into yellow-green `(214,253,39)` while
the ROOT map kept its own scale, and `P.Map max = 251` left the ROOT map at
`0..251` across several redraws), and the geometry was checked on
screen under Xvfb: with a 400x1000 ADC window the captured ROI came out
78x224 (narrow and tall), the yellow box was measured at 162x463 px against an
ideal 2.872 aspect (0.5% off), and the bright half of the frame is at the
bottom in the ADC image and in the ROOT photon map alike.

`test/pede_live_test.cpp` is a standalone check (no hardware, no GUI) with two
claims: a `Pedestal corrected` save follows the live pedestal instead of the
snapshot taken at Start Save (it offers one frame, installs a different
`PedeSet` mid-run, offers a second frame, and expects the two `/data` slices to
carry the two different pedestals), and the periodic checkpoint leaves a clean
pair on disk while the save still runs (the JSON has `"complete": false`, no
`.part` is left, and the **live** HDF5 read back with the SWMR flag has exactly
the frames written, not the over-allocated block).  Build and run it against the
already-built objects:

```
g++ -std=c++17 -O2 -pthread $(root-config --cflags) \
    $(pkg-config --cflags Qt5Core Qt5Gui Qt5Widgets) \
    -I. -I../../dataStructures -I../../interpolations -I../../tiffio/include \
    -I../../../slsSupportLib/include -I../../../slsReceiverSoftware/include \
    -I../../../libs/rapidjson -I../../../build/_deps/libzmq-src/include \
    test/pede_live_test.cpp online_jf_h5.o online_jf_core.o online_jf_root.o \
    -o test/pede_live_test \
    $(pkg-config --libs Qt5Core Qt5Gui Qt5Widgets) $(root-config --libs) \
    ../../../build/_deps/libzmq-build/lib/libzmq.a -lhdf5_cpp -lhdf5 -lsodium \
    -lpthread -lrt -lm
cd test && LD_LIBRARY_PATH=/usr/lib64/root ./pede_live_test
```

`test/panel_map_test.cpp` is a JFSTRX-only check that the six-panel fill
geometry matches the strixel decoder: every booked panel (79x165, 47x320,
59x476 twice) must be fully covered by the decoder map, and only the decoder's
few extra right-edge columns may fall outside. Build it with `-DJFSTRX` against
the JFSTRX objects (same command as above plus `-DJFSTRX`).

## Files

| file | contents |
|---|---|
| `online_jf.h` | shared state, `Config`, `PedeSet`/`PedestalStore`, `GateEngine`, mailboxes, `Pipeline`, ROOT declarations |
| `online_jf_core.cpp` | globals, pedestal file I/O, gate math, acq + analysis threads |
| `online_jf_root.cpp` | ROOT objects, command queue, ROOT redraw thread, plotting |
| `online_jf_gui.cpp` | `FastImageView`, merged `MainWindow`, CLI parsing, `main()` |
| `online_jf_h5.cpp` | decimated ROI HDF5 writer (receiver-compatible) + JSON sidecar |
| `test/jf_feeder.cpp` | synthetic slsReceiver / republish subscriber |
| `test/pede_live_test.cpp` | standalone check: live pedestal + periodic clean checkpoint |
| `test/panel_map_test.cpp` | JFSTRX-only check of the six-panel fill geometry |
