#include "G4RunManager.hh"
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "PrimaryGeneratorAction.hh"
//
#include "RunAction.hh"
#include "SteppingAction.hh"
#include "EventAction.hh"

#include "AnalysisManager.hh"
#include "Randomize.hh"

#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"

int main(int argc, char** argv)
{
  //detect interactive mode (if no arguments) and define UI session
  G4UIExecutive* ui = nullptr;
  if (argc == 1) { ui = new G4UIExecutive(argc,argv); }

  if (argc > 2) {
    G4int seed = atoi(argv[2]);
    G4Random::setTheSeed(seed);
    G4cout << "***********************" << G4endl;
    G4cout << "*** " << seed << "  ***" << G4endl;
    G4cout << "***********************" << G4endl;
  }

  G4double y = (argc > 3) ? atof(argv[3])*CLHEP::mm : 30*CLHEP::cm;
  DetectorConstruction* detector = new DetectorConstruction(y);

  G4RunManager* rm = new G4RunManager();
  rm->SetUserInitialization(detector);

  // Instantiate the analysis manager
  AnalysisManager* analysis = AnalysisManager::getInstance();

  // Create ROOT file, histograms and ntuple
  analysis->RegisterOutputFile(y);

  G4VUserPhysicsList* physics = new PhysicsList();
  rm->SetUserInitialization(physics);

  G4VUserPrimaryGeneratorAction* pga = new PrimaryGeneratorAction();
  rm->SetUserAction(pga);

  G4UserRunAction* ra = new RunAction();
  rm->SetUserAction(ra);

  EventAction* event = new EventAction();
  rm->SetUserAction(event);

  rm->SetUserAction(new SteppingAction(event, y));

  //initialize visualization
  G4VisManager* visManager = nullptr;

  //get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  if (ui)  {
    //interactive mode
    visManager = new G4VisExecutive;
    visManager->Initialize();
    ui->SessionStart();
    delete ui;
  } else  {
    //batch mode
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command+fileName);
  }
  analysis->Finish();

  //job termination
  delete visManager;
  delete rm;
}

