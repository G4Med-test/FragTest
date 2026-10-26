#include "AnalysisManager.hh"
#include "G4AnalysisManager.hh"
#include "G4Exception.hh"

#include <fstream> 
#include <sstream>

using namespace std;

AnalysisManager* AnalysisManager::instance = nullptr;

AnalysisManager::AnalysisManager() : pThickness(600.), booked(false), fFragmZ(-1), fBraggTot(-1)
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
  if (booked && thick != pThickness)
  {
    G4Exception("AnalysisManager::RegisterOutputFile", "FragTest001", FatalException,
                "The phantom thickness cannot change after the histograms are booked.");
  }
  if (ANGLE_EXP_POINTS.find(thick) == ANGLE_EXP_POINTS.end())
  {
    std::ostringstream msg;
    msg << "Phantom thickness " << thick << " mm has no experimental binning; use one of:";
    for (const auto& it_t: ANGLE_EXP_POINTS) { msg << " " << it_t.first; }
    msg << " mm (/FragTest/phantomThickness).";
    G4Exception("AnalysisManager::RegisterOutputFile", "FragTest002", FatalException, msg.str().c_str());
  }

  pThickness = thick;

  auto g4analysis = G4AnalysisManager::Instance();
  g4analysis->SetDefaultFileType("root");
  g4analysis->SetVerboseLevel(1);

  if (!booked)
  {
    G4int nDbpBins = 3000;
    G4int nZBins = 5;
    std::ostringstream name, title;

    name << "hist_z_thick_" << (int) thick;
    title << "Fragment Z for thickness=" << (int) thick;

    fBraggTot = g4analysis->CreateH1("h10", "BraggTot", nDbpBins, 0., pThickness);
    fFragmZ   = g4analysis->CreateH1(name.str(), title.str(), nZBins, 0.5, (G4double)nZBins + 0.5);

    for (auto it_z: ANGLE_EXP_POINTS.at(thick))
    {
      std::ostringstream hname, htitle;
      hname << "hist_theta_thick_" << (int) thick << "_z_" << (int) it_z.first;
      htitle << "Fragment theta for thickness="<< (int) thick << " and Z=" << (int) it_z.first;

      mapAngle[it_z.first] = g4analysis->CreateH1(hname.str(), htitle.str(),
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

        mapEnergy[it_z.first][it_a.first] = g4analysis->CreateH1(hname.str(), htitle.str(), it_a.second);
      }
    }
    booked = true;
  }

  g4analysis->OpenFile("output.root");
}

//////////////////////////////////////////////

void AnalysisManager::FillZ(G4double Z)
{
  G4AnalysisManager::Instance()->FillH1(fFragmZ, Z, 1.0);
}

//////////////////////////////////////////////

void AnalysisManager::Finish() 
{   
  auto g4analysis = G4AnalysisManager::Instance();
  if (!g4analysis->IsOpenFile()) { return; }

  auto hZ = g4analysis->GetH1(fFragmZ);
  for (G4int ib = 0; ib < (G4int) hZ->axis().bins(); ++ib)
  {
    G4cout << "Fragments Z=" << ib + 1 << ": " << hZ->bin_entries(ib) << G4endl;
  }

  g4analysis->Write();
  g4analysis->CloseFile();
}

//////////////////////////////////////////////

void AnalysisManager::FillBraggTot(G4double disY, G4double edep)
{
  G4AnalysisManager::Instance()->FillH1(fBraggTot, disY, edep);
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
      G4AnalysisManager::Instance()->FillH1(mapAngle[Z], ia/2);
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
        G4AnalysisManager::Instance()->FillH1(mapEnergy[Z][theta_bin], en);
        return;
    }
  }
}
