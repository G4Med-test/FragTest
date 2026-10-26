#include "DetectorMessenger.hh"

#include "DetectorConstruction.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

/////////////////////////////////////////////////////////////////////////////
DetectorMessenger::DetectorMessenger(DetectorConstruction* pDet)
:pDetector(pDet)
{
  testDir = new G4UIdirectory("/FragTest/");
  testDir->SetGuidance("FragTest geometry commands");

  thicknessCmd = new G4UIcmdWithADoubleAndUnit("/FragTest/phantomThickness",this);
  thicknessCmd->SetGuidance("Set the thickness of the water phantom.");
  thicknessCmd->SetGuidance("Must be one of the experimental depths (59, 159, 258, 279, 288, 312, 347 mm)");
  thicknessCmd->SetGuidance("and must be given before /run/initialize.");
  thicknessCmd->SetParameterName("thickness",false);
  thicknessCmd->SetUnitCategory("Length");
  thicknessCmd->SetDefaultUnit("mm");
  thicknessCmd->SetRange("thickness>0.0");
  // The geometry is built once by /run/initialize: changing it afterwards
  // would not move the phantom, only the histogram booking.
  thicknessCmd->AvailableForStates(G4State_PreInit);
}

/////////////////////////////////////////////////////////////////////////////
DetectorMessenger::~DetectorMessenger()
{
  delete thicknessCmd;
  delete testDir;
}

/////////////////////////////////////////////////////////////////////////////
void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
  if( command == thicknessCmd )
   { pDetector->SetPhantomThickness(thicknessCmd->GetNewDoubleValue(newValue));}
}
