# FragTest

**Fragmentation test of a 400 MeV/u ¹²C ion beam in water**, benchmarking
Geant4's hadronic physics models (QGSP_BIC_EMY, QMD, INCL, QBBC,
QGSP_BIC_HP_EMY) for fragment yields against Haettner et al.

---

## 🔬 What this test does

A 400 MeV/u carbon-12 beam is shot into a water column; the fragment
yield/charge distribution is scored at several water depths (59-347 mm,
spanning the Bragg peak region) and compared across hadronic physics models
(`FragTestExp.py` holds the experimental reference data).

## 📚 Reference

- Source: `geant-validation-tests/src/FragTest`.
- Haettner E. et al. — carbon-ion fragmentation measurements in water.

## Note: distinct from `LowEFrag` and `BraggPeak`

This is a different slide item from "62 MeV/u ¹²C fragmentation"
(→ `LowEFrag`, already in this organization) and from the carbon-ion
Bragg-curve test found at `geant-validation-tests/src/BraggPeak` (62/200/400
MeV/u ¹²C dose-depth curves, not fragment yields) — `FragTest` specifically
measures fragment distributions at 400 MeV/u.

## Import notes

- `main.cc`, `include/`, `src/` come from `FragTest.cc`/`include`/`src`.
- `macro/` contains the 8 macros found directly in that source tree (kept
  for reference, not used by CI — none of the 3 `run*MeV.mac` ones are
  self-contained, and one even `/control/execute`s a file,
  `run400MeV.mac`, that doesn't exist in the repository), plus one
  concrete, self-contained macro built from `geant-config-generator`'s real
  `template.conf` — see `validation/README.md`.
- `validation/parser.py` was **rewritten** for the `metadata()`/`parse()`
  module API that `ci-workflows/validation/export.py` actually calls (the
  first version, copied unmodified from
  `geant-config-generator/tests/geant4/FragTest/parser.py`, used the old
  `gts.BaseParser` class API and would have failed CI the same way
  `fanoCavity` initially did). `FragTestExp.py` (experimental reference
  data) is still copied alongside it, now as a plain sibling-module import
  (`from FragTestExp import ...`, same pattern as `LowEFrag`'s
  `LowEFragExp`) rather than the original's package-relative
  `from . import FragTestExp`. See `validation/README.md` for details,
  including why — unlike every other test in this import batch — no
  analysis had to be restructured into single-point records despite the
  move to one-macro-one-run. The vendored `gts` package is no longer needed
  and was removed.
