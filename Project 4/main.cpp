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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Entity.h"
#include <string>
#include <vector>
using namespace std;

struct GameState {
	Entity* player;
	Entity* button;
	vector<Entity*> tiles;
	vector<Entity*> enemies;
};

const float WIDTH = 5.0f;
const float LENGTH = 3.75f;

SDL_Window* displayWindow;
bool gameIsRunning = true;

GameState state;
ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

GLuint LoadTexture(const char* filePath) {
	int w, h, n;
	unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
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

void stopGame(bool win) {
	for (Entity* enemy : state.enemies) {
		enemy->isActive = false;
	}
	if (win) {
		for (Entity* enemy : state.enemies) {
			enemy->textureID = LoadTexture("./images/dead_enemy.png");
		}
	}
}

void loadGame() {
	// Initialize Player
	state.player = new Entity(LoadTexture("./images/player.png"), 0.5f, 1.4f, glm::vec3(-1 * WIDTH + 1, 0, 0));
	state.player->speed = 5.0f;
	state.player->acceleration.y = -9.81f;
	state.player->animRight = new int[2]{ 2, 3 };
	state.player->animLeft = new int[2]{ 0, 1 };
	state.player->animIndices = state.player->animRight;
	state.player->animFrames = 2;
	state.player->animIndex = 0;
	state.player->animTime = 0;
	state.player->animCols = 2;
	state.player->animRows = 2;

	// Initialize Platform
	GLuint tile_id = LoadTexture("./images/tile.jpg");
	for (int i = 1; i < WIDTH; ++i) { // upper surface
		Entity* tile = new Entity(tile_id);
		tile->position = glm::vec3(i + 0.5, 0, 0);
		tile->Update(0);
		state.tiles.push_back(tile);
	}
	for (int i = -1 * WIDTH; i < WIDTH; ++i) { // ground surface
		if (i == -3) continue;
		Entity* tile = new Entity(tile_id);
		tile->position = glm::vec3(i + 0.5, -1 * LENGTH + 0.5, 0);
		tile->Update(0);
		state.tiles.push_back(tile);
	}

	// Initialize Button
	state.button = new Entity(LoadTexture("./images/button.png"), 1, 0.7);
	state.button->position = glm::vec3(WIDTH - 0.6, 0.6, 0);
	state.button->Update(0);

	// Initialize Enemies
	GLuint enemy_id = LoadTexture("./images/enemy.png");
	Entity* enemy = new Entity(enemy_id, 1.2, 0.6);
	enemy->position = glm::vec3(-2.5, LENGTH, 0);
	enemy->velocity.y = -2;
	state.enemies.push_back(enemy);
	enemy = new Entity(enemy_id, 1.2, 0.6);
	enemy->position = glm::vec3(WIDTH - 0.6, -1*LENGTH + 1.2, 0);
	enemy->velocity.x = -2;
	state.enemies.push_back(enemy);    
	enemy = new Entity(enemy_id, 1.2, 0.6);
	enemy->position = glm::vec3(WIDTH - 2, LENGTH - 1, 0);
	enemy->acceleration.y = -9.81;
	state.enemies.push_back(enemy);
}


void Initialize() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Project 4: Rise of the AI",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		640, 480, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 480);
	program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

	viewMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho((-1 * WIDTH), WIDTH, (-1 * LENGTH), LENGTH, -1.0f, 1.0f);
	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetColor(1.0f, 0.0f, 0.0f, 1.0f);

	glUseProgram(program.programID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.624f, 0.78f, 1.0f, 0.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	loadGame();
}

void ProcessInput(string gameState) {

	state.player->movement = glm::vec3(0);

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			gameIsRunning = false;
			break;

		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_SPACE:
				state.player->jump = true;
				break;
			}
			break;
		default:
			break;
		}
	}

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_LEFT]) {
		if (state.player->position.x > -1 * WIDTH + 0.7) {
			state.player->movement.x = -1.0f;
			state.player->animIndices = state.player->animLeft;
		}
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {
		if (state.player->position.x < WIDTH - 0.7) {
			state.player->movement.x = 1.0f;
			state.player->animIndices = state.player->animRight;
		}
	}

	if (glm::length(state.player->movement) > 1.0f) {
		state.player->movement = glm::normalize(state.player->movement);
	}
}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;
void Update() {
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - lastTicks;
	lastTicks = ticks;

	deltaTime += accumulator;
	if (deltaTime < FIXED_TIMESTEP) {
		accumulator = deltaTime;
		return;
	}

	// Enemy behavior: dropping
	Entity* spyder = state.enemies[0];
	if (spyder->position.y < -2 * LENGTH) {
		spyder->position.y = LENGTH;
	}
	spyder->Update(FIXED_TIMESTEP);
	// Enemy behavior: patrolling
	spyder = state.enemies[1];
	if (spyder->position.x < -1.5 || spyder->position.x > WIDTH - 0.5) {
		spyder->velocity.x *= -1;
	}
	spyder->Update(FIXED_TIMESTEP, {}, false);
	// Enemy behavior: jumping
	spyder = state.enemies[2];
	if (spyder->position.y < 0.7) {
		spyder->velocity.y = -1;
		spyder->jump = true;
	}
	spyder->Update(FIXED_TIMESTEP, {});

	// Update player
	while (deltaTime >= FIXED_TIMESTEP) {
		deltaTime -= FIXED_TIMESTEP;
		state.player->Update(FIXED_TIMESTEP, state.tiles);
	}
	accumulator = deltaTime;
}


void Render(string& gameState, GLuint fontID) {
	glClear(GL_COLOR_BUFFER_BIT);

	// Decide gameState
	if (state.player->CheckCollision(state.button)) {
		if (gameState != "Game Over") {
			gameState = "You Win";
			stopGame(true);
		}
	}
	for (Entity* enemy : state.enemies) {
		if (state.player->CheckCollision(enemy)) {
			gameState = "Game Over";
			stopGame(false);
			break;
		}
	}
	if (state.player->position.y < -1 * LENGTH) {
		gameState = "Game Over";
		stopGame(false);
	}
	DrawText(&program, fontID, gameState, 0.4f, -0.18f, glm::vec3(-1 * WIDTH + 0.4, LENGTH - 1, 0));

	for (Entity* tile: state.tiles) {
		tile->Render(&program);
	}
	for (Entity* enemy: state.enemies) {
		enemy->Render(&program);
	}
	state.button->Render(&program);
	state.player->Render(&program);

	SDL_GL_SwapWindow(displayWindow);
}


void Shutdown() {
	SDL_Quit();
}

int main(int argc, char* argv[]) {
	Initialize();

	string gameState = "Welcome Press button to get rid all enemies";
	GLuint fontID = LoadTexture("./images/font.png");

	while (gameIsRunning) {
		ProcessInput(gameState);
		Update();
		Render(gameState, fontID);
	}

	Shutdown();
	return 0;
}