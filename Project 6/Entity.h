#pragma once
#include "Map.h"

class Entity {
public:
	glm::vec3 position;
	glm::vec3 movement;
	glm::vec3 acceleration;
	glm::vec3 velocity;
	float speed = 1.0f;

	float vertices[12];
	float width, height;

	bool shoot = false;
	int shootDirection;
	bool isActive = true;
	GLuint textureID;
	glm::mat4 modelMatrix;
   
    int* animRight = NULL;
    int* animLeft = NULL;
    int* animUp = NULL;
    int* animDown = NULL;

    int* animIndices = NULL;
    int animFrames = 0;
    int animIndex = 0;
    float animTime = 0;
    int animCols = 1;
    int animRows = 1;

	bool collidedTop = false;
	bool collidedBottom = false;
	bool collidedLeft = false;
	bool collidedRight = false;
    
	enum EntityType { PLAYER, CHASER, SHOOTER, PLAYER_BULLET, ENEMY_BULLET };
	EntityType entityType;

	Entity(GLuint id, float w = 1, float h = 1);
	bool CheckCollision(const Entity* other) const;
	void CheckCollisionsX(const std::vector<Entity*> platforms);
	void CheckCollisionsY(const std::vector<Entity*> platforms);
	void CheckCollisionsX(Map* map);
	void CheckCollisionsY(Map* map);
	virtual void Update(float deltaTime, Map* map, const std::vector<Entity*> objects = {}, Entity* target = nullptr);
	virtual void Render(ShaderProgram* program);
    void DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index);
   
};
