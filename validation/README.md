# Portal JSON parser

Rewritten from `geant-config-generator/tests/geant4/FragTest/parser.py`
(old `gts.BaseParser` + PyROOT, which merged a (MODEL, all-THICKNESS)
repeated-seed job group) for the module-level `metadata()`/`parse()` API
that `ci-workflows/validation/export.py` actually calls, reading ROOT with
`uproot` instead (no PyROOT on the runner). `FragTestExp.py` (the
experimental reference data `parser.py` depends on via
`from FragTestExp import exp, ANGLE_EXP_POINTS, ENERGY_EXP_POINTS`) is
copied alongside it — the same sibling-module import pattern already used
by `LowEFrag`'s `from LowEFragExp import ...` in this organization.

## One macro per water thickness

The water thickness is a real geometry parameter: it sizes the phantom
(`DetectorConstruction`) and selects which experimental binning
(`include/Binning.hh`) the histograms are booked with, so each run's
`output.root` only holds `hist_*_thick_<T>*` for its own depth. In
`geant-config-generator` it was passed as the third command-line argument
(`run.sh`: `FragTest FragTest.mac %RANDOMSEED% %THICKNESS%`), which the CI
(`apptainer run <image> <macro>`) cannot do; it is now set in the macro with

    /FragTest/phantomThickness 288 mm

which must come **before** `/run/initialize` (the geometry is built there;
the command is only accepted in `PreInit`, a later one interrupts the batch).
A thickness without experimental binning (anything but 59, 159, 258, 279,
288, 312, 347 mm) is a fatal `G4Exception` at `/run/beamOn`.

`config.json` lists 7 macros, `QGSP_BIC_EMY_400MeV_<T>mm.mac`, one per
`params.conf` depth, and `metadata()` reads `THICKNESS` from that command.
Energy/angular distributions and yield-by-Z are per-thickness charts and come
out of one run unchanged; the yield-vs-thickness curve instead becomes one
**single-point** chart per Z and per macro (`nPoints: 1`), the same
restructuring used for `fanoCavity`'s `ROVER` and `microyz`'s energy sweeps:
the portal combines records sharing `testName`/`title`/`model`/`parameters`
across the 7 macros' `plots/*.json`. The `/ njobs` seed-count rescaling of the
original is dropped (one run per macro, normalised to its `NEVENTS`).

## The macro: not self-contained in its original form, and why

None of `geant-validation-tests/src/FragTest`'s own run macros
(`runBIC400MeV.mac`, `runQMD400MeV.mac`, `runINCL400MeV.mac`) are
self-contained — each does `/control/execute physicsXXX.mac` +
`/control/execute run400MeV.mac`, and **`run400MeV.mac` doesn't even exist
in the repository** (missing file). `macro/QGSP_BIC_EMY_400MeV_<T>mm.mac` are
built instead from `geant-config-generator`'s own real `template.conf`
(physics list fixed to `QGSP_BIC_EMY`, matching `physicsBIC.mac`), with its
`/control/alias`/`/control/multiply` beam-energy computation pre-resolved
to literal numbers at generation time, plus the
`/FragTest/phantomThickness` line above (`totalEnergy = 400 MeV/u × 12 =
4800.0 MeV`, `energySigma = totalEnergy × 0.0015 = 7.2 MeV`), the same
technique used for `BraggPeakGSI_MT` in this organization. The other 4
`params.conf` models (`QMD`, `INCL`, `QBBC`, `QGSP_BIC_HP_EMY`) are not
included — generate more macros the same way if needed (7 per model).

## `FindBin` and solid-angle reproduction

`find_bin()` reproduces ROOT's `TH1::FindBin(value)` (returning `None` for
the original's "wrong bin number" skip/continue cases) against `uproot`'s
0-indexed bin-edge arrays. `get_angular_distribution()`/
`get_energy_distribution()`'s solid-angle formulas are unchanged from the
original `getSolidAngle()`.
