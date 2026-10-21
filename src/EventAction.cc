#include "EventAction.hh"
#include "G4Event.hh"

EventAction::EventAction():evtID(0)
{}

EventAction::~EventAction()
{}

void EventAction::BeginOfEventAction(const G4Event* evt)
{
  evtID = evt->GetEventID();
}

void EventAction::EndOfEventAction(const G4Event*)
{}
