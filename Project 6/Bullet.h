#pragma once
#include "Enemy.h"
#include "Player.h"
#include <SDL_mixer.h>

class Bullet : public Entity {
private:
	Mix_Chunk* hurt;
public:
    Bullet(GLuint id);
	glm::vec3 setBulletMovement(int shootDirection);
    void Update(float deltaTime, Map* map, const std::vector<Entity*> objects = {}, Entity* target = nullptr) override;
};
