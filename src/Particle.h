#include "cinder/app/App.h"

class Particle {
public:
	Particle();
	Particle(ci::vec2 location);

	void run();
	void update();
	void display();

	ci::vec2 mLocation;


private:
	ci::vec2 mAcceleration;
	ci::vec2 mVelocity;
};