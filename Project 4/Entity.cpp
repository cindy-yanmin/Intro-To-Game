#include "Entity.h"
using namespace std;

Entity::Entity(GLuint id, float w, float h, glm::vec3 p)
{
	speed = 0; 
	textureID = id;
	width = w; height = h;

	
	position = p;
	movement = glm::vec3(0);
	velocity = glm::vec3(0);
	acceleration = glm::vec3(0);
	modelMatrix = glm::mat4(1.0f);

	vertices[0] = position.x - w / 2;
	vertices[1] = position.y - h / 2;
	vertices[2] = position.x + w / 2;
	vertices[3] = position.y - h / 2;
	vertices[4] = position.x + w / 2;
	vertices[5] = position.y + h / 2;
	vertices[6] = position.x - w / 2;
	vertices[7] = position.y - h / 2;
	vertices[8] = position.x + w / 2;
	vertices[9] = position.y + h / 2;
	vertices[10] = position.x - w / 2;
	vertices[11] = position.y + h / 2;
}

bool Entity::CheckCollision(const Entity* other) const {
	if (!isActive || !other->isActive) return false;

	float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
	float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);
	return (xdist < 0 && ydist < 0);
}

void Entity::CheckCollisionsX(const vector<Entity*> platforms)
{
	for (Entity* object: platforms)
	{

		if (CheckCollision(object))
		{
			float xdist = fabs(position.x - object->position.x);
			float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
			if (velocity.x > 0) {
				position.x -= penetrationX;
				velocity.x = 0;
			}
			else if (velocity.x < 0) {
				position.x += penetrationX;
				velocity.x = 0;
			}
		}
	}
}

void Entity::CheckCollisionsY(const vector<Entity*> platforms)
{
	for (Entity* object : platforms)
	{

		if (CheckCollision(object))
		{
			float ydist = fabs(position.y - object->position.y);
			float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
			if (velocity.y > 0) {
				position.y -= penetrationY;
				velocity.y = 0;
			}
			else if (velocity.y < 0) {
				position.y += penetrationY;
				velocity.y = 0;
			}
		}
	}
}

void Entity::Update(float deltaTime, vector<Entity*> platforms, bool calculateVolX)
{
	if (!isActive) return;

	if (animIndices != NULL) {
		if (glm::length(movement) != 0) {
			animTime += deltaTime;

			if (animTime >= 0.25f)
			{
				animTime = 0.0f;
				animIndex++;
				if (animIndex >= animFrames)
				{
					animIndex = 0;
				}
			}
		}
		else {
			animIndex = 0;
		}
	}

	if (jump) {
		jump = false;
		velocity.y += 8.5;
	}

	if (calculateVolX) velocity.x = movement.x * speed;
	velocity += acceleration * deltaTime;
	
	position.y += velocity.y * deltaTime; // Move on Y
	CheckCollisionsY(platforms);// Fix if needed

	position.x += velocity.x * deltaTime; // Move on X
	CheckCollisionsX(platforms);// Fix if needed

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
}

void Entity::DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index)
{
	float u = (float)(index % animCols) / (float)animCols;
	float v = (float)(index / animCols) / (float)animRows;

	float width = 1.0f / (float)animCols;
	float height = 1.0f / (float)animRows;

	float texCoords[] = { u, v + height, u + width, v + height, u + width, v,
		u, v + height, u + width, v, u, v };

	float vertices[] = { -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1 };

	glBindTexture(GL_TEXTURE_2D, textureID);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::Render(ShaderProgram* program) {
	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
	program->SetModelMatrix(modelMatrix);

	if (animIndices != NULL) {
		DrawSpriteFromTextureAtlas(program, textureID, animIndices[animIndex]);
		return;
	}

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);

}