#ifndef DetectorConstruction_H 
#define DetectorConstruction_H 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4VPhysicalVolume;

class DetectorConstruction : public G4VUserDetectorConstruction{

public:
  DetectorConstruction();
  DetectorConstruction(G4double fooPhantomThickness);
  ~DetectorConstruction() override;

  G4VPhysicalVolume* Construct() override;

private:
  G4VPhysicalVolume* physicalWorld;
  G4VPhysicalVolume* physicalPhantom;
  G4double phantomThickness;

};
#endif
