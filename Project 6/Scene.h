#pragma once
#include <ctime>
#include <string>
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"

struct GameState {
	time_t timer = 0;
	time_t startTime;
	time_t lastWaveTime;
	
	Map* map = nullptr;
	Player* player = nullptr;
    std::vector<Bullet*> bullets;
	std::vector<Entity*> enemies;
};

class Scene {
public:
	GameState state;
	glm::mat4 viewMatrix;
    
    std::vector<std::pair<int, int>> spawnLocations = {
        std::make_pair(2,-2), std::make_pair(18,-2), std::make_pair(18,-13), std::make_pair(2,-13)
    };
    
	Scene(unsigned int* l);
	~Scene();
	bool IsGame() const;
	void resetGame();
	void makePlayerBullet();
	void makeEnemyBullet(Entity* enemy);

	void Initialize(bool isGame = false);
	int Update(float deltaTime);
	void Render(ShaderProgram* program, const std::string& gameState);
private:
	unsigned int* levelData;
	int width = 21, height = 16;

	bool isGame;
	GLuint fontTextureID;
};
