#include "cinder/app/App.h"
#include "Particle.h"

class ParticleSystem {
public:
	ParticleSystem();
	ParticleSystem(ci::vec2 location);

	std::vector<Particle> mParticles;

	void addParticle(ci::vec2 location);
	void run();

	ci::vec2 mOrigin;
	
};