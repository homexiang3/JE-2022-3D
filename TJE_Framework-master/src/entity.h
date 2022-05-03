#pragma once

#include "framework.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"

class Entity
{
public:
	Entity(); //constructor
	virtual ~Entity(); //destructor

	//some attributes
	std::string name;
	Matrix44 model;

	//methods overwritten
	virtual void render();
	virtual void update(float elapsed_time);

	//useful methods
	Vector3 getPosition();

	//arbol de escena
	Entity* parent;
	std::vector<Entity*> children;//vector of children
	void addChild(Entity* ent);
	void removeChild(Entity* ent);
	Matrix44 getGlobalMatrix();
};

class EntityMesh : public Entity {
public:
	//Attributes of this class 
	Mesh* mesh;
	Texture* texture;
	Shader* shader;
	Vector4 color;

	//methods overwritten 
	void render();
	void update(float dt);
};

/*otros ejemplos

class EntityCamera : public Entity {};
class EntityLight : public Entity {};
class EntitySound : public Entity {};
class EntityTrigger : public Entity {};

*/