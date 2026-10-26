#ifndef DetectorConstruction_H 
#define DetectorConstruction_H 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class DetectorMessenger;

class DetectorConstruction : public G4VUserDetectorConstruction{

public:
  DetectorConstruction();
  DetectorConstruction(G4double fooPhantomThickness);
  ~DetectorConstruction() override;

  G4VPhysicalVolume* Construct() override;

  void SetPhantomThickness(G4double thickness) { phantomThickness = thickness; }
  G4double GetPhantomThickness() const { return phantomThickness; }

private:
  G4VPhysicalVolume* physicalWorld;
  G4VPhysicalVolume* physicalPhantom;
  G4double phantomThickness;
  DetectorMessenger* messenger;

};
#endif
