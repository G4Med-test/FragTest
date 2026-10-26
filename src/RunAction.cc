
#include "RunAction.hh"
#include "G4Run.hh"
#include "AnalysisManager.hh"
#include "DetectorConstruction.hh"
#include "G4SystemOfUnits.hh"

RunAction::RunAction(DetectorConstruction* det)
  :detector(det)
{}

RunAction::~RunAction()
{}

void RunAction::BeginOfRunAction(const G4Run* aRun)
{
  //get total number of events
  G4int NumberOfEvents = aRun->GetNumberOfEventToBeProcessed();
  G4cout << "Run: NumberOfEvents: " << NumberOfEvents << G4endl ;

  // Book the histograms of the current phantom thickness and open output.root
  AnalysisManager::getInstance()->RegisterOutputFile(detector->GetPhantomThickness() / mm);
}

void RunAction::EndOfRunAction(const G4Run*)
{
  AnalysisManager::getInstance()->Finish();
}

