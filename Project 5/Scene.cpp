#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Scene.h"

using namespace std;

GLuint LoadTexture(const char* filePath) {
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
	float size, float spacing, glm::vec3 position) {

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

void Scene::Initialize(int enemyCount) {
	// Load fontID
	state.fontID = LoadTexture("./src/font.png");

	// Initialize Player
	state.player = new Entity(LoadTexture("./src/player.png"), 0.7f, 1.1f);
	state.player->position = glm::vec3( 1, 0, 0 );
	state.player->animRight = new int[2]{ 2, 3 };
	state.player->animLeft = new int[2]{ 0, 1 };
	state.player->animIndices = state.player->animRight;
	state.player->animFrames = 2;
	state.player->animIndex = 0;
	state.player->animTime = 0;
	state.player->animCols = 2;
	state.player->animRows = 2;

	// Initialize Map
	GLuint mapTextureID = LoadTexture("./src/tileset.png");
	state.map = new Map(width, height, levelData, mapTextureID, 1.0f, 4, 2);

	// Initialize Enemies
	GLuint enemy_id = LoadTexture("./src/enemy.png");
	for (int i = 1; i <= enemyCount; ++i) {
		Entity* enemy = new Entity(enemy_id, 0.9f, 0.45f);
		enemy->position = glm::vec3(6*i, -2, 0);
		state.enemies.push_back(enemy);
	}
}

#define FIXED_TIMESTEP 0.0166666f
bool Scene::Update(float& deltaTime) {
	// Update player
	while (deltaTime >= FIXED_TIMESTEP) {
		deltaTime -= FIXED_TIMESTEP;
		state.player->Update(FIXED_TIMESTEP, state.map);
	}
	if (state.player->position.x > (width - 5.6)) state.nextScene = true;

	// Check lose condition
	for (Entity* enemy : state.enemies) {
		enemy->Update(FIXED_TIMESTEP, state.map);
		// Keep on jumping
		if (enemy->collidedBottom) {
			enemy->velocity.y = -1;
			enemy->jump = true;
		}
		if (state.player->CheckCollision(enemy))
			return true;
	}
	if (state.player->position.y < -10) return true;
	
	return false;
}

void Scene::Render(ShaderProgram *program, const string& gameState, int lives) {
	
	state.map->Render(program);
	state.player->Render(program);
	for (Entity* enemy : state.enemies) {
		enemy->Render(program);
	}

	DrawText(program, state.fontID, "Lives: " + to_string(lives), 0.4f, -0.15f, 
		glm::vec3(state.player->position.x - state.player->width, state.player->position.y + state.player->height, 0));
	if (gameState != "") {
		DrawText(program, state.fontID, gameState, 0.4f, -0.15f, glm::vec3(1, -1, 0));
		// DrawText(program, state.fontID, "Press enter to start", 0.4f, -0.15f, glm::vec3(2.5, -1.5, 0));
	}
}

void Scene::stopGame(bool win) {
	state.nextScene = false;
	for (Entity* enemy : state.enemies) {
		enemy->isActive = false;
	}
	if (win) {
		for (Entity* enemy : state.enemies) {
			enemy->textureID = LoadTexture("./src/dead_enemy.png");
		}
	}
}

Scene::Scene(int w, int h, unsigned int* l) : width(w), height(h), levelData(l) {}

Scene::~Scene() {
	delete state.player;
	delete state.map;
}
