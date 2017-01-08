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
	//mAcceleration = vec2(0.01, 0.01);
	mVelocity = vec2(diffX/25, diffY/25);
}

void Particle::run(vec2 offset)
{
	update(offset);
	//display();
}

void Particle::run()
{
	update();
	//display();
}

// Method to update location
void Particle::update()
{
	mVelocity += mAcceleration;
	mLocation += mVelocity;
	//mLocation += offset;
}

void Particle::update(vec2 offset)
{
	mVelocity = offset;
	mLocation += mVelocity;
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