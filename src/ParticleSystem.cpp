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
	mParticles.push_back(Particle(location, mOrigin));
}

void ParticleSystem::run(ci::vec2 newCentroid)
{
	float diffY = mOrigin.y - newCentroid.y;
	float diffX = mOrigin.x - newCentroid.x;
	vec2 diff = vec2(diffY, diffX);
	mOrigin = newCentroid;
	
	for (vector<Particle>::iterator it = mParticles.begin(); it != mParticles.end(); ++it) {
		it->run(diff);
	}

}

void ParticleSystem::reverse()
{
	for (vector<Particle>::iterator it = mParticles.begin(); it != mParticles.end(); ++it) {
		it->reverseDirection();
	}
}