#include <stdio.h>
#include "Enemy.h"

using namespace std;

Enemy::Enemy(GLuint id, float w, float h) : Entity(id, w, h) {
    animUp = new int[1]{ 2 };
    animDown = new int[1]{ 0 };
    animRight = new int[1]{ 1 };
    animLeft = new int[1]{ 3 };
    animIndices = animUp;
    animFrames = 1;
    animIndex = 0;
    animTime = 0;
    animCols = 2;
    animRows = 2;
    
    shootDirection = 1;
    cooldown = 500;
    lastShotTime = 450;
}

void Enemy::AIChaser(Entity* target){
    if (target == nullptr || !this->isActive) return;
    
    // movement
    if (target->position.x > position.x){
        movement.x = 1;
    }else if (target->position.x < position.x){
        movement.x = -1;
    }
    
    if (target->position.y > position.y){
        movement.y = 1;
    }else if (target->position.y < position.y){
        movement.y = -1;
    }
    
    // orientation
    float diffX = position.x - target->position.x;
    float diffY = position.y - target->position.y;
    
    if (abs(diffX) > abs(diffY)){
        if (target->position.x > position.x){
            animIndices = animRight;
            shootDirection = 2;
        }else{
            animIndices = animLeft;
            shootDirection = 3;
        }
    }else{
        if (target->position.y> position.y){
            animIndices = animUp;
            shootDirection = 0;
        }else{
            animIndices = animDown;
            shootDirection = 1;
        }
    }
    
}

void Enemy::AIShooter(Entity* target){
    if (target == nullptr || !this->isActive ) return;
    
    float diffX = position.x - target->position.x;
    float diffY = position.y - target->position.y;
    
    // chase player
    AIChaser(target);
    
	// if I get roughly linear shot, shoot if not on cd
    if (lastShotTime >= cooldown){
        if (abs(diffX) < 0.1){ //lined up vertically
            cout << "pew pew vertical" << endl;
            this->shoot = true;
            lastShotTime = 0;
        }else if (abs(diffY) < 0.1){ //lined up horizontally
            cout << "pew pew horizontal" << endl;
            this->shoot = true;
            lastShotTime = 0;
        }
        
    }else{
        lastShotTime += 1;
    }
	
}

void Enemy::Update(float deltaTime, Map* map, const std::vector<Entity*> objects, Entity* target)
{
    this->movement = glm::vec3(0);
    switch (entityType){;
        case CHASER:
            AIChaser(target);
            break;
        case SHOOTER:
            AIShooter(target);
            break;
        default:
            return;
    }
    this->movement = glm::normalize(this->movement);
    Entity::Update(deltaTime, map, objects, target);
}
