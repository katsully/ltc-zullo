#include "cinder/app/App.h"
#include "Particle.h"
#include "cinder\Rand.h"
#include "cinder/gl/gl.h"
#include "CinderOpenCV.h"

using namespace ci;
using namespace ci::app;
using namespace std;

Particle::Particle()
{
}

Particle::Particle(ci::vec2 location, ci::vec2 origin)
{
	mLocation = location;
	mOriginalLocation = origin;

	// get distance between point and centroid of the shape
	float diffY = mLocation.y - origin.y;
	float diffX = mLocation.x - origin.x;
	dist = vec2(diffX, diffY);
	mAcceleration = vec2(0, 0);
	mVelocity = vec2(diffX/15, diffY/15);
}

void Particle::run(ci::vec2 newCentroid)
{
	update(newCentroid);
	//display();
}

// Method to update location
void Particle::update(ci::vec2 offset)
{
	mVelocity += mAcceleration;
	mLocation += mVelocity;
	mLocation += offset;
}

void Particle::display()
{
	gl::color(Color(1.0, 1.0, 1.0));
	gl::drawSolidEllipse(mLocation, 12.0, 12.0);
}

void Particle::reverseDirection()
{
	mVelocity *= -1;
}