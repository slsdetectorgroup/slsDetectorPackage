# Notes

Working notes for `jf_online_display_gui`.  The README is the reference for how the
program behaves; this file records provenance, verification and things that are
easy to trip over.

## 2026-09-23 — handoff from another session (verified here)

An all-night run showed the **display followed the pedestal drift but the saved
HDF5 did not**.  Three changes were made by that session and are on disk; I
re-verified them in this workspace on 2026-09-23.

### What changed

| file | change |
|---|---|
| `online_jf_h5.cpp` | `RoiSaver::offer()` corrects each frame with the **live** pedestal (`PedestalStore::instance().get()`), not the snapshot taken by the GUI at Start Save.  Periodic checkpoint: shrink `/data` + the 14 parameter datasets to exactly `framesWritten` (undoing the `kExtendBlock = 128` over-allocation), `H5Fflush(SCOPE_GLOBAL)`, rewrite the JSON.  JSON writer factored into `writeJsonSidecar(bool complete)` writing `<name>.json.part` → `fsync` → `rename` → directory `fsync` (`syncDirOf`).  File created as **v110 (SWMR)**: `H5Pset_libver_bounds(LATEST)` + `H5Fcreate`, all objects created up front, then `H5Fstart_swmr_write()` before the first frame. |
| `online_jf.h` | `StartInfo::checkpointMs = 30000` (0 disables) |
| `README.md` | pedestal tracking / saving / file format / JSON / crash-and-mid-run sections, test description, files table |
| `test/pede_live_test.cpp` | standalone check: live pedestal + checkpoint + SWMR live read |

Consequences of SWMR worth remembering: objects cannot be created after it
starts, so `/pedestal tracking` and `/pedestal tracking frame` are now created
(empty) for **every** save, and the baseline pedestal image is written just after
the SWMR switch.  (The 10-minute cadence of that series, and its ROI-shaped
images, are unchanged.)

### Verification done here

* `make` — clean, binary current with the sources.
* `test/pede_live_test` (built with the README's line, plus `$(root-config --cflags)`
  which that line omits) — **PASS**:

```
checkpoint1: periodic json (complete:false)=true  live swmr read=true  h5 extent=1  px=900.0
checkpoint2: live swmr read=true  h5 extent=2  slice1 px=850.0
final: complete=true  no .part left=true
PASS: live pedestal + periodic clean checkpoint
```

The two different pixel values are the point: the first frame was corrected with
pedestal A, the second with pedestal B installed mid-run, so the file follows the
tracked pedestal.  Generated artefacts were deleted afterwards.

### Operational caveats

* **v110 needs HDF5 ≥ 1.10.**  Older analysis installs cannot open these files at
  all — this is the one to check before an overnight run.
* **Reading mid-run needs the SWMR read flag**: `h5py.File(path, "r",
  libver="latest", swmr=True)` and `dset.refresh()` to see new frames.
  `h5dump`/`h5ls` on the live file will not work.  A closed file reads normally.
* `Pedestal corrected` is still **off by default** (raw data).  Pedestal tracking
  only reaches `/data` when it is ticked; in raw mode the only pedestal in the
  file is slice 0.
* `/pedestal data` is the pedestal at save start; `/pedestal tracking`
  (+ `/pedestal tracking frame`) is the history of what was actually used, one
  image per checkpoint interval.
* `checkpointMs` defaults to 30 s and is **not exposed** in the GUI or on the
  command line yet.

### Open / optional

* `--no-swmr` (or a `StartInfo` fallback) if a downstream reader turns out to be
  on HDF5 < 1.10.
* Expose the checkpoint interval (`--checkpoint MS` and/or a GUI field).
* If an external reader does not see new frames after a checkpoint, add explicit
  per-dataset `H5Dflush`; `H5Fflush(SCOPE_GLOBAL)` was enough in the test.
* Unrelated: the older `onlinedisp_zmq.cpp` extracts the ROI from
  `zHeader.detSpec1`, which is a hack (that field is the bunch id) — the stream
  header already carries `zHeader.rx_roi` (`std::array<int,4>`, filled from the
  receiver's per-port ROI).  That file was not touched.

## Workspace health

* `../_AGENT.txt` was rewritten (2026-09-23, on request): it now points at
  `jf_online_display_gui/` as the active work, with the old `onlinedisp_zmq` +
  `jf_event_finder_*` set kept in a clearly marked "legacy / superseded"
  section.  The design rules that both generations share (ROOT-thread rule,
  photon map, display cadence, gate semantics, SWMR files) are listed there too.
* `test/_run/` was removed (2026-09-23, on request).  For the record, what was
  in it was not worth keeping: the `..._084120_...` run had only its sidecar left
  (the HDF5 was already gone) and `..._090709_...` had `frames.saved = 0` — an
  empty save written before the handoff, with no `complete` field.  `test/` now
  holds only `jf_feeder[.cpp]` and `pede_live_test.cpp`.
* `jf_online_display_gui/` and the h5/write path are untracked in git; nothing here is
  committed.
* `.gitignore` in this directory excludes `*.o`, the `jf_online_display_gui` and
  `test/jf_feeder` binaries, and runtime data (`current_pede.dat`,
  `roi_list.dat`, `save_dest_list.dat`, `jfroi_*.h5/json`).  Note it does *not*
  cover `test/pede_live_test` (the test binary), so that one should be deleted
  after use.

## 2026-09-30 — JFSTRX display + startup defaults

* `-DJFSTRX`: the **ADC and gain windows are now unmapped** (raw 1024x512), so
  gate-ROI selection and the hover read-out stay in raw coordinates.  The mapped
  view is a separate, display-only window, **"ADC Strixel Panels (JFSTRX)"**,
  with the six groups arranged like the ROOT canvas (left chip1 g3/g2/g1, right
  chip6 g1/g2/g3).
* The six panels use the strixel **pixel aspect** `X = 75*(75/Y)`, `Y =
  25/15/18.75 um` -> 9:1, 25:1, 16:1, via `FastImageView::setPixelAspectX`
  (non-uniform x/y scaling; ADC and gain stay square).
* The receiver ROI is parsed from the JSON `rx_roi` array (the old `detSpec1`
  hack is gone) and drawn as the **red** rectangle on the raw ADC image.
* Raw geometry is now `kRawWidth`/`kRawHeight` (1024x512); `nx`/`ny` under
  JFSTRX are the strixel *display* geometry only.  Fixed the assembled-frame
  height, the multi-interface slab split, the gate-ROI defaults and the HDF5
  pedestal extraction, which all wrongly used the mapped `nx`/`ny`.
* **Startup defaults changed**: the signal gate is **off** (`--gate` enables it)
  and pedestal subtraction is **off** (`--use-pede` enables it; the "No pede"
  button starts checked).  README examples now pass `--gate` where gating is
  expected.
* `test/panel_map_test.cpp` (JFSTRX objects) checks the six-panel fill geometry
  against the decoder: all six panels fully covered, only the decoder's 183
  extra right-edge columns dropped (same as the ROOT histograms).
* The strixel ROI-outline experiment (`buildReceiverRoiOutline`) was removed -
  ROI on the mapped panels was judged too complex.
* At ~14:40 the on-disk `jf_online_display_gui` was found truncated to 0 bytes
  (concurrent build in this directory); `make jfstrx` rebuilt it (14:41).

## 2026-09-30 (later) — three reported bugs

1. **Zoom-out stopped early** (`FastImageView::wheelEvent`): it called
   `matchViewportToWindow()` on every wheel step, so the image dimension that
   reached its limit first froze the other and the whole frame could never be
   shown.  Outward zoom now only `clampViewport()`s; the window-shape matching
   still runs while zooming in and on resize.
2. **ROI PH (A7) empty**: the fill condition used `gate_.roiActive()`, which is
   only true while "ROI only" is on, so a merely *selected* ROI left A7 empty.
   The ROI snapshot/spectra now use `gate_.roiSet()`; "ROI only" still controls
   the gate restriction (unrelated to showing the ROI).
3. **"Reset His" took 30-50 s**: the ROOT thread executed the reset and blocked
   on `gHistoMtx`, which the analysis thread holds around every frame - under
   the heavy JFSTRX fill it can starve.  The reset is now performed by the
   analysis thread at the top of its next frame (lock already held), with a
   non-blocking `try_lock` fallback on the ROOT thread for the idle case.  The
   ROOT thread also now wakes immediately (`gCmdCv`) for commands and forced
   redraws instead of waiting for the `ROOT update (ms)` tick, so a large
   update interval no longer delays a button.

**Build footgun:** the Makefile does not track flags.  After `make jfstrx`,
a plain `make` only rebuilds changed files and then links a mix of JFSTRX and
non-JFSTRX objects (I hit a link error that way).  Always `make clean` before
switching between `make` and `make jfstrx`.

## HDF5 C++ headers (build dependency)

`#include "H5Cpp.h"` needs the HDF5 **C++ dev** package, not just the runtime:

| distro | package | where the header lands |
|---|---|---|
| RHEL / Rocky / Alma / Fedora | `hdf5-devel` | `/usr/include/H5Cpp.h` |
| Debian / Ubuntu | `libhdf5-dev` | `/usr/include/hdf5/serial/H5Cpp.h` |
| openSUSE / SLES | `hdf5-devel` | default include dir |
| Arch | `hdf5` | `/usr/include` |
| conda-forge | `hdf5` | `$CONDA_PREFIX/include` |
| macOS (brew) | `hdf5` | `$(brew --prefix hdf5)/include` |

The Makefile now discovers the include/library dirs via `pkg-config` (`hdf5` or
`hdf5-serial`) and `h5c++ -show`, and both can be overridden, e.g. on Ubuntu:

```
make HDF5_INC=-I/usr/include/hdf5/serial \
     HDF5_LIB=-L/usr/lib/x86_64-linux-gnu/hdf5/serial
```

## 2026-10-06 — external review (`../../../REVIEW_CLAUDE.md`): triage and what was applied

A 39-item review by Claude covers this program plus the Qt6 port, the legacy
tools, `test_bench/` and repo hygiene.  **Part of it was written against a
different checkout**: items 2, 3, 4 (mostly) and 9 are already fixed in *this*
tree, and the review's own item 37 mentions `build-qt6/` cache paths under
`~/swtests/slsDetectorPackage/...`, which does not exist here.  Everything below
was checked against the code in this workspace before touching it.

### Applied and verified here

| item | what was wrong | fix |
|---|---|---|
| 4 (rest) | a header with **no data part** would make the data `recv` swallow the next message's header as pixels | require `ZMQ_RCVMORE` after a valid header, else count an error and drop the header (the header-length clamp and the drain-on-reject paths were already present) |
| 8 | the save recorded the **startup** gate settings, so `gate` in the HDF5 attribute and JSON sidecar was wrong after any live edit | read `enabled/threshold/minPixels/contiguous` from `GateEngine` (new getters) instead of `g_cfg`; verified end to end - a save started from the command line records the live values (`threshold 137`, `min_pixels 9`) |
| 11 (part) | the acq thread **busy-spun at 100 % CPU** waiting for a lagging partner: a held slab kept its socket polled and level-triggered `zmq_poll` fired forever | clear `events` for a socket whose slab is held, so it is not polled until it is released |
| 18 | ten shortcuts never fired: `QKeySequence("s") == QKeySequence("S")`, so s/S, b/B, r/R, o/O, p/P were ambiguous and Qt activated neither | upper-case actions are now `Shift+S/B/R/O/P` (verified: `Shift+S` opens Set Ranges, 352 → 366 windows) |
| 19 | the run examples still passed the removed `<energy>` argument, so `2000` was parsed as the host | README examples, the header comment and the Makefile comment corrected (the two remaining `<energy>` mentions are the *legacy* chain and a historical note, both correct) |
| 22 (part) | `--pede-tracking` left the button unchecked, and `--gate-roi` left `ROI only` unchecked so the first press toggled the wrong way | the tracking button is initialised from `gPedeTracking`, and a restricting `--gate-roi` now checks `ROI only` (through the `m_applyingRoi` guard, so nothing re-captures or warns) |

### Found while fixing item 22 — a bug of my own

The tracking button's enable rule lived in the 500 ms `updateFlow()` poll, which
also *unchecked* the button when the gate was off.  During start-up that cleared
a `--pede-tracking` button (`ptrack=off`, reproduced headless).  The force-off is
now event-driven (in the gate checkbox's own handler); the poll only enables.
After the change: `--pede-tracking` gives `ptrack=on`.

`test/jf_feeder` had also gone missing from the tree (the Makefile has no target
for the test programs — review item 34); rebuilt by hand for the verification.

### Already fixed in this tree (nothing to do)

| item | state |
|---|---|
| 2 (HDF5 never closed) | the file is built with the C++ constructor `H5File(h5Path, …)` so it owns the only reference; the code comments say so explicitly |
| 3 (budget/stop not enforced) | `offer()` returns early on `stopRequested`, and the writer stops at `saved >= maxFrames` |
| 4 (clamps) | `hn > hdr.size()` clamp, `truncated` data check and the drain-on-reject paths are all present |
| 9 (cumulative drop rate) | uses the 1 Hz window (`pedeTrackSignalDropRate(passCur, dropCur, …)`) — fixed earlier the same day |

### Checked and found *correct* (not defects)

While verifying item 8 I chased two values in a saved file that looked wrong and
are not: the file recorded `enabled: false` and `no_pede: true` for a run whose
command line had neither `--gate` nor `--use-pede`.  Both are the documented
defaults - `online_jf.h:189` (`gateEnabled = false`) with `--gate` to enable, and
`gNoPede{true}` with `--use-pede` to clear.  The review's item 22 is the flip
side of this (the widgets were not synced from the config); the config itself is
intentional.  Since it is easy to lose an hour to this, both switches are now
spelled out in the README's options section, and a regression run with
`--gate --use-pede` behaves as expected (with a 100 ADU pedestal and a 250 ADU
threshold on 3000/200 ADU frames: `pass=0 drop=148`, i.e. the ROI half is
reduced to 100 and correctly rejected).

### Deliberately deferred, with reasons

* **13 — chip/ROI spectra saturate (top physics item).**  `TH1I` bin contents
  saturate at `INT_MAX` and the `uint32_t` accumulators wrap: ~55 k pixels per
  chip per frame land in 1–2 noise bins, so at 500–1300 fps the spectra are
  wrong within an hour, and an all-night run at 25 fps is close to the limit too.
  The fix is mechanical but wide (37 `TH1I` sites across four files plus the
  accumulator and sync signatures) and touches the core data path, so it gets
  its own careful pass with a test rather than a rushed edit at the end of a
  long session.  Plan: accumulators → `uint64_t`, histograms/clones → `TH1D`,
  then check that entries, means and the ADU/keV ratio (40.0000) are unchanged.
* **10** (the tracker can overwrite a just-loaded pedestal — re-prime from the
  store under the lock, skip a stale install), **12** (per-port `rx_roi` with the
  `rowBase` offset), **20** (closing the window should quit — the parentless ADC
  /Gain windows keep the process alive), **21** (Delete in the ROI/save-destination
  dialogs uses stale indices), **23–30** (races on `g_pheneThreshold`/the ROI,
  `renderImage`'s 524 k `setPixel` calls and the 500 ms `roi_list.dat` re-read,
  NaN pedestals, rapidjson asserts, the finish/start save race, frame 0, and the
  small GUI defects).
* **11 (the rest)** — partial-frame assembly is a feature, not a fix; the README
  now documents the real behaviour and calls the promise an open item.
* **Everything outside `jf_online_display_gui`** — the Qt6 port (1, 16, 17, 35, 36), the
  legacy tools (5, 6, 7, 14, 15, 31 — the project's own `_AGENT.txt` marks them
  superseded), `test_bench` (32), repo hygiene (33, 37, 38 — `git checkout --`
  for the symlinks needs repository write access, which is still deferred) and
  maintainability (39).  These need decisions of their own; none of them were
  changed here.

## 2026-10-14 — virtual photon counting: display side done, save side specified

Requested: take the photon map out of ROOT, give it its own Qt window filled the
same way but **not cumulative by default** and plotted every N images (default
2000), put those controls plus the photon threshold in a "Virtual photon
counting" panel left of the Display, and add a save mode that writes the
non-cumulative maps every M images (M >= 200) with the first frame number and
timestamp, M and the thresholds recorded.

### Done (build green, window present, no crashes)

* Core: `gPmapEvery` (default 2000) and `gPmapCumulative` (default **false**)
  atomics; the photon map is published into the image mailbox every
  `gPmapEvery` **analysed frames** (independent of the ADC image decimation),
  and when not cumulative the accumulator is zeroed right after the publish, so
  each image holds the photons counted since the previous one.  The counting
  itself is unchanged.
* GUI: a **"Virtual photon counting"** panel (left column, under the gates -
  the Display panel is in the right column at that height) with the
  `cumulative` checkbox, `plot every N images` (1..1000000, default 2000) and
  the **photon threshold (ADU)** spin moved out of the Display group.
* GUI: a new top-level window **"Photon Map (virtual photon counting)"**
  (1104x552, same colour palette as the ADC image, colour range 0..8 photons
  per pixel), refreshed from the mailbox in `processImages()`.
* Fixed while doing it: the panel was built before `photonThrSpin_` existed, so
  the grid received a null widget and the spin floated at (0,0) - the same
  add-before-create mistake as the Quit button earlier.  The panel is now
  created after the spin and inserted with `leftCol->insertWidget(2, ...)`.

### Stage 3, part A — done and verified

* **The ROOT photon map is gone**: no `his3000` booking, no `disp3000` clone and
  no `A5` canvas.  All the remaining references are behind `if (his3000)` /
  `if (disp3000)` / `if (A5)` guards, so they are simply inert; the two
  unguarded dereferences (the clone and the z-range) were the only ones that
  needed deleting.  Verified: no `Plotting Canvas Photon Map` window appears.
* The **P.Map** range pair now reads and writes the Qt photon window's colour
  range (`liveMapRange` -> `m_photonView->colorRange`, and the panel's opening
  range), and the ADC zoom also sets the photon window's viewport.
* **Save with no gate ROI works**: the early refusal in `toggleRoiSave` is gone;
  `roiSnapshot()` already reports the full frame when no ROI is set.  Verified:
  with no `--gate-roi`, `/data` came out `(151, 512, 1024)` - the pedestal slice
  plus 150 full frames.

### Stage 3, part B — the photon-map save mode (next)

Design, so this can be written mechanically:

1. Save panel: a radio pair (`save frames` / `virtual photon counting`) plus an
   `M` spinbox, minimum **200**, default 2000; the spinbox is only meaningful in
   the photon mode.
2. Core: `gPmapSaveEvery` (int, default 2000) and `gSaveMode` (0 = frames,
   1 = photon maps) atomics; a **second accumulator** `g_pmapSaveAccum` (NCH
   floats) filled in the same worker pass (one extra add per counted photon
   pixel, gated by a `FrameJob::pmapSave` flag so there is no atomic read per
   pixel); every `gPmapSaveEvery` analysed frames it is copied into a single-slot
   mailbox and zeroed - never cumulative.
3. Window provenance: the acq thread owns the saver, so let the acq loop keep the
   window bookkeeping while in this mode: on the first gate-passed frame after a
   publish record `firstFrame = hdr.frameNumber` and `firstStamp =
   hdr.timestamp`, count frames, and when the analysis publishes a map, tag it
   with those two values.  The map's own frame count goes in too.
4. Writer: a new dataset `/virtual photon counting` (float32, 512x1024, one image
   per publish, extendable) plus `/virtual photon counting frame` (uint64) and
   `/virtual photon counting timestamp`; JSON gains `save_mode`,
   `photon_map_every` (M), `photon_threshold` (ADU) and the window count.
   `/data` stays frames-only and is not created in this mode.
5. `--pmap-every N` on the command line would make this testable without the GUI
   (the display cadence is otherwise 2000 frames = 80 s at 25 fps).

### Stage 3, part B — plumbing in place, inert

Done in the tree and building:

* `gSaveMode` (0 = frames, 1 = virtual photon counting) and `gPmapSaveEvery`
  (default 2000) atomics, plus `pmapSaveReset()`.
* A **second accumulator** `g_pmapSaveAccum` filled in the same worker pass
  (`FrameJob::pmapSave`, so no atomic read per pixel), published into a
  single-slot `PmapSlot` every `gPmapSaveEvery` analysed frames and zeroed
  immediately - never cumulative.
* `Pipeline::offerToSaver()` replaces the two direct `saver_.offer()` calls: in
  the frame mode it is the old path, in the photon mode it keeps the window
  bookkeeping (first frame number, first timestamp, frame count) in the acq
  thread and tags each published map with it.
* `RoiSaver::offerPhotonMap()` declared; its body currently only counts the map
  in `pmapPending` and writes nothing - **deliberately inert**: `gSaveMode` is
  still 0 everywhere, so no path can select the photon mode yet and the existing
  behaviour is unchanged.  Nothing can silently produce a frames-only file while
  claiming the photon mode.

Still to write (the design is above):

1. The writer body: `/virtual photon counting` (float32, 512x1024, extendable,
   **created up front** because SWMR forbids creating objects after the start)
   plus `/virtual photon counting frame` and `/virtual photon counting timestamp`
   (uint64), drained from a `pmapQueue` in the writer loop, and the JSON fields
   (`save_mode`, `photon_map_every`, `photon_threshold`, window count).
2. The Save panel controls: the radio pair (`save frames` /
   `virtual photon counting`) and the `M` spinbox (minimum 200), filling
   `StartInfo::photonMode/photonEvery/photonThreshold` (the fields are already
   there) and calling `pmapSaveReset()` when a save starts.
3. `--pmap-every N` and `--pmap-save-every N` (or one `--save-photon` switch) so
   the mode can be exercised headlessly; with the display default of 2000 frames
   a test needs 80 s at 25 fps otherwise.

### Stage 3, part B — finished and verified (this session)

Everything listed as "still to write" above is in the tree, plus the GUI
controls and the headless switches:

* Writer: `/virtual photon counting` (float32, {nmaps, h, w}, chunked, created
  before SWMR starts) + `/virtual photon counting frame` + `/virtual photon
  counting timestamp` (U64, one value per map), drained from `Impl::pmapQueue`
  in the writer loop; `RoiSaver::offerPhotonMap()` is live and inert unless
  `info.photonMode` and a save is running.  JSON: `save_mode`,
  `photon_map_every`, `photon_threshold`, and a `photon_maps` block with
  `saved`, `every`, `threshold_adu`, `cumulative: false`, `pedestal_active`,
  `dataset`.  In photon mode `frames.saved` reports the **maps**.
* Budget: in photon mode "Number of frames to be saved" budgets the **maps**
  (the GUI relabels the row to "Number of photon maps to be saved"), and the
  run stops by itself with `stop_reason = "photon-map budget reached"`, leaving
  a closed `complete: true` pair.  Frame mode is unchanged
  (`frame budget reached`).
* Save panel: radio pair + `photon map every N frames` spinbox (minimum 200,
  greyed while "save frames" is selected).  The radio also re-labels the budget
  row.  The photon-map mode is allowed with **no gate ROI** (full frame).
* Virtual photon counting panel (left column, under the gates): `cumulative`
  (default **off**), `plot every N images` (default 2000), `photon threshold
  (ADU)` (moved out of the Display panel).  The map lives in its own Qt window
  `Photon Map (virtual photon counting)`; it is **not** a ROOT histogram any
  more (that was the old `his3000`, removed).
* CLI: `--pmap-every N`, `--pmap-save-every N` (min 200), `--save-photon`,
  `--save-budget N`, `--start-save` — the same actions as the GUI controls, so
  a save can run headless.

**Bug found and fixed while verifying (important).**  Step 5a of the analysis
loop (the save-map publish) took `gHistoMtx` although the enclosing frame scope
already holds it (`histoLock`, top of the frame).  `std::mutex` is not
recursive, so the **analysis thread deadlocked on the first frame** whenever
`gSaveMode == 1` — with `--save-photon`, and even with no save running.  The
symptom was a program that kept counting frames (`in=` growing, because the acq
thread does not depend on the analysis thread) while nothing downstream ever
appeared: no photon map, no ROOT update, `save=0/0`.  Two `gdb` snapshots
caught it (`analysisLoop` parked in `pthread_mutex_lock` at a call site that
only exists in the photon path).  Fix: no second lock (the frame scope holds it)
and the whole save-map path is now gated on `saver_.running()` as well, so
`gSaveMode == 1` without a running save costs nothing and cannot block.

**Verified end to end** (Xvfb + mwm, `test/jf_feeder`, zero-filled pedestal —
the real `current_pede.dat` is ~2000 ADU, so the synthetic 100/300 ADU frames
would count nothing):

| check | result |
|---|---|
| `--save-photon --pmap-save-every 200 --start-save`, 40 s | maps at frames 0/201/401/601/801, `[pmap] map N written (200 frames, first frame F)` |
| `/virtual photon counting` | (5, 512, 1024) float32; every map 40 non-zero pixels, sum ~5320 (40 px x 133 counts = 2/3 of 200 frames), identical maps = non-cumulative |
| provenance | `/virtual photon counting frame` `[0 201 401 601 801]` |
| `--save-budget 2` | exactly 2 maps, self-stop `photon-map budget reached`, `complete: true` |
| frame mode `--save-budget 100` | `/data` (101, 512, 1024), 100 frames, `frame budget reached`, no photon datasets |
| GUI path | clicking `virtual photon counting` re-labels the row to *Number of photon maps to be saved*, enables the M spinbox and sets `Status: saving virtual photon counting maps`; `Start/Stop save` then writes maps (first map at the frame the save started, e.g. 431) |
| photon window | `Photon Map (virtual photon counting)` renders the 40-pixel cluster (green, ~133 counts on the 0..256 scale) |
| no pedestal | `--save-photon` with `--no-pede` (or no pedestal file) is **refused**: `[save] refusing virtual photon counting: "No pede" is on, ...`, no HDF5 file is created, and the scripted `--start-save` path is not blocked by a dialog; frame mode with no pedestal still saves |

The photon-map window draws **zero-count pixels white** (`FastImageView::
setZeroWhite`, opt-in and used only there), reproducing the ROOT `colz` look
where empty cells show the canvas background; the palette and the colour bar are
untouched, and single photons are still coloured (the special case is `v <= 0`,
not "below the range").  Verified on screen: white map, cluster still green.

Housekeeping: `test/_scratch/` (XTest driver, screenshots, run logs, HDF5
outputs, the synthetic pedestal) was removed after the runs; the tree keeps only
`test/jf_feeder[.cpp]`, `test/pede_live_test.cpp`, `test/panel_map_test.cpp`.

Two things left open for the next session, both decisions rather than bugs:

1. ~~**Photon counting needs a pedestal.**~~  **Decided by the operator: no
   photon-map saving without a pedestal.**  Counting still requires `g0`
   (unchanged), and a virtual-photon-counting save now *refuses to start* when
   no pedestal is active (`No pede`, or nothing loaded): the button drops back,
   the status line and stdout say why, no file is created, and the modal warning
   is skipped under `--start-save` so scripted runs never wait on a dialog.
   Frame saving is unaffected.
2. The map's `first frame number`/`timestamp` come from the acq thread, i.e.
   from the first **gate-passed** frame of the window (none is available if the
   gate drops the whole window, in which case the previous values persist).  The
   analysed-frame count in `PmapItem::frames` is exact.

### ROI in photon mode, greying convention, control names (follow-up)

* **The gate ROI applies to the saved photon maps too** (operator's choice):
  a map is the ROI window of the full-frame counting map, cropped in the same
  data coordinates as `/data`.  This fixed a real bug: `writePhotonMap` used to
  hand `h*w` floats straight to HDF5, i.e. the first `h*w` pixels of the
  1024x512 map, which is the *wrong region* whenever a ROI is set.  Verified
  against the full-frame map: `/virtual photon counting` for a `0 0 100 50` ROI
  is bit-identical to `full[0:50, 0:100]`, with the 16 non-zero pixels exactly
  where the feeder's wrapped cluster is (row 1, x 0..15); a frame save with the
  same ROI gives `/data` `{nframes, 50, 100}`, so both modes crop alike.
* **Greying is a look, not an enablement** (`greyStage(QGroupBox*, bool,
  std::initializer_list<QWidget*> keepActive)`): the box, its title and its
  other contents dim, while the widget that *switches the stage back on* keeps
  normal text - otherwise a bypassed stage looks stuck.  Kept readable:
  `Pede/Raw toggle` (Pedestal), `Gate enabled` (signal gate), `Pedestal
  tracking`, `gain correction on/off` (Gain), and the storage-cell combo.  The
  `keV PH` checkbox stays disabled+greyed while the gain is off (earlier
  request: it keeps its tick for next time).
* **Virtual photon counting box greys without a pedestal** (`greyStage(
  pmapGroup_, !pede)`), matching the fact that the counting needs `g0` and the
  save refuses to start: one rule, the panel and the file agree.
* Renames: `No pede (raw ADC)` -> **`Pede/Raw toggle`** (state still spelled out
  by the label next to it), `Apply gain (g)` -> **`gain correction on/off (g)`**,
  plus a read-only **`gain corrected`** indicator beside it (no mouse events, no
  focus; it ticks with the flag in `updateFlow()` and dims with the panel).

* Pedestal tracking panel follow-up: the button is `pedestal tracking on/off`
  and the box now greys on **selectability**, not on activity - `greyStage(
  pedeTrackGroup_, !gateOn)`, matching the button's own `setEnabled(gateOn)`,
  while the value label under it (`off` / `on: N samples, ...`) reports the
  activation.  Verified in three states: gate off -> box and button greyed;
  gate on + tracking off -> box black and label `off`; after clicking the
  toggle -> button pressed and label `on: 0 samples, delta 0.0 ADU, 0 updates`
  (0 samples is correct with the feeder, which only drops a third of the frames
  while the tracker needs >90 %).

### Photon threshold in keV while the gain correction is on

Requested: leave the photon-counting threshold in ADU with the gain off, switch
it to keV (default **4 keV**) with the gain on, and remember both values.

* Core: `g_pheneThresholdKev` (default `PHOTON_THRESHOLD_KEV_DEFAULT` 4.0) next
  to `g_pheneThreshold` (150 ADU).  The analysis picks by `gain_flag` and hands
  the worker `FrameJob::pcGain` / `pcNominalGain`, so the counting value is
  `adcpedecorr / (g0[i] or 40)` - the same ADU/keV the keV spectra use - and the
  increment follows in the same unit: `(value + thr) / (2 x thr)`.  The unit
  follows the gain stage itself, not the `keV PH` checkbox (that one only
  decides what the spectra display).
* GUI: the threshold row is a `QDoubleSpinBox` (integer ADU, one decimal keV)
  whose label switches `(ADU)` <-> `(keV)`; `setApplyGain()` stores the value
  that is on screen into its own unit and loads the other one (signals blocked),
  so toggling never destroys either.  `--apply-gain` added as the CLI mirror of
  the Gain button (the gain stage was the only one without one) - needed to test
  this headlessly.
* Reported: `--stats` prints `pthr=150ADU` / `pthr=4keV`; the JSON carries
  `photon_threshold_unit` and `photon_maps.threshold` + `threshold_unit`
  (`threshold_adu` is gone - the key was wrong as soon as the unit could
  change).
* Verified with the synthetic feeder, a zero pedestal and a **g0 = 1.0
  ADU/keV** gain map (float64 - the loader wants 6 x 1024x512 doubles; a float32
  file is rejected with the byte count):
  - gain off: `pthr=150ADU`, one 200-frame map with **40 non-zero pixels, sum
    5320, max 133** - i.e. only the 300 ADU cluster, unchanged from before;
  - `--apply-gain`: `pthr=4keV`, the **same frames** give **524288 non-zero
    pixels** (every pixel: keV = ADC/1.0, so even the 100 ADU baseline is
    100 keV > 4 keV) with max 5925 - proof that the counting really divides by
    ADU/keV before comparing;
  - GUI: starting with `--apply-gain` shows `photon threshold (keV): 4.0`;
    clicking the gain off shows `photon threshold (ADU): 150`; clicking it back
    on gives a screenshot byte-identical to the first keV one, so both values
    survive the round trip.
* Mid-save guard: toggling the gain while a photon-map save runs changes the
  unit of the maps written after it, so `setApplyGain()` prints
  `[gain] warning: the photon-counting unit is now keV while a photon-map save
  is running ...` (and the same for ADU).  Verified with a photon save started
  by `--start-save` and a gain click: the warning appears once, the save keeps
  running and writes its map, the JSON keeps the start-of-save threshold/unit.

### Window handling, labels and the flow arrows

* **Quit closes every window.**  The ADC / gain / photon-map windows are
  separate top-levels and the ROOT canvases live in the ROOT thread, so
  `quitApp()` and `closeEvent()` now call `closeSideWindows()` (hide + close
  each) and `QApplication::quit()`.  Closing only the control window used to
  leave the side windows - and the process, because Qt quits on the *last*
  window - on screen.  Verified on all three paths with the window count taken
  from `xwininfo -root -tree`:
  - Quit button, dialog confirmed: 6 app windows -> **0**, process gone;
  - Quit button with a photon-map save running: the save is flushed
    (`[shutdown] flushing the ROI save ...`, JSON `complete: true`), then
    **0** windows / no process;
  - window-manager close (a `WM_DELETE_WINDOW` ClientMessage, the path that was
    broken): confirmed -> **0** windows / no process.
* Gain panel label: the no-map case is now
  `no gain map loaded : using defaults` (the "defaults" are the nominal
  40 ADU/keV, used for the keV spectra *and* the keV photon threshold).
* Flow arrows: two new connectors - **Gain -> Virtual photon counting** (the
  calibration; its threshold is keV while the gain is on) and **Virtual photon
  counting -> Display** (the map has a window of its own) - and the save path is
  now **orange**: the Gain -> ROI rail and the ROI -> Save arrow.  Blue stays
  the analysis/display chain.  `FlowStrip::Path` gained a `save` flag and
  `makeFlowArrow()` a colour argument (`#4673af` blue, `#de7d14` orange).
  Verified on screen in the gain-on/pede-on state: both new arrows land on the
  right panels, the rail and the ROI -> Save arrow are orange, the rest blue.
* The `JF_TIMING` `[rect]` diagnostic also prints `quit`, `track` and `pmap`
  now, which is how the click-based tests above locate those widgets.

### Dynamic flow colours (arrows static, colours not)

Reworked after the operator's feedback: the geometry of the diagram is fixed,
only the colour class of each connector changes.

* Three classes (`FlowClass` + `flowColor()`): `FlowData` orange `#de7d14`,
  `FlowNormal` light blue `#5b9bd5`, `FlowPedestal` lighter blue `#a8c8e8`.
* The **data path follows the save mode**: `FlowStrip::setSaveMode(photonMaps)`
  flips the Gain -> ROI rail and the Gain -> counting connector, and
  `updateFlow()` (called from the radio handler) flips the two label arrows
  ROI -> Save and counting -> Save.  So in `save frames` the orange pair is
  rail + ROI -> Save, and in `virtual photon counting` it is Gain -> counting +
  counting -> Save - exactly two arrows at a time.
* Completeness: every stage now has an incoming and an outgoing connector.
  The missing one was **Virtual photon counting -> Save** (a new arrow under the
  counting panel, the left-hand twin of ROI -> Save).  Display and Save are the
  sinks; the storage cell has no incoming connector because the receiver stream
  enters there.
* The colour legend is in the flow strip's tooltip and in the README table.
* Verified on screen: frames mode -> rail + ROI -> Save orange, counting arrows
  light blue; after clicking `virtual photon counting` (and the label switching
  to "Number of photon maps to be saved") -> Gain -> counting and counting ->
  Save orange, rail + ROI -> Save light blue.  `[save] mode: ...` is logged on
  every change.
* Test note: the `JF_TIMING` `[rect]` lines can be ~30 px stale (they are printed
  on a layout pass that may run before the panels settle), so a scripted click
  should target the widget with a small downward tolerance - found the hard way
  when a radio click missed.

### The gate is a funnel on the line (not a stage)

Operator's rule: the signal gate does not modify the data, so no connector may
enter or leave its box.  Reworked accordingly:

* The flow is now `Pedestal -> [funnel] -> Gain` (and `[funnel] -> tracking`),
  i.e. one junction where the gate acts, drawn as a small trapezoid on the line.
  The gate panel's only connector is a short blue **control arrow** into the
  funnel's side; with the gate off the funnel and that arrow are drawn pale
  (`FlowOff`, #c9c9c9) instead of disappearing.
* Two feeders reach the funnel and both are always drawn: through the **Pedestal**
  (`FlowPedestal`) and straight from the **storage cell** (`FlowNormal`), the
  raw bypass that avoids the greyed Pedestal box.  `setStages(pede, gate, pmap)`
  makes the live one coloured and the idle one pale grey - so in raw mode the
  data visibly comes from block 1, as the operator asked.
* `FlowStrip::Path` grew `x` (vertical position), `head` (false = merge into the
  junction) and `startAtJog` (begin on the vertical, for the funnel -> Gain and
  funnel -> tracking branches); `cross()` honours them; `drawFunnel()` paints the
  symbol plus its control arrow; `kFunnelX = 24` is where the flow passes the
  gate.
* `updateFlow()` calls `setStages(pede, gateOn, gSaveMode == 1)`, so the four
  states (pede/raw x gate on/off) are all live: verified with screenshots in all
  four combinations - pedestal feeder live + funnel blue (pede+gate), funnel and
  control arrow pale (pede+no gate), bypass live and pedestal feeder pale
  (raw+gate), and the bypass stays live with the funnel pale (raw+no gate).

### Flow diagram, final pass (operator's corrections)

* **Lighter blue is now only the pedestal tracking pair** - the gate block ->
  tracking input and the tracking -> Pedestal feedback.  The storage cell ->
  Pedestal and Pedestal -> funnel connectors are part of the data flow, so they
  are orange like the rest of the route.
* **Orange is the whole save route**, entry point to Save panel, and it follows
  the mode: frames = storage cell -> Pedestal -> funnel -> Gain -> ROI -> Save,
  maps = ... -> Gain -> VPC -> Save.  Display-only branches stay light blue, and
  in photon mode the Gain -> ROI rail is light blue as well.
* **A route that carries no data is not drawn** (the operator found the pale
  grey confusing): the raw bypass exists only in raw mode, the Pedestal path
  only while the pedestal is in use, and only the selected mode's Save arrow
  (ROI -> Save or VPC -> Save) is shown.  The one thing that still greys is the
  gate funnel when the gate is off.
* The **tracking input** now leaves the top-right corner of the gate block
  (`yIn(sigGroup_, 6)`), not the funnel.
* Verified with screenshots in four states: pede+gate frames (orange: SC ->
  Pedestal -> funnel -> Gain -> rail -> ROI -> Save; bypass absent; tracking
  pair lighter blue), the same in photon mode (orange through Gain -> counting
  and the counting -> Save arrow, rail light blue, ROI -> Save absent),
  raw+gate frames (orange storage cell -> funnel -> Gain -> ROI -> Save; the
  Pedestal box carries no in/out at all, only the tracking feedback),
  and pede+no-gate (funnel and its control arrow pale, everything else
  unchanged).

### The display path and the save path can differ (last refinement)

The operator pointed out that the orange route must follow the *save* logic, not
the `Pede/Raw` toggle: `flowStageStates()` now computes

* `displayPede = PedestalStore::loaded() && !gNoPede`  (unchanged: the display,
  gate, spectra and the map accumulation),
* `savePede = (gSaveMode == 1) || (saveCorrected_ && !gNoPede)`  (the maps always
  need `g0`; the frames are corrected only with the checkbox),
* and `FlowStrip::setStages(displayPede, savePede, gateOn, photonMaps)` draws:

| situation | what the diagram shows |
|---|---|
| display pede, `save frames`, `Pedestal corrected` **off** (the default!) | Pedestal feeder **light blue** (display only) + bypass **orange** (the file) - both live, the divergence visible |
| `Pedestal corrected` **on** (or photon maps) | Pedestal feeder **orange**, bypass not drawn |
| `Pede/Raw` = raw (both raw) | Pedestal path not drawn at all, bypass orange |
| photon mode | pedestal route orange through Gain -> counting -> Save; the Gain -> ROI rail is light blue and the ROI -> Save arrow is absent |

`saveCorrected_` also got a `toggled -> updateFlow()` connection (it had none) so
the diagram follows the checkbox, and a `saveCorrected` entry in the `[rect]`
geometry diagnostics for the click tests.

Verified with screenshots: `--use-pede --gate` in frames mode showed the blue
storage cell -> Pedestal -> funnel next to the orange storage cell -> funnel ->
Gain -> ROI -> Save (both feeders into one funnel), and clicking `Pedestal
corrected` turned the pedestal route orange and removed the bypass.

### ROI input arrow + "Unset ROI" button

* The ROI's incoming connector is now mode-dependent and always orange:
  `Gain -> ROI` (the rail) while frames are written, `counting -> ROI` (a new
  connector) while the maps are - they are cropped to the ROI on the way out, so
  the ROI is only ever on the save path.  Only one of the two is drawn at a
  time; `setStages()` flips them with `p4_.on = !photonMaps` / `p9_.on =
  photonMaps`.
* New **Unset ROI** button in the ROI panel (`unsetRoi()`): releases the "ROI
  only" restriction if it is on, then `GateEngine::clearRoi()` (so `roiSet_` and
  `roiOn_` are false and the rectangle is the full frame), clears the yellow
  overlay, resets the counters and updates the read-outs.  The ROI list file is
  not touched.
* Verified: with `--gate-roi 100 50 200 100 --gate --stats` the live line showed
  `roi=200x100@100,50`; clicking **Unset ROI** logged `[roi] ROI unset: full
  frame` and the same line then showed `roi=off`, the panel reading
  "no ROI selected | gate: full frame".  In `--save-photon` (photon mode) the
  screenshot shows the orange connector from the counting panel into the ROI and
  no rail from the Gain.

### Save-time locking, truthful JSON, ROI->Save, and small labels

Operator requests, all in and verified with a running `--save-photon` run:

* **The arrow into the Save panel always leaves the ROI** (never the counting
  panel): `arrPmapDown_` is gone, `arrRoiDown_` is always drawn and always
  orange.  The maps reach the file through the ROI, which is also why the ROI's
  own input flips between the Gain (frames) and the counting (maps).
* `cumulative` -> **`cumulative (toggle to reset)`**, and the toggle now really
  resets (`resetHis()`), so the label is true.
* **`Pedestal corrected` moved under the `save frames` radio** (it is a property
  of the saved frames; the maps are always pedestal-subtracted) and is greyed
  while the photon-map mode is selected.
* **The save counters no longer read 0 in photon mode**: `RoiSaver::Status`
  gained `pmaps` + `photonMode` (Impl::pmapsInFile, fed by the writer loop), so
  the GUI status line says "N maps written, M dropped" and `--stats` says
  `save=1map/0` instead of `0/0`.
* **Settings are locked while a save runs** (`applySaveLock()`): the whole save
  panel except `Start/Stop save` and the status label, the storage-cell filter,
  the manual pedestal work, the gain correction (keV PH stays live) and the
  ROI select/load/unset buttons.  Live on purpose: everything display-only, the
  pedestal tracker, the gate **threshold** and **min pixels**.  Those two open
  `Changing settings during a run, are you sure?`; No puts the old value back
  (`pipeline_.gate().threshold()/minPixels()`), Yes applies it and sets
  `gGateChangedDuringSave`, which makes `writeJsonSidecar()` add
  `gate_settings_modified_during_run: true` and a `gate_settings_note` (the
  `gate` block stays the criterion from save start).  The lock is applied on
  start, released on stop - including the automatic stop (budget/error), which
  `updateStatus()` detects through `m_saveWasRunning`.
* Verified on screen during a running photon-map save: save panel greyed except
  Start/Stop, SC/pede/gain greyed, Threshold 150.0 and Min pixels 5 live, the
  Virtual photon counting panel's two display controls live with the new label,
  the photon threshold greyed, `Pedestal corrected` greyed under the frames
  radio, and the Save status reading "1 map written, 0 dropped".

### Layout bug (mine), destination list, "queue full"

* **The `virtual photon counting` radio was rendered outside the Save box.**  When
  I moved `Pedestal corrected` under the `save frames` radio I dropped the
  `saveForm->addRow(saveModePhoton_);` line, so the radio had no layout slot and
  Qt gave it a default geometry (measured: x=9, y=915, 100x30 - i.e. floating at
  the left edge above the Save box, which is exactly what showed up as a stray
  "Virtual phot..." fragment).  Re-added; the `[rect]` diagnostic now reads
  `saveFramesRadio 21,996 | saveCorrected 21,1025 | savePhotonRadio 21,1054`,
  all 788 px wide inside the box.  (`layoutFlow()` also repaints the central
  widget after solving now, which is cheap insurance against leftovers in the
  pads' wake.)
* **The previous-destinations list is no longer polluted by the default**: the
  start-up `--h5-dir`/`--h5-base` pair is captured once (`saveDefaultDir_`,
  `saveDefaultBase_`) and a save that uses exactly that pair logs
  `[save] <dir>/<base> is the default destination, not added to the
  previous-destinations list` instead of recording it.  Verified: the list stayed
  at 22 lines over a `--start-save` run with the default destination.  My own
  test entries (`*_scratch*`, 23 of them) were pruned from
  `save_dest_list.dat`; the real `/mnt/sls_det_storage/...` destinations and the
  `test/_run`/`testPHmapsave` entries were left alone.
* The dropped counter now reads `(writer behind: queue full)` and the README
  explains it: the writer buffers at most 4096 items / 256 MB, so a slower disk
  than the stream means the excess is dropped (and still counted by the gate
  counters).

* The drop note was attached unconditionally, so a healthy run read
  "0 dropped (queue full)" - a problem that was not there.  It now says
  `0 dropped (nothing lost)` while nothing is lost and only switches to
  `(writer behind: queue full)` when the counter is non-zero.  Verified on a
  running photon-map save: "1 map written, 0 dropped (nothing lost)".

* The "Changing settings during a run, are you sure?" popup is now raised only
  when **`Gate enabled` is ticked** (`confirmGateChangeDuringRun` returns early
  on `!pipeline_.gate().enabled()`): with the gate off (passthrough) the
  threshold and min-pixels change nothing about the saved frames, so they go
  through silently and no `gate_settings_*` note is written.  Verified with a
  running save: gate on + clicking a gate spinbox -> dialog shown and declining
  logged `[save] gate change refused during the run (Min pixels above threshold
  -> 6)`; the same click with `--no-gate` -> no dialog, no log line.

### Image windows redrawn as plots (frame, axes, pixel grid, scale panel)

Operator request: replace the side colour bar with a panel and make the images
read as plots.

* `FastImageView::imageRect()` now reserves axis margins (52 left / 26 bottom /
  6 top-right) instead of the old bar strip; `paintEvent()` draws the frame,
  tick labels outside the plot (Y = data rows, row 0 at the bottom), a dotted
  grid inside it, and keeps the gate-ROI / receiver-ROI overlays as before.
* `m_sx` (screen pixels per image pixel, from the last paint) drives the pixel
  detail: thin outlines from 16 px, in-pixel values from 50 px
  (`setIntegerValues(true)` for the photon map -> whole numbers, one decimal for
  the real-valued images).  The label colour is chosen from the luminance of the
  pixel that was actually painted; the loops are capped (<=256 px visible for
  the grid, <=64 for the labels) so a deep zoom can never stall the painter.
* The colour bar is off (`m_showBar = false`; `scaleRect()` returns an empty
  rect so the old drag code is inert) and each image window now carries a
  `ScalePanel` under it: `ScaleHisto` (the image's own 256-bin distribution with
  the mapped range highlighted, refreshed at most every 300 ms from
  `renderImage()`), a palette chooser wired to `paletteLut()` (kept in sync with
  the `b`/`Shift+B`/`c` shortcuts), a lin/log switch, and min/max spin boxes
  (also synced from `Set Ranges...`).
* log mode maps `log10(max(v, 1e-2))` against `log10(max(range, 1e-2))`, so
  values <= 0 land at 1e-2 = -2 instead of -inf, and the "zero is white" rule
  for the photon map still applies first.
* Verified on screen: the framed plot with the X labels (475..505) and Y labels
  (97..90, high at the top) outside the frame, the dotted grid, the per-pixel
  outlines at ~33 screen px/pixel, and the panel below with the histogram,
  `palette: Col (jet)`, `lin`, `min -200.0`, `max 1000.0` (the ADC defaults).
  Not yet seen on screen: the in-pixel numbers (they need >50 px/pixel, my
  scripted wheel zoom stopped at ~33) and the histogram with a varied image
  (a uniform test frame gives a single bin).  The old bar's drag-to-stretch on
  the scale is gone by design: the range is set with the spin boxes (dragging on
  the histogram is a possible follow-up).

* Scale histogram refined to the operator's spec: fixed 38 px strip (the image
  keeps the window stretch, the panel has a fixed height - about a tenth of the
  window), one **solid** bar per bin whose width is the bin spacing on screen,
  and the two range markers pinned at **1/4 and 3/4** of the width with the value
  axis rescaling under them (the drawn window is the selected range widened by
  half a range on each side, so dragging a marker moves that end of the range
  and the marker snaps back to its fixed spot).  The bars are draggable
  (`onRangeChanged` -> the panel pushes the new range into the view and the spin
  boxes) and the ends are printed next to the markers.  A **second lin/log
  switch** was added: `image` for the intensity mapping, `histo` for the bar
  heights (log10(count+1)); the histogram's value axis always follows the image
  switch.  Verified on screen: the thin strip with the two orange markers at
  1/4 and 3/4 labelled `702.7` and `1000`, and a drag of the left marker moving
  `min` from -200 to 702.7 (the bars were off-window by then, which is the
  fixed-marker behaviour).

### Palette list widened to the matplotlib family

The scale panel's palette chooser now offers 18 maps instead of 3 (the original
`Col (jet)`, the two ROOT-like greys, then the new ones).  The `Palette` enum
keeps `Color/BW1/BW2` first so the `b` / `Shift+B` / `c` shortcuts and
`setPaletteChoice()` keep working, and `paletteName()` (new, in
`online_jf_root.cpp`, deliberately outside the anonymous namespace) feeds the
combo so the names live in one place.

* New: Turbo, Viridis, Plasma, Inferno, Magma, Cividis, Hot, Grey (linear),
  Grey reversed, Coolwarm, BWR, Seismic, RdBu, YlGnBu, Sunset.
* Anchors: the perceptually uniform ones are matplotlib's published anchors and
  Turbo is Google's; the diverging/sequential set uses matplotlib's definitions
  and the ColorBrewer / Paul Tol anchors (YlGnBu, Sunset) as republished in
  `bokeh/palettes.py`, which is where I looked them up - bokeh's licence header
  attributes Viridis/Magma/Plasma/Inferno to Nathaniel Smith et al. (CC0),
  Turbo to Google (Apache-2.0), the Brewer maps to Cynthia Brewer (Apache-2.0)
  and Tol's to Paul Tol (BSD-3).  Everything is interpolated by the existing
  piecewise-linear `PaletteDef`, whose stop/colour arrays were grown from 5 to
  24 entries to hold the 15-anchor turbo ramp; the LUTs are now one
  `g_luts[kNumPalettes][256]` table built by the same `buildLut()` loop, so a new
  palette is only an enum entry plus its anchors.
* Verified: the entry list builds and the app runs; selecting `Seismic
  (diverging)` in the ADC window turned the image seismic-blue (raw ~100 ADU on
  the -200..1000 range lands just below the white midpoint, as expected) with
  the panel and histogram following.

* The **gain window** lost its scale panel entirely (no histogram, no palette /
  lin-log / range controls): its image holds four valid numbers - the gain bits
  0..3 - so there is neither a distribution to show nor a scale to choose.  It
  now uses a fixed `Palette::Gain4` map (0 muted blue, 1 muted green, 2 muted
  yellow, 3 muted red, all desaturated on purpose so a large flat map does not
  glare) with the range pinned to 0..3, and whole-number in-pixel labels.  The
  palette is in the enum but deliberately **not** in the chooser loop
  (`i <= int(Palette::Sunset)`), so the three image windows keep their own
  choices.

* In-pixel numbers: the font now also has to fit the *width* of the label, not
  just the pixel height - five characters for the photon counts, six for the
  real-valued images ("XXXXX.y"), at ~0.62 em per digit, still capped at 0.30 em
  of the pixel so the "small tidy number" look is kept (range now 7..20 px).
  At 50 px/pixel that is 12 px instead of 15, which is what makes five digits
  fit; the widest label stays under the pixel edge for every zoom level above
  the 50 px threshold.

### Zoom coordination switch (Display panel)

Operator request: zooming used to be ADC-master only (the ADC view propagated to
the gain/photon windows and the ROOT plots, never the other way round).  Now
`zoomTo(FastImageView *src, ...)` takes the view that asked, and every image view
reports its zoom through the same wiring (`wireZoom()` for ADC, gain and photon),
so **any** window zooms all of them.  A new `zoom coordination` checkbox in the
Display panel (default **on**) gates the propagation: when it is off, `zoomTo()`
returns immediately, so each window keeps its own zoom and nothing is pushed to
the others - including the ROOT canvases, which as a consequence no longer follow
the ADC while the box is off (say the word if they should keep following it
regardless).  Both transitions are logged (`[zoom] coordination on/off`) and
every coordinated zoom logs its source (`[zoom] coordinated from ADC/gain/photon`).

Verified: with the default on, five wheel steps over the ADC window produced 10
`coordinated from ADC` events (two per step: wheel plus the drag path); the
checkbox is present at a sane place in the Display panel
(`[rect] zoomCoord 476,730 333x23`, ticked).  The OFF case is a single
`if (!zoomCoord_) return;` on that same path.

### Makefile: both variants from one `make`

`make jfstrx` used to rebuild the *same* binary with `-DJFSTRX` (the old
`.buildflags` stamp existed only to force a full rebuild when switching), so the
two variants overwrote each other.  Now the objects and the executables are
separate:

* `online_jf_*.o` + `jf_online_display_gui` (normal),
* `online_jf_*.JFSTRX.o` + `jf_online_display_gui_JFSTRX` (compiled with
  `-DJFSTRX`),

and `all` builds both, so the two can never be mixed by accident (they have
different geometry and struct layouts).  `make normal` and `make jfstrx` remain
as aliases, the flag stamp is gone, `clean` removes both sets, and `.gitignore`
covers the second binary and the `.JFSTRX.o` files.

* Zoomed images were painted over the axes: the scaled image is much larger than
  the plot area (only the viewport window is meant to be visible), and the ROI
  overlays were drawn in widget coordinates without a clip, so a ROI partly
  outside the visible window ran across the tick labels and up to the window
  edge.  `paintEvent()` now sets `setClipRect(ir)` around the image and around
  the two overlays (restoring before the axes so the frame, grid and labels are
  drawn on top of a clean widget).  Verified by sampling a deep-zoom screenshot:
  the margin left of the frame is the widget's black background, not image
  colour, while the plot interior still carries the image.

* The strip-xel decoder (../../dataStructures/jungfrauLGADStrixelsData_new.h)
  dumped its header on every construction - "Jungfrau strixels 2X single chip
  with full module data" / "sizeofheader = 112" - once per displayed frame in
  the JFSTRX build.  Those three `std::cout` statements are commented out (the
  header is shared, outside this project, so the edit needed the wider sandbox
  permission; it is a comment-only change and easy to revert).
* Deepest zoom is now build-dependent (`kMinViewport`): 8 image pixels across in
  the plain build as before, **2** in the JFSTRX build, so single strixel pixels
  can be inspected (the mapped panels are ~1000 x 1900).  The same constant
  guards `setViewport()` and `zoomTo()`, so a coordinated zoom propagates down
  to the new limit.

### Dual-interface alignment: drain fast, pair index-aligned, shed on backlog

Operator report: after the machine had been busy for a while the two interfaces
stopped being aligned and never recovered, with the deltas fluctuating. There
were two causes, one on each side.

* **Holding a slab is back-pressure.** The old pairing stopped polling an
  interface whose slab was waiting for its partner (`events = 0`) and accepted
  only *equal* frame numbers. That fills slsReceiver's ZMQ ring, and the
  receiver then skips frames **on its own side** - per interface, not per
  frame - so the two frame numbers drift apart and, with exact matching, never
  meet again. Waiting "until the partner catches up" made it worse.

What the code does now (`Pipeline::tryAssemble()` in `online_jf_core.cpp`):

* **Always drain.** Every socket is polled on every turn and drained down to
  its newest message (bounded to 32 messages per socket and wake-up, so no
  interface is starved). Nothing is ever held, so the receiver's buffer stays
  empty - the whole point at 2 kHz.
* **Pair index-aligned or not at all.** When the newest slabs carry different
  frame numbers (one interface skipped), the pair is **dropped** (`resync`) and
  the next one is tried, so every frame that is displayed or saved is stitched
  from a single detector frame. No chasing, no unbounded wait, no half frames.
* **Latch for a permanent offset.** If nothing pairs for 500 ms the offset is
  structural (an interface that keeps skipping), so the newest slabs are paired
  anyway (`skewedPairs_`, logged) to keep the stream alive; strict pairing
  comes back by itself as soon as a pair matches again.
* **Decimation is manual by default**, `--decimate 2|4|8` (1 frame in N, dropped
  by detector frame number so both interfaces drop the same ones).  The automatic
  ladder (1 -> 2 -> 4 -> 8, stepping back up after 5 s clean) only runs with
  `--auto-decimate`: frames are queued in the *gate*, so the backlog is
  `framesIn - framesPass - framesDrop - framesScDrop` - counting gate rejects as
  backlog used to pin the ladder at 1/8 whenever the gate rejected anything.
* **A silent interface is reported, not waited for**, so a dead stream cannot
  freeze the pipeline.
* Audit: `--stats` prints `resync` / `behind` / `incomplete` and the
  per-second `[align] keep=1/<N> shed=<n>` line; ladder steps and "dropping the
  pair" messages are rate-limited.

Verified against the live receiver `10.1.1.106 30026+30027` at ~2000 frames/s:
20 s runs alone and with eight CPU hogs (`yes`) both keep `drop=0`,
`in == pass`, `keep=1/1`, `shed=0`, with no unpaired-interface message. The
synthetic feeder with equal keys gives `resync=0`; with a permanent one-frame
offset (`dual ... 1`) the latch keeps ~16 frames/s at 20 fps instead of
stalling. At 2 kHz the interfaces are a frame apart often enough that roughly
one pair in four is dropped by the strict rule - the intended trade (aligned
frames only). A one-frame lookahead per interface (swap two buffers, no copy)
would recover them; it is the obvious follow-up if the saved-frame rate ever
matters.
