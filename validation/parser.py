#!/usr/bin/env python

# FragTest portal parser: metadata() reads the macro, parse() the
# "hist_z_thick_<T>" / "hist_theta_thick_<T>_z_<Z>" /
# "hist_energy_thick_<T>_z_<Z>_theta_<A>" histograms written to output.root.
#
# Ported from geant-config-generator/tests/geant4/FragTest/parser.py
# (gts.BaseParser + PyROOT, merging repeated-seed job groups) to the
# module-level metadata()/parse() API expected by
# ci-workflows/validation/export.py, reading ROOT with uproot (no PyROOT on
# the runner). `THICKNESS` is not actually a macro-configurable parameter —
# geant-config-generator's own template.conf never substitutes it, so every
# run already scores all 7 water depths from params.conf (`!THICKNESS=59,
# 159, 258, 279, 288, 312, 347`) into one output.root; the only thing that
# genuinely varies between macros is the hadronic physics MODEL. There is
# therefore no job-group merge needed at all: every chart the original
# built from a (MODEL, all-THICKNESS) job group is still fully computable
# from a single macro/run.
import math
import os

from geantval import getJSON, one_command, single_run
from FragTestExp import exp as EXP_YIELD, ANGLE_EXP_POINTS, ENERGY_EXP_POINTS

THICKNESSES = (59, 159, 258, 279, 288, 312, 347)
Z_ELEMENTS = (1, 2, 3, 4, 5)
DET_ANGLE_WIDTH = math.atan(20. / 2940.)


def metadata(commands):
    nevents = single_run(commands)
    return {"TEST": "FragTest",
            "MODEL": one_command(commands, "/physics/addPhysics"),
            "NEVENTS": nevents}


def find_bin(edges, value):
    """0-indexed equivalent of ROOT TH1::FindBin, None if under/overflow."""
    if value < edges[0] or value >= edges[-1]:
        return None
    for i in range(len(edges) - 1):
        if edges[i] <= value < edges[i + 1]:
            return i
    return None


def bin_content(hist, value):
    edges = hist.axis().edges(flow=False).tolist()
    ib = find_bin(edges, value)
    if ib is None:
        return None
    values = hist.values(flow=False)
    width = edges[ib + 1] - edges[ib]
    return float(values[ib]), width


def get_yield_by_z(root, ntotal):
    result = {}
    for thickness in THICKNESSES:
        hist = root["hist_z_thick_%d" % thickness]
        result[thickness] = {}
        for z in Z_ELEMENTS:
            content = bin_content(hist, z)
            if content is None:
                continue
            result[thickness][z] = content[0] / float(ntotal)
    return result


def normalize_factor(yield_by_z, z, thickness):
    return EXP_YIELD[z][thickness] / yield_by_z[thickness][z]


def get_angular_distribution(root, thickness, z, ntotal):
    points = ANGLE_EXP_POINTS[thickness][z]
    hist = root["hist_theta_thick_%d_z_%d" % (thickness, z)]
    edges = hist.axis().edges(flow=False).tolist()
    values = hist.values(flow=False)
    result = []
    for i, point in enumerate(points):
        if i >= len(values):
            continue
        n = float(values[i])
        start = abs(point) if point != 0 else 0.
        start_rad = math.radians(start)
        end_rad = start_rad + DET_ANGLE_WIDTH
        if point != 0:
            start_rad -= DET_ANGLE_WIDTH
            solid_ang = math.pi * (math.cos(start_rad) - math.cos(end_rad))
        else:
            solid_ang = 2. * math.pi * (math.cos(start_rad) - math.cos(end_rad))
        y = n / ntotal / solid_ang
        err = math.sqrt(n) / ntotal / solid_ang
        result.append((point, y, err))
    return result


def get_energy_distribution(root, thickness, z, angle, ntotal):
    solid_ang_base = 2. * math.pi * (1 - math.cos(DET_ANGLE_WIDTH)) if angle == 0 else \
        math.pi * (math.cos(math.radians(angle) - DET_ANGLE_WIDTH) - math.cos(math.radians(angle) + DET_ANGLE_WIDTH))
    solid_ang = solid_ang_base * (2. if angle != 0 else 1.)
    hist = root["hist_energy_thick_%d_z_%d_theta_%d" % (thickness, z, angle)]
    result = []
    for energy in sorted(ENERGY_EXP_POINTS[thickness][z][angle]):
        content = bin_content(hist, energy)
        if content is None:
            continue
        n, width = content
        denom = solid_ang * ntotal * width
        result.append((energy, n / denom, math.sqrt(n) / denom))
    return result


def parse(job):
    import uproot

    ntotal = float(job["NEVENTS"])
    filename = os.path.join(job["path"], "output.root")
    with uproot.open(filename) as root:
        yield_by_z = get_yield_by_z(root, ntotal)

        for thickness in THICKNESSES:
            for z in Z_ELEMENTS:
                if z not in yield_by_z.get(thickness, {}):
                    continue
                for angle in ENERGY_EXP_POINTS.get(thickness, {}).get(z, {}):
                    values = get_energy_distribution(root, thickness, z, angle, ntotal)
                    if not values:
                        continue
                    xval = [v[0] for v in values]
                    yval = [v[1] for v in values]
                    yerr = [v[2] for v in values]
                    factor = normalize_factor(yield_by_z, z, thickness)
                    parameters = [{"names": "Z", "values": str(z)},
                                  {"names": "thickness", "values": "%s mm" % thickness},
                                  {"names": "angle", "values": str(angle)}]
                    yield getJSON(job, "chart", mctool_name="GEANT4", mctool_model=job["MODEL"],
                                  observableName="energy distribution", targetName="water",
                                  beamParticle="C12", beamEnergies=[400], parameters=parameters,
                                  title="energy distribution", xAxisName="Energy, MeV",
                                  yAxisName="(N/N_{0})(#Omega*MeV)^{-1}",
                                  xValues=xval, yValues=yval, yStatErrorsPlus=yerr, yStatErrorsMinus=yerr)
                    yield getJSON(job, "chart", mctool_name="GEANT4", mctool_model=job["MODEL"],
                                  observableName="energy distribution (scaled to exp. yield)", targetName="water",
                                  beamParticle="C12", beamEnergies=[400], parameters=parameters,
                                  title="energy distribution", xAxisName="Energy, MeV",
                                  yAxisName="(N/N_{0})(#Omega*MeV)^{-1}",
                                  xValues=xval, yValues=[y * factor for y in yval],
                                  yStatErrorsPlus=[e * factor for e in yerr],
                                  yStatErrorsMinus=[e * factor for e in yerr])

        for z in Z_ELEMENTS:
            xvalues, yvalues, yerr = [], [], []
            for thickness in THICKNESSES:
                if z not in yield_by_z.get(thickness, {}):
                    continue
                y = yield_by_z[thickness][z]
                xvalues.append(thickness)
                yvalues.append(y)
                yerr.append(math.sqrt(y * ntotal) / ntotal)
            if not xvalues:
                continue
            yield getJSON(job, "chart", mctool_name="GEANT4", mctool_model=job["MODEL"],
                          observableName="fragment yield", targetName="water",
                          beamParticle="C12", beamEnergies=[400],
                          parameters=[{"names": "Z", "values": str(z)}],
                          title="fragment yield", xAxisName="thickness of water, mm", yAxisName="N/N_{0}",
                          xValues=xvalues, yValues=yvalues, yStatErrorsPlus=yerr, yStatErrorsMinus=yerr)

        for thickness in THICKNESSES:
            if thickness not in yield_by_z:
                continue
            xvalues, yvalues, yerr = [], [], []
            for z in sorted(yield_by_z[thickness]):
                y = yield_by_z[thickness][z]
                xvalues.append(z)
                yvalues.append(y)
                yerr.append(math.sqrt(y * ntotal) / ntotal)
            yield getJSON(job, "histogram", mctool_name="GEANT4", mctool_model=job["MODEL"],
                          observableName="fragment yield by Z", targetName="water",
                          beamParticle="C12", beamEnergies=[400],
                          parameters=[{"names": "thickness", "values": "%s mm" % thickness}],
                          title="fragment yield by Z", xAxisName="Z", yAxisName="N/N_{0}",
                          binEdgeLow=[x - 0.5 for x in xvalues], binEdgeHigh=[x + 0.5 for x in xvalues],
                          binContent=yvalues, yStatErrorsPlus=yerr, yStatErrorsMinus=yerr)

        for thickness in THICKNESSES:
            for z in yield_by_z.get(thickness, {}):
                items = get_angular_distribution(root, thickness, z, ntotal)
                if not items:
                    continue
                items.sort(key=lambda x: x[0])
                xval = [v[0] for v in items]
                yval = [v[1] for v in items]
                yerr = [v[2] for v in items]
                factor = normalize_factor(yield_by_z, z, thickness)
                parameters = [{"names": "Z", "values": str(z)},
                              {"names": "thickness", "values": "%s mm" % thickness}]
                yield getJSON(job, "chart", mctool_name="GEANT4", mctool_model=job["MODEL"],
                              observableName="angular distribution", targetName="water",
                              beamParticle="C12", beamEnergies=[400], parameters=parameters,
                              title="angular distribution", xAxisName="angle, deg",
                              yAxisName="(N/N_{0})#Omega^{-1}",
                              xValues=xval, yValues=yval, yStatErrorsPlus=yerr, yStatErrorsMinus=yerr)
                yield getJSON(job, "chart", mctool_name="GEANT4", mctool_model=job["MODEL"],
                              observableName="angular distribution (scaled to exp. yield)", targetName="water",
                              beamParticle="C12", beamEnergies=[400], parameters=parameters,
                              title="angular distribution", xAxisName="angle, deg",
                              yAxisName="(N/N_{0})#Omega^{-1}",
                              xValues=xval, yValues=[y * factor for y in yval],
                              yStatErrorsPlus=[e * factor for e in yerr],
                              yStatErrorsMinus=[e * factor for e in yerr])
