#include "G4RunManager.hh"
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "PrimaryGeneratorAction.hh"
//
#include "RunAction.hh"
#include "SteppingAction.hh"
#include "EventAction.hh"

#include "Randomize.hh"

#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"

int main(int argc, char** argv)
{
  //detect interactive mode (if no arguments) and define UI session
  G4UIExecutive* ui = nullptr;
  if (argc == 1) { ui = new G4UIExecutive(argc,argv); }

  // All input parameters come from the macro: the phantom thickness with
  // /FragTest/phantomThickness (before /run/initialize, default 288 mm) and,
  // if needed, the random seed with /random/setSeeds.
  DetectorConstruction* detector = new DetectorConstruction(288*CLHEP::mm);

  G4RunManager* rm = new G4RunManager();
  rm->SetUserInitialization(detector);

  G4VUserPhysicsList* physics = new PhysicsList();
  rm->SetUserInitialization(physics);

  G4VUserPrimaryGeneratorAction* pga = new PrimaryGeneratorAction();
  rm->SetUserAction(pga);

  G4UserRunAction* ra = new RunAction(detector);
  rm->SetUserAction(ra);

  EventAction* event = new EventAction();
  rm->SetUserAction(event);

  rm->SetUserAction(new SteppingAction(event, detector));

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

  //job termination
  delete visManager;
  delete rm;
}

