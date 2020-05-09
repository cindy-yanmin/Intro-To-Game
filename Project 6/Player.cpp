#include "Player.h"
#include "Bullet.h"

Player::Player(GLuint id, float w, float h) : Entity(id, w, h) {
	animUp = new int[1]{ 0 };
	animDown = new int[1]{ 1 };
	animRight = new int[1]{ 2 };
	animLeft = new int[1]{ 3 };
	animIndices = animLeft;
	animFrames = 1;
	animIndex = 0;
	animTime = 0;
	animCols = 2;
	animRows = 2;
    
	speed = 25.0f;
	shootDirection = 3;
    entityType = PLAYER;
}

void Player::Update(float deltaTime, Map* map, const std::vector<Entity*> objects, Entity* target)
{
	Entity::Update(deltaTime, map, objects, target);
    movement = glm::vec3(0);
}
