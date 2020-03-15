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
#include "time.h"
#include <string>
#include <vector>
using namespace std;

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, projectionMatrix;
const float WIDTH = 5, LENGTH = 3.75;

struct Entity {
	Entity(GLuint id, float w, float h, float x = 0, float y = 0) {
		ID = id; X = x; Y = y;
		width = w; height = h;
		vertices[0] = x - w / 2;
		vertices[1] = y - h / 2;
		vertices[2] = x + w / 2;
		vertices[3] = y - h / 2;
		vertices[4] = x + w / 2;
		vertices[5] = y + h / 2;
		vertices[6] = x - w / 2;
		vertices[7] = y - h / 2;
		vertices[8] = x + w / 2;
		vertices[9] = y + h / 2;
		vertices[10] = x - w / 2;
		vertices[11] = y + h / 2;
	}
	void stop() {
		accelerateX = 0;
		accelerateY = 0;
	}
	void drop() {
		accelerateY = -0.5;
	}
	bool isCollide(const Entity& item) const {
		float xdist = fabs(item.X - X) - ((width + item.width) / 2.0f);
		float ydist = fabs(item.Y - Y) - ((height + item.height) / 2.0f);
		return (xdist < -0.1 && ydist < -0.1 );
	}
	void render() const {
		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		program.SetModelMatrix(matrix);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glBindTexture(GL_TEXTURE_2D, ID);
		glDrawArrays(GL_POLYGON, 0, 6);
	}
	GLuint ID;
	float vertices[12];
	float X, Y, width, height;
	float accelerateX = 0, accelerateY = 0;
	float lastTicks = 0.0f;
	glm::mat4 matrix = glm::mat4(1.0f);
};

void loadGame(Entity&, vector<Entity>&);
GLuint LoadTexture(const char*);
void DrawText(ShaderProgram*, GLuint, string, float, float, glm::vec3);
void Initialize();
void ProcessInput(Entity&, vector<Entity>&, string&);
void Update(Entity&, const vector<Entity>&, string&);
void Render(const Entity&, const vector<Entity>&, const string&);
void Shutdown();

int main(int argc, char* argv[]) {
	// load window
	Initialize();

	// load entities
	Entity spaceship(LoadTexture("spaceship.png"), 0.7, 1);
	vector<Entity> tiles;
	string gameState = "Welcome :) Press SPACE to start the game";

	while (gameIsRunning) {
		ProcessInput(spaceship, tiles, gameState);
		Update(spaceship, tiles, gameState);
		Render(spaceship, tiles, gameState);
	}

	Shutdown();
	return 0;
}

void Initialize() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Project 3: Lunar Lander",
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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void ProcessInput(Entity& spaceship, vector<Entity>& tiles, string& gameState) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			gameIsRunning = false;
		}
	}

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_LEFT]) {
		// Decrease spaceship acceleration
		spaceship.accelerateX -= 0.01;
	}
	if (keys[SDL_SCANCODE_RIGHT]) {
		// Increase spaceship acceleration
		spaceship.accelerateX += 0.01;
	}
	if (keys[SDL_SCANCODE_SPACE]) {
		// Start the game
		loadGame(spaceship, tiles);
		spaceship.drop();
		gameState = "";
	}
}

void Update(Entity& spaceship, const vector<Entity>& tiles, string& gameState) {
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - spaceship.lastTicks;
	spaceship.lastTicks = ticks;

	// Decide game state
	if (spaceship.Y < (-1 * LENGTH + 0.5)) {
		spaceship.stop();
		gameState = "Mission Successful :) Press SPACE to play";
	}
	if (spaceship.X < (-1 * WIDTH) || spaceship.X > WIDTH) {
		spaceship.stop();
		gameState = "Mission Failed! Press SPACE to play again";
	}
		
	for (const Entity& tile : tiles) {
		if (spaceship.isCollide(tile)) {
			spaceship.stop();
			gameState = "Mission Failed! Press SPACE to play again";
		}
	}

	spaceship.matrix = glm::mat4(1.0f);
	spaceship.X += (spaceship.accelerateX * deltaTime);
	spaceship.Y += (spaceship.accelerateY * deltaTime);
	spaceship.matrix = glm::translate(spaceship.matrix, glm::vec3(spaceship.X, spaceship.Y, 0.0f));
}

void Render(const Entity& spaceship, const vector<Entity>& tiles, const string& gameState) {
	glClear(GL_COLOR_BUFFER_BIT);
	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

	spaceship.render();

	for (size_t i = 0; i < tiles.size(); ++i) {
		tiles[i].render();
	}

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

	if (gameState != "") {
		GLuint fontID = LoadTexture("font.png");
		DrawText(&program, fontID, gameState, 0.4f, -0.17f, glm::vec3(-1 * WIDTH + 0.4, LENGTH - 1, 0));
	}

	SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
	SDL_Quit();
}

void loadGame(Entity& spaceship, vector<Entity>& tiles) {
	// set up spaceship
	spaceship.X = 0;
	spaceship.Y = LENGTH - 0.5;
	spaceship.stop();

	// load barriers
	tiles.clear();
	srand(time(NULL));
	GLuint tile_id = LoadTexture("tile.jpg");
	int randExit = rand() % 4 - 2;
	for (int i = -2; i < 2; ++i) { // barriers
		if (i == randExit) continue;
		tiles.emplace_back(tile_id, 1, 1, i + 0.5, randExit + 1);
	}
	randExit = rand() % (int(WIDTH) * 2) - WIDTH;
	for (int i = -1 * WIDTH; i < WIDTH; ++i) { // planet surface
		if (i == randExit) continue;
		tiles.emplace_back(tile_id, 1, 1, i + 0.5, -1 * LENGTH + 0.5);
	}
}

GLuint LoadTexture(const char* filePath) {
	int w, h, n;
	unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

	if (image == NULL) {
		std::cerr << "Unable to load image. Make sure the path is correct\n";
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
