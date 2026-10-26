#ifndef ANALYSISMANAGER_HH
#define ANALYSISMANAGER_HH

#include "globals.hh"

#include <map>
#include <vector>

using namespace std;

// Thin wrapper around G4AnalysisManager: histograms are written to
// output.root by Geant4's own ROOT writer, no ROOT installation needed.
class AnalysisManager {

public:

  ~AnalysisManager();
  static AnalysisManager* getInstance();

  // Book the histograms for the given phantom thickness (one of the
  // experimental depths in Binning.hh) and open output.root
  void RegisterOutputFile(G4double thick);

  void FillAngle(G4double Z, G4double thetaDeg);
  void FillEnergy(G4double Z, G4double thetaDeg, G4double en);

  //----1D Histograms--------------------------------
  void FillZ(G4double Z);
  void FillBraggTot(G4double disY, G4double edep);
  void Finish();

private:

  static AnalysisManager* instance;
  AnalysisManager();

  G4double pThickness;
  G4bool booked;

  //-----1D Histograms (G4AnalysisManager ids)--------
  G4int fFragmZ;
  G4int fBraggTot;

  //-----Binning for 1D histograms--------------------
  std::pair<G4double, G4double> getEnergyBin(G4double energy, const std::vector<G4double>& energies);

  std::map<G4double, G4int> mapAngle; // Z
  std::map<G4double, std::map<G4double, G4int>> mapEnergy; // Z, thetaDeg

  #include "Binning.hh"

};

#endif
