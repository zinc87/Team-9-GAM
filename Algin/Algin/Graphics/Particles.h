#pragma once
#include "../Header/pch.h"

struct ParticleInstance {

	glm::vec3 pos;
	glm::vec4 color;
	float size;
	float rot;
};

struct SimParticle {
	glm::vec3 pos;
	glm::vec3 vel;
	float age;
	float lifetime;
};

class Particle : public AG::Pattern::ISingleton<Particle> {

public:

	void init();


	void createParticleQuad();
	void createInstanceVBO();

	void updateTest(float t);
	void drawParticles();
	inline float frand01() { return float(rand()) / float(RAND_MAX); }
	void emitter();

	float timeAccum = 0.f;

	glm::vec3 emitterPosition{};
	float speedControl{};
	float upVelControl{};
	float gravControl{};
private:
	GLuint particleQuadVAO{};
	GLuint particleQuadVBO{};
	GLuint particleQuadEBO{};

	GLuint particleInstanceVBO{};

	size_t maxParticles = 10000;
	size_t numAlive = 500;

	std::vector<ParticleInstance> particles;
	std::vector<SimParticle> pool;
	std::vector<int> alive;
	size_t nextFree = 0;
	float spawnAccumulator = 0.f;
	float spawnRate = 200.f;

	
};

