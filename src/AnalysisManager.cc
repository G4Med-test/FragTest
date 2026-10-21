#include "AnalysisManager.hh"
#include "TFile.h"
#include "TNtuple.h"
#include "TH1D.h"

#include <fstream> 
#include <sstream>

using namespace std;

AnalysisManager* AnalysisManager::instance = nullptr;

AnalysisManager::AnalysisManager() : pThickness(600.), outputFile(nullptr)
{}

AnalysisManager::~AnalysisManager() 
{}

AnalysisManager* AnalysisManager::getInstance()
{
  if (!instance) { instance = new AnalysisManager; }
  return instance;
}

void AnalysisManager::RegisterOutputFile(G4double thick)
{
  pThickness = thick;

  outputFile = new TFile("output.root", "RECREATE");

  fragNtuple = new TNtuple("ntuple101", "frag","AA:ZZ:xPos:phanThickness:zPos:KE:Gtime:thetaDeg");
  
  G4int nDbpBins = 3000;
  G4int nZBins = 5;
  std::ostringstream name, title;
  
  name << "hist_z_thick_" << (int) thick;
  title << "Fragment Z for thickness=" << (int) thick;

  fBraggTot = new TH1D("h10", "BraggTot", nDbpBins, 0., pThickness);
  fFragmZ   = new TH1D(name.str().c_str(), title.str().c_str(), nZBins, 0.5, (G4double)nZBins + 0.5);

  for (auto it_z: ANGLE_EXP_POINTS.at(thick))
  {
    std::ostringstream hname, htitle;
    hname << "hist_theta_thick_" << (int) thick << "_z_" << (int) it_z.first;
    htitle << "Fragment theta for thickness="<< (int) thick << " and Z=" << (int) it_z.first;
    
    mapAngle[it_z.first] = new TH1D(hname.str().c_str(), htitle.str().c_str(), 
                                    it_z.second.size() / 2, -0.5, (G4double)(it_z.second.size() / 2) - 0.5);
  }

  for (auto it_z: ENERGY_EXP_POINTS.at(thick))
  {
    for (auto it_a: it_z.second)
    {
      std::ostringstream hname, htitle;
      hname << "hist_energy_thick_" << (int) thick << "_z_" << (int) it_z.first << "_theta_" << (int) it_a.first;
      htitle << "Fragment energy for thickness="<< (int) thick << " and Z=" << (int) it_z.first;
      htitle << " and abs(theta)=" << (int) it_a.first;
      
      mapEnergy[it_z.first][it_a.first] = new TH1D(hname.str().c_str(), htitle.str().c_str(), 
                                                   it_a.second.size()-1, &(it_a.second.at(0)));
    }
  }
}

//////////////////////////////////////////////

void AnalysisManager::FillZ(G4double Z)
{
  fFragmZ->Fill(Z, 1.0);
}

//////////////////////////////////////////////

void AnalysisManager::Finish() 
{   
  fFragmZ->Print("all");
  if(outputFile) { 
    outputFile->Write(); 
    outputFile->Close(); 
  } 
}

//////////////////////////////////////////////

void AnalysisManager::FillBraggTot(G4double disY, G4double edep)
{
  fBraggTot->Fill(disY, edep);
}

///////////////////////////////////////////////

std::pair<G4double, G4double> AnalysisManager::getEnergyBin(G4double energy, const std::vector<G4double>& energies)
{
  G4double binWidthRight, binWidthLeft;
  size_t energy_id = (size_t) (std::find(energies.cbegin(), energies.cend(), energy) - energies.begin());
  if (energy_id == 0) 
  {
    binWidthRight = (energies[energy_id + 1] - energies[energy_id]) / 2.;
    binWidthLeft = binWidthRight;
  }
  else 
  {
    if(energy_id == energies.size())
    {
      binWidthLeft = (energies[energy_id] - energies[energy_id - 1]) / 2.;
      binWidthRight = binWidthLeft;
    }
    else
    {
      binWidthRight = (energies[energy_id + 1] - energies[energy_id]) / 2.;
      binWidthLeft = (energies[energy_id] - energies[energy_id - 1]) / 2.;
    }
  }

  return std::make_pair(binWidthLeft, binWidthRight);
}

//////////////////////////////////////////////

void AnalysisManager::FillAngle(G4double Z, G4double thetaDeg)
{
  for (size_t ia = 0; ia < ANGLE_EXP_POINTS.at(pThickness).at(Z).size(); ia += 2)
  {
    if ((thetaDeg >= ANGLE_EXP_POINTS.at(pThickness).at(Z).at(ia)) && (thetaDeg <= ANGLE_EXP_POINTS.at(pThickness).at(Z).at(ia+1)))
    {
      mapAngle[Z]->Fill(ia/2);
    }
  }
}

//////////////////////////////////////////////

void AnalysisManager::FillEnergy(G4double Z, G4double thetaDeg, G4double en)
{
  const G4double angleWidth = 0.39;
  G4double theta_bin;
  
  for (auto it_a: ENERGY_EXP_POINTS.at(pThickness).at(Z))
  {
    theta_bin = it_a.first;
    if ((thetaDeg >= theta_bin - angleWidth) && (thetaDeg <= theta_bin + angleWidth))
    {      
        mapEnergy[Z][theta_bin]->Fill(en);
        return;
    }
  }
}
