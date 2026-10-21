#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class EventAction;
class G4Step;
class AnalysisManager;

class SteppingAction : public G4UserSteppingAction
{
public:
  SteppingAction(EventAction*, G4double phanThickness);
  ~SteppingAction();

  void UserSteppingAction(const G4Step* aStep);

private:
  EventAction* event;
  AnalysisManager* analysis;
  G4double phantomThickness;
};
#endif

