#include "PrimaryGeneratorAction.hh"
#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"

#include "G4ParticleDefinition.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
  gun  = new G4GeneralParticleSource();  
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete gun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  gun->GeneratePrimaryVertex(anEvent);
}



