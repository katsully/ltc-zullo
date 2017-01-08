#include "cinder/app/App.h"
#include "ParticleSystem.h"
#include "CinderOpenCV.h"
#include "boost/range/combine.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

ParticleSystem::ParticleSystem()
{
}

ParticleSystem::ParticleSystem(ci::vec2 location, cinder::Color c)
{
	mOrigin = location;
	mParticles = vector<Particle>();
	color = c;
}

void ParticleSystem::addParticle(ci::vec2 location)
{
	mParticles.push_back(Particle(location, mOrigin));
}

void ParticleSystem::run(cv::vector<cv::Point> newPoints)
{

	if (!reversing) {

		for (vector<Particle>::iterator it = mParticles.begin(); it != mParticles.end(); ++it) {
			it->run();
		}
	}
	else {

		std::vector<Particle>::iterator it1 = mParticles.begin();
		std::vector<cv::Point>::iterator it2 = newPoints.begin();
		for (; it1 != mParticles.end() && it2 != newPoints.end(); ++it1, ++it2)
		{	
			//run some code
			float diffY = it1->mLocation.y - it2->y;
			float diffX = it1->mLocation.x - it2->x;
			vec2 diff = vec2(-diffX / 20, -diffY / 20);
			it1->run(diff);
		}
	}

}

void ParticleSystem::reverse()
{
	for (vector<Particle>::iterator it = mParticles.begin(); it != mParticles.end(); ++it) {
		it->reverseDirection();
	}
}