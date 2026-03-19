#include "Particles.h"
#include "pch.h"

void Particle::init() {

    createParticleQuad();
    createInstanceVBO();
    
    particles.reserve(maxParticles);
    glNamedBufferData(particleInstanceVBO, maxParticles * sizeof(ParticleInstance), nullptr, GL_STREAM_DRAW);
    
    pool.resize(maxParticles);
    alive.clear();
    nextFree = 0;
    spawnAccumulator = 0.f;

}

void Particle::createParticleQuad() {
    std::vector<float> quadVerts = {
        // x,     y,   ,z     u,    v
        -0.5f, -0.5f, 0.f, 0.0f, 0.0f,  // vertex 0
         0.5f, -0.5f, 0.f, 1.0f, 0.0f,  // vertex 1
         0.5f,  0.5f, 0.f, 1.0f, 1.0f,  // vertex 2
        -0.5f,  0.5f, 0.f, 0.0f, 1.0f   // vertex 3
    };

    std::vector<GLuint> indices = {
        0, 1, 2,
        0, 2, 3
    };


    glCreateVertexArrays(1, &particleQuadVAO);
    glCreateBuffers(1, &particleQuadVBO);

    glNamedBufferData(particleQuadVBO, quadVerts.size() * sizeof(float), quadVerts.data(), GL_STATIC_DRAW);

    glVertexArrayVertexBuffer(particleQuadVAO, 0, particleQuadVBO, 0, 5 * sizeof(float));

    //position
    glEnableVertexArrayAttrib(particleQuadVAO, 0);
    glVertexArrayAttribFormat(particleQuadVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(particleQuadVAO, 0, 0);

    //uv
    glEnableVertexArrayAttrib(particleQuadVAO, 1);
    glVertexArrayAttribFormat(particleQuadVAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glVertexArrayAttribBinding(particleQuadVAO, 1, 0);

    glCreateBuffers(1, &particleQuadEBO);
    glNamedBufferData(particleQuadEBO, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    glVertexArrayElementBuffer(particleQuadVAO, particleQuadEBO);
}

void Particle::createInstanceVBO() {

    glCreateBuffers(1, &particleInstanceVBO);


    glNamedBufferData(particleInstanceVBO, maxParticles * sizeof(ParticleInstance), nullptr, GL_STREAM_DRAW);

    glVertexArrayVertexBuffer(particleQuadVAO, 1, particleInstanceVBO, 0, sizeof(ParticleInstance));

    //pos
    glEnableVertexArrayAttrib(particleQuadVAO, 2);
    glVertexArrayAttribFormat(particleQuadVAO, 2, 3, GL_FLOAT, GL_FALSE, offsetof(ParticleInstance, pos));
    glVertexArrayAttribBinding(particleQuadVAO, 2, 1);

    //color
    glEnableVertexArrayAttrib(particleQuadVAO, 3);
    glVertexArrayAttribFormat(particleQuadVAO, 3, 4, GL_FLOAT, GL_FALSE, offsetof(ParticleInstance, color));
    glVertexArrayAttribBinding(particleQuadVAO, 3, 1);

    //size
    glEnableVertexArrayAttrib(particleQuadVAO, 4);
    glVertexArrayAttribFormat(particleQuadVAO, 4, 1, GL_FLOAT, GL_FALSE, offsetof(ParticleInstance, size));
    glVertexArrayAttribBinding(particleQuadVAO, 4, 1);

    //rot
    glEnableVertexArrayAttrib(particleQuadVAO, 5);
    glVertexArrayAttribFormat(particleQuadVAO, 5, 1, GL_FLOAT, GL_FALSE, offsetof(ParticleInstance, rot));
    glVertexArrayAttribBinding(particleQuadVAO, 5, 1);

    glVertexArrayBindingDivisor(particleQuadVAO, 1, 1);
}


void Particle::emitter() {
    SimParticle p{};

    //emitter position
    p.pos = emitterPosition;

    float angle = frand01() * 6.2831853f;
    float speed = speedControl + frand01() * 2.0f;
    p.vel = glm::vec3(std::cos(angle), upVelControl, std::sin(angle)) * speed;

    p.age = 0.f;
    p.lifetime = 2.0f + frand01() * 1.0f; // 2�3s
    pool[nextFree] = p;
    alive.push_back((int)nextFree);
    nextFree = (nextFree + 1) % pool.size();
}

void Particle::updateTest(float t) {

    // spawn
    spawnAccumulator += t * spawnRate;
    while (spawnAccumulator >= 1.f && alive.size() < pool.size()) {
        emitter();
        spawnAccumulator -= 1.f;
    }

    // simulate
    const glm::vec3 gravity(0.f, -gravControl, 0.f);
    size_t w = 0; // write cursor for compacting "alive"
    for (size_t r = 0; r < alive.size(); ++r) {
        int i = alive[r];
        SimParticle& p = pool[i];

        p.age += t;
        if (p.age >= p.lifetime) continue; // drop (don�t advance w)

        p.vel += gravity * t * 0.3f;      // gentle gravity
        p.pos += p.vel * t;

        alive[w++] = i; // keep
    }
    alive.resize(w);

    // build instances for rendering
    particles.resize(alive.size());
    for (size_t k = 0; k < alive.size(); ++k) {
        const SimParticle& s = pool[alive[k]];
        float life = s.age / s.lifetime;           // 0..1
        particles[k].pos = s.pos;
        particles[k].size = glm::mix(0.25f, 0.05f, life); // shrink over life
        // fade from orange to red and alpha out
        particles[k].color = glm::vec4(
            glm::mix(1.0f, 1.0f, life),
            glm::mix(0.7f, 0.2f, life),
            0.2f,
            glm::smoothstep(1.0f, 0.0f, life)     // fade alpha
        );
        particles[k].rot = 0.0f; // add spin later
    }
}

void Particle::drawParticles() {

    auto camera = CAMERAMANAGER.getCurrentCamera().lock();
    if (!camera || RenderPipeline::GetInstance().GetPipeline() != RenderPipeline::STATE::LIGHT) {

        return;
    }

    SHADERMANAGER.Use("Particle");
    GLuint prog = SHADERMANAGER.GetShaderProgram();

    
    glNamedBufferSubData(particleInstanceVBO, 0, particles.size() * sizeof(ParticleInstance), particles.data());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLint locP = glGetUniformLocation(prog, "P");
    GLint locV = glGetUniformLocation(prog, "V");

    if (locP != -1)
        glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));
    if (locV != -1)
        glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));

    glBindVertexArray(particleQuadVAO);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0, (GLsizei)particles.size());
    glBindVertexArray(0);

}
