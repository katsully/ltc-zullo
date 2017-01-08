#include "cinder/app/App.h"
#include "Particle.h"
#include "CinderOpenCV.h"

class ParticleSystem {
public:
	ParticleSystem();
	ParticleSystem(ci::vec2 location);

	std::vector<Particle> mParticles;

	void addParticle(ci::vec2 location);
	void run(cv::vector<cv::Point> newPoints);
	void reverse();

	ci::vec2 mOrigin;
	bool reversing = false;
	
};