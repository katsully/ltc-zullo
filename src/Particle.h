#include "cinder/app/App.h"

class Particle {
public:
	Particle();
	Particle(ci::vec2 location, ci::vec2 origin);

	void run();
	void update();
	void display();

	ci::vec2 mLocation;
	ci::vec2 dist;


private:
	ci::vec2 mAcceleration;
	ci::vec2 mVelocity;
};