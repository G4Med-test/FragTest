#ifndef DetectorMessenger_h
#define DetectorMessenger_h 1

#include "globals.hh"
#include "G4UImessenger.hh"

class DetectorConstruction;
class G4UIdirectory;
class G4UIcmdWithADoubleAndUnit;

class DetectorMessenger: public G4UImessenger
{
  public:

    DetectorMessenger(DetectorConstruction*);
   ~DetectorMessenger() override;

    void SetNewValue(G4UIcommand*, G4String) override;

  private:

  DetectorConstruction* pDetector;

  G4UIdirectory*             testDir;
  G4UIcmdWithADoubleAndUnit* thicknessCmd;
};

#endif
