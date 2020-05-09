#pragma once
#include "Entity.h"
#include "Player.h"
#include "Bullet.h"


class Enemy : public Entity {
public:
    time_t cooldown;
    time_t lastShotTime;
    
    Enemy(GLuint id, float w, float h);
    void Update(float deltaTime, Map* map, const std::vector<Entity*> objects = {}, Entity* target = nullptr) override;
    void AIChaser(Entity* target);
    void AIShooter(Entity* target);
};

