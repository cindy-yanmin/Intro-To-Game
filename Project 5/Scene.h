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
#include <string>
#include <vector>
#include "Entity.h"

struct GameState {
	GLuint fontID;
	Map* map;
	Entity* player;
	std::vector<Entity*> enemies;
	bool nextScene = false;
};

class Scene {
public:
	Scene(int w, int h, unsigned int* l);
	~Scene();
	GameState state;
	void Initialize(int enemyCount = 0);
	bool Update(float& deltaTime);
	void Render(ShaderProgram* program, const std::string& gameState, int lives);
	void stopGame(bool win);
private:
	int width, height;
	unsigned int* levelData;
};
