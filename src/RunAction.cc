
#include "RunAction.hh"
#include "G4Run.hh"
#include "AnalysisManager.hh"

RunAction::RunAction()
{}

RunAction::~RunAction()
{}

void RunAction::BeginOfRunAction(const G4Run* aRun)
{
  //get total number of events
  G4int NumberOfEvents = aRun->GetNumberOfEventToBeProcessed();
  G4cout << "Run: NumberOfEvents: " << NumberOfEvents << G4endl ;
}

void RunAction::EndOfRunAction(const G4Run*)
{}

