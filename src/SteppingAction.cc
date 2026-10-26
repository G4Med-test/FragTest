#include "SteppingAction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4ParticleDefinition.hh"
#include "G4VPhysicalVolume.hh"

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "EventAction.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

#include "AnalysisManager.hh"
#include "DetectorConstruction.hh"

#include <cmath>

#include "Randomize.hh"

using namespace std;

SteppingAction::SteppingAction(EventAction* eventActionPointer, 
			       DetectorConstruction* det)
  :event(eventActionPointer),
   analysis(AnalysisManager::getInstance()),
   detector(det)
{}

SteppingAction::~SteppingAction()
{}

void SteppingAction::UserSteppingAction(const G4Step* aStep)
{

  if(aStep->GetTrack()->GetNextVolume()) {
    const G4VPhysicalVolume* preVol = aStep->GetPreStepPoint()->GetPhysicalVolume();
    const G4VPhysicalVolume* postVol= aStep->GetPostStepPoint()->GetPhysicalVolume();
    if(preVol != postVol) {
      const G4ParticleDefinition *def = aStep->GetTrack()->GetDefinition();
      G4int A = def->GetBaryonNumber();
      G4double pdgZ = def->GetPDGCharge();
      int Z = (int) pdgZ;
      G4double disX = aStep->GetTrack()->GetPosition().x() / mm;
      G4double disY = aStep->GetTrack()->GetPosition().y() / mm;
      G4double disZ = aStep->GetTrack()->GetPosition().z() / mm;
      //G4double KE = aStep->GetTrack()-> GetKineticEnergy()/MeV;
      G4double asin_value = std::min(std::sqrt(disX*disX + disZ*disZ) / 2940.0, 1.0);      
      G4double thetaDeg = std::asin(asin_value) / deg;      
      G4double thetaDegSigned = (disX / std::abs(disX)) * thetaDeg;
      if(A > 0 && pdgZ > 0. && pdgZ <= 5. && postVol->GetName() == "physRecVol" && thetaDeg < 10.4) {
        //G4cout << "--- Z= " << Z << "  A= " << A << G4endl;
	      G4double globalTime = aStep->GetTrack()->GetGlobalTime()/ns;
	      //analysis->FillFragNtuple((G4double)A, pdgZ, disX, disY, disZ, phantomThickness, KE, globalTime, thetaDeg);
        analysis->FillZ(pdgZ);

        //Fill ntuple for angle distribution
        analysis->FillAngle(pdgZ, thetaDegSigned);

        //Fill energy distribution
        const G4double timeSigma = 0.53 / 2.355;
        const G4double lightSpeedNano = 3e8 * 1e-9;
        const G4double radius = 2.94;
        const G4double fragTimes[] = {4.94827, 4.95578, 4.97499, 4.98133, 4.98405, 4.99258, 5.00771};
        const G4double restMasses[] = {938.28, 3727.38, 6535.36563, 8394.793646, 10255.10192};

        G4double GtimeSigmaed = G4RandGauss::shoot(globalTime, timeSigma);
        G4double beta = radius / ((GtimeSigmaed - fragTimes[Z - 1]) * lightSpeedNano);
        if (beta >= 1)
        {
          std::cerr << "Avoid MathError: 1.0 / sqrt(1 - " << pow(beta, 2) << "). Skip this event." << std::endl;
        }
        else
        {
          G4double energyPerNuc = (1. / std::sqrt(1 - pow(beta, 2)) - 1) * restMasses[Z - 1];
          analysis->FillEnergy(pdgZ, thetaDeg, energyPerNuc);
        }
      }
      if(A > 0 && disY > 0.0 && preVol->GetName() == "physcialPhantom") 
      {
	      // analysis->FillZ(pdgZ);
      }
    } else if(preVol->GetName() == "physcialPhantom") {
      analysis->FillBraggTot(0.5*(aStep->GetPreStepPoint()->GetPosition().y() 
				  + aStep->GetPostStepPoint()->GetPosition().y() 
				  - detector->GetPhantomThickness()), 
			     aStep->GetTotalEnergyDeposit());
    }
  }
}
