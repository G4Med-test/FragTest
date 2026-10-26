#!/usr/bin/env python

# FragTest portal parser: metadata() reads the macro, parse() the
# "hist_z_thick_<T>" / "hist_theta_thick_<T>_z_<Z>" /
# "hist_energy_thick_<T>_z_<Z>_theta_<A>" histograms written to output.root.
#
# Ported from geant-config-generator/tests/geant4/FragTest/parser.py
# (gts.BaseParser + PyROOT, merging repeated-seed job groups) to the
# module-level metadata()/parse() API expected by
# ci-workflows/validation/export.py, reading ROOT with uproot (no PyROOT on
# the runner). The water thickness is a geometry parameter: each macro sets
# it with /FragTest/phantomThickness and its output.root only holds the
# histograms of that depth, so the yield-vs-thickness curve is exported as
# one single-point chart per macro (see README.md).
import math
import os

from geantval import getJSON, one_command, single_run
from FragTestExp import exp as EXP_YIELD, ANGLE_EXP_POINTS, ENERGY_EXP_POINTS

THICKNESSES = (59, 159, 258, 279, 288, 312, 347)
Z_ELEMENTS = (1, 2, 3, 4, 5)
DET_ANGLE_WIDTH = math.atan(20. / 2940.)
LENGTH_UNITS_MM = {"um": 1e-3, "mm": 1., "cm": 10., "m": 1e3}


def length_mm(value):
    """Convert a Geant4 length argument such as '288 mm' to mm."""
    parts = value.split()
    number, unit = (parts[0], parts[1]) if len(parts) == 2 else (parts[0], "mm")
    return float(number) * LENGTH_UNITS_MM[unit]


def metadata(commands):
    nevents = single_run(commands)
    thickness = length_mm(one_command(commands, "/FragTest/phantomThickness"))
    matches = [t for t in THICKNESSES if abs(t - thickness) < 1e-6]
    if not matches:
        raise ValueError("/FragTest/phantomThickness %g mm is not one of %s mm" % (thickness, THICKNESSES))
    return {"TEST": "FragTest",
            "MODEL": one_command(commands, "/physics/addPhysics"),
            "THICKNESS": matches[0],
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


def get_yield_by_z(root, thickness, ntotal):
    hist = root["hist_z_thick_%d" % thickness]
    result = {}
    for z in Z_ELEMENTS:
        content = bin_content(hist, z)
        if content is None:
            continue
        result[z] = content[0] / float(ntotal)
    return result


def normalize_factor(yield_by_z, z, thickness):
    if yield_by_z[z] == 0:
        return 0.
    return EXP_YIELD[z][thickness] / yield_by_z[z]


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
    thickness = job["THICKNESS"]
    filename = os.path.join(job["path"], "output.root")
    with uproot.open(filename) as root:
        yield_by_z = get_yield_by_z(root, thickness, ntotal)

        for z in Z_ELEMENTS:
            if z not in yield_by_z:
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

        # One macro = one thickness: single-point chart per Z, combined across
        # the macros by the portal into the yield-vs-thickness curve.
        for z in Z_ELEMENTS:
            if z not in yield_by_z:
                continue
            y = yield_by_z[z]
            err = math.sqrt(y * ntotal) / ntotal
            yield getJSON(job, "chart", mctool_name="GEANT4", mctool_model=job["MODEL"],
                          observableName="fragment yield", targetName="water",
                          beamParticle="C12", beamEnergies=[400],
                          parameters=[{"names": "Z", "values": str(z)}],
                          title="fragment yield", xAxisName="thickness of water, mm", yAxisName="N/N_{0}",
                          xValues=[thickness], yValues=[y], yStatErrorsPlus=[err], yStatErrorsMinus=[err])

        xvalues = sorted(yield_by_z)
        yvalues = [yield_by_z[z] for z in xvalues]
        yerr = [math.sqrt(y * ntotal) / ntotal for y in yvalues]
        if xvalues:
            yield getJSON(job, "histogram", mctool_name="GEANT4", mctool_model=job["MODEL"],
                          observableName="fragment yield by Z", targetName="water",
                          beamParticle="C12", beamEnergies=[400],
                          parameters=[{"names": "thickness", "values": "%s mm" % thickness}],
                          title="fragment yield by Z", xAxisName="Z", yAxisName="N/N_{0}",
                          binEdgeLow=[x - 0.5 for x in xvalues], binEdgeHigh=[x + 0.5 for x in xvalues],
                          binContent=yvalues, yStatErrorsPlus=yerr, yStatErrorsMinus=yerr)

        for z in yield_by_z:
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
