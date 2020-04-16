#include "Scene.h"
#include <SDL_mixer.h>
using namespace std;


#define WIDTH 5.0f
#define LENGTH 3.75f

SDL_Window* displayWindow;
bool gameIsRunning = true;
ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;


unsigned int menu_data[] = {
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 2, 2, 0, 1, 0, 4, 0,
 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};

unsigned int level1_data[] = {
 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 5, 5, 0, 0, 0, 5, 5, 0, 0, 0, 0, 0, 0, 0, 2, 4, 0, 0, 0, 0,
 5, 5, 5, 5, 6, 6, 5, 5, 5, 6, 6, 5, 5, 5, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5,
 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6 };

unsigned int level2_data[] = {
 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0,
 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0,
 0, 0, 0, 5, 6, 5, 0, 0, 1, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 6, 5, 0, 0, 0,
 5, 5, 5, 6, 6, 6, 5, 5, 5, 0, 0, 6, 5, 0, 0, 2, 0, 0, 5, 6, 6, 6, 5, 5, 5,
 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 5, 0, 0, 6, 6, 6, 6, 6, 6, 6 };

unsigned int level3_data[] = {
 0, 0, 0, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 1, 0, 0, 7, 7, 0, 7, 0, 7, 0, 0, 0, 7, 0, 7, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 7, 0, 7, 0, 0, 0, 0,
 7, 7, 7, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 7, 0, 2, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 7, 7, 7, 7, 7 };

Scene* sceneList[] =
{
	new Scene(11, 8, menu_data),
	new Scene(24, 8, level1_data),
	new Scene(25, 8, level2_data),
	new Scene(24, 8, level3_data)
};
int currentSceneIndex = 0;
Scene* currentScene = sceneList[currentSceneIndex];

void switchScene(string& gameState) {
	switch (currentSceneIndex) {
	case 0:
		gameState = "";
		currentSceneIndex = 1;
		break;
	case 1:
		currentSceneIndex = 2;
		break;
	case 2:
		currentSceneIndex = 3;
		break;
	case 3:
		gameState = "Congratulations :) You won!!!";
		currentScene->Initialize();
		currentScene->stopGame(true);
		return;
 	case -1:
		gameState = "Oh no!!! You lost, good game :(";
		currentScene->Initialize();
		currentScene->stopGame(false);
		return;
	default:
		break;
	}
	currentScene = sceneList[currentSceneIndex];
	if (currentScene) currentScene->Initialize(currentSceneIndex);
}

void Initialize() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Project 5: Platformer!",
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

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	Mix_Music* bgm = Mix_LoadMUS("./src/bgm.mp3");
	Mix_PlayMusic(bgm, -1);

	currentScene->Initialize();
}

void ProcessInput(string& gameState, int& lives, Mix_Chunk* jump) {
	Entity* player = (currentScene->state).player;
	player->movement = glm::vec3(0);

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
				if (player->collidedBottom) {
					Mix_FadeInChannel(-1, jump, 0, 5);
					player->jump = true;
				}
				break;
			case SDLK_RETURN:
				if (currentSceneIndex == 0) {
					lives = 3;
					switchScene(gameState);
				}
				break;
			}
			break;
		default:
			break;
		}
	}

	if (gameState != "") return;

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_LEFT]) {
		if (player->position.x > 0.5) {
			player->movement.x = -1.0f;
			player->animIndices = player->animLeft;
		}
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {
		player->movement.x = 1.0f;
		player->animIndices = player->animRight;
	}

	if (glm::length(player->movement) > 1.0f) {
		player->movement = glm::normalize(player->movement);
	}
}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;
void Update(string& gameState, int& lives) {
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - lastTicks;
	lastTicks = ticks;

	deltaTime += accumulator;
	if (deltaTime < FIXED_TIMESTEP) {
		accumulator = deltaTime;
		return;
	}

	// Update scene
	bool lostLive = currentScene->Update(deltaTime);
	accumulator = deltaTime;

	// Sidescrolling
	viewMatrix = glm::mat4(1.0f);
	float pos = (currentScene->state).player->position.x;
	if (pos > WIDTH)
		viewMatrix = glm::translate(viewMatrix, glm::vec3(-pos, LENGTH, 0));
	else
		viewMatrix = glm::translate(viewMatrix, glm::vec3(-WIDTH, LENGTH, 0));
	
	// Switch scene
	if (lostLive) {
		if (lives > 0) lives--;
		if (lives == 0) { // Lost
			currentSceneIndex = -1;
			switchScene(gameState);
		} else 
			currentScene->Initialize();
	}
	if (currentSceneIndex && currentScene->state.nextScene)
		switchScene(gameState);
}

void Render(const string& gameState, int lives) {
	glClear(GL_COLOR_BUFFER_BIT);
	program.SetViewMatrix(viewMatrix);

	currentScene->Render(&program, gameState, lives);
	SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
	SDL_Quit();
}

int main(int argc, char* argv[]) {
	Initialize();
	
	string gameState = "Welcome to Peppa Pig's Adventure";
	Mix_Chunk* jump = Mix_LoadWAV("./src/jump.wav");
	int lives = 3;

	while (gameIsRunning) {
		ProcessInput(gameState, lives, jump);
		Update(gameState, lives);
		Render(gameState, lives);
	}

	Shutdown();
	return 0;
}
