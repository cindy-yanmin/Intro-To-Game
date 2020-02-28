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
#include <string>
#include <vector>

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, projectionMatrix;
const float WIDTH = 5, LENGTH = 3.75;

struct Item {
	Item(GLuint id, float w, float h, float x = 0, float y = 0) {
		ID = id;
		X = x;
		Y = y;
		width = w;
		height = h;
	}
	void reset() {
		X = 0;
		Y = 0;
		velocityX = 0;
		velocityY = 0;
	}
	void wander(int minSpeed) {
		while (velocityX * velocityX + velocityY * velocityY < minSpeed) {
			velocityX = float(rand() % 1000) / 100 - 5;
			velocityY = float(rand() % 1000) / 100 - 5;
		}
	}
	void move(float distance) {
		matrix = glm::mat4(1.0f);
		Y += distance;
		matrix = glm::translate(matrix, glm::vec3(0.0f, Y, 0.0f));
	}
	bool isCollide(const Item& item) const {
		float xdist = fabs(item.X - X) - ((width + item.width) / 2.0f);
		float ydist = fabs(item.Y - Y) - ((height + item.height) / 2.0f);
		return (xdist < 0 && ydist < 0);
	}
	GLuint ID;
	float X, Y, width, height;
	float velocityX = 0, velocityY = 0;
	float lastTicks = 0.0f;
	glm::mat4 matrix = glm::mat4(1.0f);
};

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

void Initialize() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Project 2: PONG!", 
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
	projectionMatrix = glm::ortho((-1* WIDTH), WIDTH, (-1*LENGTH), LENGTH, -1.0f, 1.0f);
	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetColor(1.0f, 0.0f, 0.0f, 1.0f);
	
	glUseProgram(program.programID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void ProcessInput(Item& ball, Item& paddle1, Item& paddle2) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			gameIsRunning = false;
		}
	}

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_A]) {
		// Move the left paddle up
		if (paddle1.Y < (LENGTH - 1)) paddle1.move(0.01);
	}
	if (keys[SDL_SCANCODE_Z]) {
		// Move the left paddle down
		if (paddle1.Y > (-1 * LENGTH + 1)) paddle1.move(-0.01);
	}
	if (keys[SDL_SCANCODE_K]) {
		// Move the right paddle up
		if (paddle2.Y < (LENGTH - 1)) paddle2.move(0.01);
	}
	if (keys[SDL_SCANCODE_M]) {
		// Move the right paddle down
		if (paddle2.Y > (-1 * LENGTH + 1)) paddle2.move(-0.01);
	}
	if (keys[SDL_SCANCODE_SPACE]) {
		// Start the game
		ball.wander(25); 
	}

}

void Update(Item& ball, const Item& paddle1, const Item& paddle2) {
	// Decide time
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - ball.lastTicks;
	ball.lastTicks = ticks;

	// Decide collision
	if (ball.isCollide(paddle1) || ball.isCollide(paddle2)) ball.velocityX *= -1;
	bool collideY = (ball.Y < -1 * LENGTH || ball.Y > (LENGTH));
	if (collideY) ball.velocityY *= -1;

	// Update ball
	ball.matrix = glm::mat4(1.0f);
	ball.X += (ball.velocityX * deltaTime);
	ball.Y += (ball.velocityY * deltaTime);
	ball.matrix = glm::translate(ball.matrix, glm::vec3(ball.X, ball.Y, 0.0f));
}

void Render(const Item& ball, const Item& paddle1, const Item& paddle2) {
	glClear(GL_COLOR_BUFFER_BIT);
	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

	program.SetModelMatrix(ball.matrix);
	float vertices[] = { -0.2, -0.2, 0.2, -0.2, 0.2, 0.2, -0.2, -0.2, 0.2, 0.2, -0.2, 0.2 };
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, ball.ID);
	glDrawArrays(GL_POLYGON, 0, 6);

	program.SetModelMatrix(paddle1.matrix);
	float vertices1[] = { -5, -1, -3.8, -1, -3.8, 1, -5, -1, -3.8, 1, -5, 1 };
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices1);
	glEnableVertexAttribArray(program.positionAttribute);
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, paddle1.ID);
	glDrawArrays(GL_POLYGON, 0, 6);

	program.SetModelMatrix(paddle2.matrix);
	float vertices2[] = { 5, -1, 3.8, -1, 3.8, 1, 5, -1, 3.8, 1, 5, 1 };
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
	glEnableVertexAttribArray(program.positionAttribute);
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, paddle2.ID);
	glDrawArrays(GL_POLYGON, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

	SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
	SDL_Quit();
}

int main(int argc, char* argv[]) {
	Initialize();

	Item ball(LoadTexture("ball.jpg"), 0.4, 0.4);
	Item paddle1(LoadTexture("paddle.jpg"), 0.1, 2, -3.8, 0);
	Item paddle2(LoadTexture("paddle.jpg"), 0.1, 2, 3.8, 0);

	while (gameIsRunning) {
		if (ball.X < -1 * WIDTH) {
			ball.reset();
			std::cout << "The player on the right side won!\n";
		}
		else if (ball.X > WIDTH) {
			ball.reset();
			std::cout << "The player on the left side won!\n";
		}
		ProcessInput(ball, paddle1, paddle2);
		Update(ball, paddle1, paddle2);
		Render(ball, paddle1, paddle2);
	}

	Shutdown();
	return 0;
}