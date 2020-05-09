#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Scene.h"

using namespace std;

GLuint LoadTexture(const char* filePath) 
{
	int w, h, n;
	unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

	if (image == NULL) {
		cerr << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(image);
	return textureID;
}

void DrawText(ShaderProgram* program, GLuint fontTextureID, string text,
	float size, float spacing, glm::vec3 position) 
{

	float width = 1.0f / 16.0f;
	float height = 1.0f / 16.0f;
	std::vector<float> vertices;
	std::vector<float> texCoords;

	for (int i = 0; i < text.size(); i++) {
		int index = (int)text[i];
		float offset = (size + spacing) * i;
		float u = (float)(index % 16) / 16.0f;
		float v = (float)(index / 16) / 16.0f;
		vertices.insert(vertices.end(), {
			offset + (-0.5f * size), 0.5f * size,
			offset + (-0.5f * size), -0.5f * size,
			offset + (0.5f * size), 0.5f * size,
			offset + (0.5f * size), -0.5f * size,
			offset + (0.5f * size), 0.5f * size,
			offset + (-0.5f * size), -0.5f * size,
			});
		texCoords.insert(texCoords.end(), {
			u, v,
			u, v + height,
			u + width, v,
			u + width, v + height,
			u + width, v,
			u, v + height,
			});
	}
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	program->SetModelMatrix(modelMatrix);

	glUseProgram(program->programID);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
	glEnableVertexAttribArray(program->texCoordAttribute);

	glBindTexture(GL_TEXTURE_2D, fontTextureID);
	glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void Scene::Initialize(bool isGame) 
{
	resetGame();
	this->isGame = isGame;
	fontTextureID = LoadTexture("./src/font.png");

	// Initialize map
	GLuint mapTextureID = LoadTexture("./src/tileset.png");
	state.map = new Map(width, height, levelData, mapTextureID, 1.0f, 4, 2);

	if (isGame) {
		//Get initial time
		state.startTime = time(NULL);
        state.lastWaveTime = time(NULL);

		// Initialize player
		state.player = new Player(LoadTexture("./src/player.png"), 1.0f, 1.2f);
		state.player->position = glm::vec3(10, -7.5, 0);

        // Initialize 4 chasers at each corner
        for (size_t i = 0; i < spawnLocations.size(); i++){
            state.enemies.push_back(new Enemy(LoadTexture("./src/chaser.png"), 1.0f, 1.0f));
            state.enemies[i]->position = glm::vec3(spawnLocations[i].first,spawnLocations[i].second,0);
            state.enemies[i]->entityType = Enemy::CHASER;
        }
        
        // Test shooter
//        state.enemies.push_back(new Enemy(LoadTexture("./src/shooter.png"), 1.0f, 1.0f));
//        state.enemies[4]->position = glm::vec3(5,-7.5,0);
//        state.enemies[4]->entityType = Enemy::SHOOTER;
    }
}

int Scene::Update(float deltaTime) 
{
	if (isGame) {
		// Update player
		if (state.player->shoot) makePlayerBullet();
        state.player->Update(deltaTime, state.map);

        // Spawn new enemies every 5 seconds
        time_t currTime = time(NULL);
        srand(time(NULL));
        if (currTime - state.lastWaveTime > 5){
            // Spawn randomly at spawn points
            for (size_t i = 0; i < spawnLocations.size(); i++){
                int randProb = rand() % 10;
                int shooterProb = rand() % 10;
                Enemy* tempEnemy;
                if (randProb < 6){ // 60% chance to spawn an enemy
                    if (shooterProb < 3){ // 30% chance to spawn a shooter
						tempEnemy = new Enemy(LoadTexture("./src/shooter.png"), 1.0f, 1.0f);
						tempEnemy->position = glm::vec3(spawnLocations[i].first, spawnLocations[i].second, 0);
                        tempEnemy->entityType = Enemy::SHOOTER;
                    }else{ // 30% chance to spawn a chaser
						tempEnemy = new Enemy(LoadTexture("./src/chaser.png"), 1.0f, 1.0f);
						tempEnemy->position = glm::vec3(spawnLocations[i].first, spawnLocations[i].second, 0);
                        tempEnemy->entityType = Enemy::CHASER;
                    }
                    state.enemies.push_back(tempEnemy);
                }
            }
            state.lastWaveTime = currTime;
        }

		// Update bullets
		for (size_t i = 0; i < state.bullets.size(); i++) {
			state.bullets[i]->Update(deltaTime, state.map, state.enemies, state.player);
		}
		
		// Update enemies 
        for (Entity* enemy : state.enemies){
			// Losing condition: collide with an enemy
			if (enemy->CheckCollision(state.player)) return -1; 

			if (enemy->shoot) makeEnemyBullet(enemy);
			enemy->Update(deltaTime, state.map, state.enemies, state.player);
        }
        
		// Winning condition: time is up
		state.timer = 120 - (time(NULL) - state.startTime);
		if (state.timer <= 0) 
			return 1;
		// Losing condition: collide with ENEMY_BULLET
		else if (!state.player->isActive) 
			return -1;
	}

	return 0;
}

void Scene::Render(ShaderProgram* program, const string& menuText) 
{
	state.map->Render(program);

	if (isGame) {
		DrawText(program, fontTextureID, "Timer: " + to_string(state.timer) + "s", 0.7f, -0.35f, glm::vec3(1.5, -1.5, 0));

		// Render player
		state.player->Render(program);
		// Render enemies
		for (Entity* enemy : state.enemies) {
			enemy->Render(program);
		}
		// Render bullets
		for (Bullet* bullet : state.bullets) {
			bullet->Render(program);
		}
	}
	else {
        DrawText(program, fontTextureID, "PEAHEAD", 1.0f, -0.4f, glm::vec3(7.5, -4, 0));
		DrawText(program, fontTextureID, menuText, 1.0f, -0.4f, glm::vec3(4, -6, 0));
	}
}

void Scene::makePlayerBullet() 
{
	state.player->shoot = false;

	Bullet* tempBullet = new Bullet(LoadTexture("./src/player_bullet.png"));
	tempBullet->entityType = Entity::PLAYER_BULLET;
	tempBullet->position = state.player->position;
	state.player->movement = -1.0f * tempBullet->setBulletMovement(state.player->shootDirection);
	
	state.bullets.push_back(tempBullet);
}

void Scene::makeEnemyBullet(Entity* enemy) 
{
	enemy->shoot = false;
    
	Bullet* tempBullet = new Bullet(LoadTexture("./src/enemy_bullet.png"));
	tempBullet->entityType = Entity::ENEMY_BULLET;
	tempBullet->setBulletMovement(enemy->shootDirection);
    //bullet spawns a little before the enemy
	float offset = .75f;
    switch(enemy->shootDirection){
        case 0: //up
            tempBullet->position = enemy->position + glm::vec3(0,offset,0);
            break;
        case 1: //down
            tempBullet->position = enemy->position + glm::vec3(0,-offset,0);
            break;
        case 2: //right
            tempBullet->position = enemy->position + glm::vec3(offset,0,0);
            break;
        case 3: //left
            tempBullet->position = enemy->position + glm::vec3(-offset,0,0);
            break;
    }
	
	state.bullets.push_back(tempBullet);
}

void Scene::resetGame()
{
	for (size_t i = 0; i < state.bullets.size(); ++i) {
		delete state.bullets[i];
	}
	state.bullets.clear();

	for (size_t i = 0; i < state.enemies.size(); ++i) {
		delete state.enemies[i];
	}
	state.enemies.clear();

	delete state.map;
	state.map = nullptr;
	delete state.player;
	state.player = nullptr;
}

Scene::Scene(unsigned int* l) : levelData(l) {}
Scene::~Scene() { resetGame(); }
bool Scene::IsGame() const { return isGame; }
