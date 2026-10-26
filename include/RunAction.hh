#ifndef RunAction_hh
#define RunAction_hh 1
//
#include "G4UserRunAction.hh"
#include "G4UnitsTable.hh"

#include <fstream>
#include <vector>

class DetectorConstruction;

class RunAction : public G4UserRunAction {
//
public:
//run action class needs pointer ot the detector construction class in order to get details of the readout geometry
//accepts pointer to detector construction class

    RunAction(DetectorConstruction*);
    ~RunAction();

    //
public:
//note argument of these methods is a pointer to a G4Run object
    void BeginOfRunAction(const G4Run*);
    void EndOfRunAction(const G4Run*);

private:
    DetectorConstruction* detector;
};

#endif
