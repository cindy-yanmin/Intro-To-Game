#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include <vector>
#include "Map.h"

class Entity {
public:
	glm::vec3 position;
	glm::vec3 movement;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	float vertices[12];
	float width, height;
	float speed;
	
	bool jump = false;
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
	int animCols = 0;
	int animRows = 0;

	bool collidedTop = false;
	bool collidedBottom = false;
	bool collidedLeft = false;
	bool collidedRight = false;

	Entity(GLuint id, float w = 1, float h = 1);
	bool CheckCollision(const Entity* other) const;
	void CheckCollisionsX(const std::vector<Entity*> platforms);
	void CheckCollisionsY(const std::vector<Entity*> platforms);
	void CheckCollisionsX(Map* map);
	void CheckCollisionsY(Map* map);
	void Update(float deltaTime, Map* map, const std::vector<Entity*> objects = {}, bool calculateVolX = true);
	void Render(ShaderProgram* program);
	void DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index);
};
