#ifndef ANALYSISMANAGER_HH
#define ANALYSISMANAGER_HH 

#include "globals.hh"

#include <map>
#include <vector>

using namespace std;

class TFile;
class TNtuple;
class TH1D;

class AnalysisManager { 

public:
  
  ~AnalysisManager();
  static AnalysisManager* getInstance();
 
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

  TFile* outputFile;	
  TNtuple *fragNtuple, *fragNtupleA, *fragNtupleE;

  //-----1D Histograms--------------------------------
  TH1D* fFragmZ;
  TH1D* fBraggTot;

  //-----Binning for 1D histograms--------------------
  std::pair<G4double, G4double> getEnergyBin(G4double energy, const std::vector<G4double>& energies);

  std::map<G4double, TH1D*> mapAngle; // Z
  std::map<G4double, std::map<G4double, TH1D*>> mapEnergy; // Z, thetaDeg

  #include "Binning.hh"

};

#endif



