#include "cinder/app/App.h"

class Particle {
public:
	Particle();
	Particle(ci::vec2 location, ci::vec2 origin);

	void run(ci::vec2 offset);
	void update(ci::vec2 newCentroid);
	void display();
	void reverseDirection();

	ci::vec2 mLocation;
	ci::vec2 dist;


private:
	ci::vec2 mAcceleration;
	ci::vec2 mVelocity;
	ci::vec2 mOriginalLocation;
};