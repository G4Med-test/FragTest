#include "SteppingAction.hh"
#include "DetectorConstruction.hh"

#include "globals.hh"
#include "G4Element.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalVolume.hh"

#include "G4Box.hh"
#include "G4Sphere.hh"

#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4ChordFinder.hh"

#include "G4Colour.hh"
#include "G4VisAttributes.hh"

#include "G4NistManager.hh"


#include "G4UserLimits.hh"
#include "Randomize.hh"
#include "G4ThreeVector.hh"

#include "G4GeometryTolerance.hh"
#include "G4GeometryManager.hh"


#include "G4IonisParamMat.hh"
#include "G4RunManager.hh"
#include "G4VSensitiveDetector.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

/////////////////////////////

DetectorConstruction::DetectorConstruction(G4double fooPhantomThickness)
{
  physicalWorld = nullptr;
  physicalPhantom = nullptr;
  phantomThickness = fooPhantomThickness;
}

DetectorConstruction::DetectorConstruction():DetectorConstruction(60*CLHEP::cm)
{}

DetectorConstruction::~DetectorConstruction()
{}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  if(physicalWorld) { return physicalWorld; }
//-------------------------------------------------------------------------------------------
//----------------------------------- MATERIALS ---------------------------------------------
//-------------------------------------------------------------------------------------------

  G4NistManager * nistMan = G4NistManager::Instance();

  //Define Vacuum
  G4Material* Vacuum  = nistMan->FindOrBuildMaterial("G4_Galactic");
  //Define Water
  G4Material* Water = nistMan->FindOrBuildMaterial("G4_WATER");

  //G4Material* Water  = nistMan->BuildMaterialWithNewDensity("WATER","G4_WATER",1.0*g/cm3);
    
//-------------------------------------------------------------------------------------------
//-------------------------------------- VOLUMES --------------------------------------------
//-------------------------------------------------------------------------------------------
  G4double distanceToDetector = 294.*cm;
//-------------------------------------------------------------------------------------------
//------------------------------------ WORLD VOLUME -----------------------------------------
//-------------------------------------------------------------------------------------------
  G4double worldx = 600.*cm;
  G4double worldy = 400.*cm;
  G4double worldz = 600.*cm;

  G4Box* world = new G4Box("world_box", worldx/2., worldy/2., worldz/2.);
  G4LogicalVolume* logicalWorld = new G4LogicalVolume(world, Vacuum, "logicalWorld", 0,0,0);
  physicalWorld = new G4PVPlacement(0,G4ThreeVector(),logicalWorld, 
				    "physicalWorld", 0, false, 0);

  G4VisAttributes* worldAttributes = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0)); // white
  worldAttributes -> SetVisibility(false);
  worldAttributes -> SetForceWireframe(true);
  logicalWorld -> SetVisAttributes(worldAttributes);

//-------------------------------------------------------------------------------------------
//----------------------------------------- PHANTOM -----------------------------------------
//-------------------------------------------------------------------------------------------
  G4double phantomx = 50.*cm;
  G4double phantomy = phantomThickness;//60.*cm;
  G4double phantomz = 50.*cm;
	
  G4cout << "Phantom set to: " << phantomy/mm << "mm" << G4endl;

  G4Box* phantom = new G4Box("phantomBox", phantomx/2. , phantomy/2., phantomz/2.);
  G4LogicalVolume* logicalPhantom = 
    new G4LogicalVolume(phantom, Water, "logicalPhantom", 0,0,0);
  physicalPhantom = 
    new G4PVPlacement(0, G4ThreeVector(0,(distanceToDetector - worldy/2. ),0), 
		      logicalPhantom, "physcialPhantom", logicalWorld, false, 0, true);
	
  G4VisAttributes* phantomAtt = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0)); // blue
  phantomAtt -> SetVisibility(false);
  phantomAtt -> SetForceWireframe(true);
  logicalPhantom -> SetVisAttributes(phantomAtt);

//-------------------------------------------------------------------------------------------
//------------------------------------ Collecting Sphere ------------------------------------
//-------------------------------------------------------------------------------------------
  G4RotationMatrix* rotMat = new G4RotationMatrix;
  rotMat->rotateY(0*deg);
  rotMat->rotateX(180*deg);
  rotMat->rotateZ(0*deg);

  G4double colInnerRadius = distanceToDetector;
  G4double colOuterRadius = colInnerRadius + 1.*micrometer;
  G4double colStartPhi = 0.*deg;
  G4double colDelPhi = 180.*deg;
  G4double colStartTheta = 0.*deg;
  G4double colDelTheta = 180.*deg;

  G4Sphere* recVolBox = 
    new G4Sphere("collectSphr", colInnerRadius, colOuterRadius, colStartPhi, 
		 colDelPhi, colStartTheta, colDelTheta);
  G4LogicalVolume* logicalRecVol = 
    new G4LogicalVolume(recVolBox, Vacuum, "logicalRecVol", 0, 0, 0);

  new G4PVPlacement(rotMat, G4ThreeVector(0, (-worldy/2. +colInnerRadius ), 0), 
		    logicalRecVol, "physRecVol", logicalWorld, false, 0, true); 

  G4VisAttributes* recVolAtt = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0)); // blue
  recVolAtt -> SetVisibility(true);
  //recVolAtt -> SetForceSolid(true);
  recVolAtt ->SetForceAuxEdgeVisible(true);
  //recVolAtt ->SetForceWireframe(true);
  logicalRecVol -> SetVisAttributes(recVolAtt);

//-------------------------------------------------------------------------------------------
//----------------------------------------- Record Volumes ----------------------------------
//-------------------------------------------------------------------------------------------
/*	G4double recX = worldx;
	G4double recY = 1.*micrometer;
	G4double recZ = worldz;
	
	G4Box* recVolBox = new G4Box("recVolBox", recX/2., recY/2., recZ/2.);
	G4LogicalVolume* logicalRecVol = new G4LogicalVolume(recVolBox, Vacuum, "logicalRecVol", 0, 0, 0);
	G4VPhysicalVolume* physRecVol = new G4PVPlacement(0, G4ThreeVector(0, (-worldy/2. + recY/2.), 0), logicalRecVol, "physRecVol", logicalWorld, false, 0, true); 

	G4VisAttributes* recVolAtt = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0)); // blue
	recVolAtt -> SetVisibility(true);
	recVolAtt -> SetForceSolid(true);
	logicalRecVol -> SetVisAttributes(recVolAtt);
*/
//-------------------------------------------------------------------------------------------
//----------------------------------------- Record Volumes ----------------------------------
//-------------------------------------------------------------------------------------------
/*	G4double recX = phantomx;
	G4double recY = 1.*micrometer;
	G4double recZ = phantomz;
	
	G4Box* recVolBox = new G4Box("recVolBox", recX/2., recY/2., recZ/2.);
	G4LogicalVolume* logicalRecVol = new G4LogicalVolume(recVolBox, Water, "logicalRecVol", 0, 0, 0);
	
	const double depthArray[]={59., 159., 258., 279., 288., 312., 347.};
	const int numberOfDepths = sizeof(depthArray) / sizeof(depthArray[0]);
	//G4cout << "Hello? Number of depths: "<< numberOfDepths << G4endl;
	for (G4int depthL = 0; depthL < numberOfDepths; depthL++)
	{
		G4VPhysicalVolume* physRecVol = new G4PVPlacement(0, G4ThreeVector(0, (phantomy/2. - depthArray[depthL]*mm), 0), logicalRecVol, "physRecVol", logicalPhantom, false, depthL, true); 
		//G4cout << phantomy/2. - depthArray[depthL] << G4endl;
	}
	
	G4VisAttributes* recVolAtt = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0)); // blue
	recVolAtt -> SetVisibility(true);
	recVolAtt -> SetForceSolid(true);
	logicalRecVol -> SetVisAttributes(recVolAtt);

*/
//-------------------------------------------------------------------------------------------
//--------------------------StepLimiting For logical volumes---------------------------------
//-------------------------------------------------------------------------------------------
  G4double stepLimitValue = 100.*mm;//0.05*mm;

  //logicalWorld -> SetUserLimits(new G4UserLimits(stepLimitValue));
  logicalPhantom -> SetUserLimits(new G4UserLimits(stepLimitValue));


//-------------------------------------------------------------------------------------------
//--------------------------Set Cuts of Regions----------------------------------------------
//-------------------------------------------------------------------------------------------
  // The cut is smaller in these regions
  G4Region* aRegion = new G4Region("DetectorLog");

  logicalPhantom -> SetRegion(aRegion);
  aRegion -> AddRootLogicalVolume(logicalPhantom);

//---------------------------------------------------------

  return physicalWorld;
}



