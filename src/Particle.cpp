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

Particle::Particle(ci::vec2 location)
{
	// fill all variables
	//mAcceleration = vec2(0.0, 0.05);
	//mVelocity = vec2(randFloat(-1, 1), randFloat(-1.0, 0.0) );
	mLocation = location;
}

void Particle::run()
{
	update();
	display();
}

// Method to update location
void Particle::update()
{

}

void Particle::display()
{
	gl::color(Color(1.0, 1.0, 1.0));
	gl::drawSolidEllipse(mLocation, 12.0, 12.0);
}