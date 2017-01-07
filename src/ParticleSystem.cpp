#include "cinder/app/App.h"
#include "ParticleSystem.h"

using namespace ci;
using namespace ci::app;
using namespace std;

ParticleSystem::ParticleSystem()
{

}

ParticleSystem::ParticleSystem(ci::vec2 location)
{
	mOrigin = location;
	mParticles = vector<Particle>();
}

void ParticleSystem::addParticle(ci::vec2 location)
{
	mParticles.push_back(Particle(location));
}

void ParticleSystem::run()
{

}