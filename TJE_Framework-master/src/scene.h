#pragma once

#include "entity.h"

class Prop
{
public:
	Prop();
	~Prop();
	int id;
	Mesh* mesh;
	Texture* texture;

};


class Scene
{
public:
	Scene();
	~Scene();
	//props of our game
	Prop props[20];
	//Entity* root;
	std::vector<EntityMesh*> entities;
	std::vector<Entity*> s_to_destroy;//destroy vector
	//poner todas las entities
	//metodos como entidades dentro de area..
	void addEntityOnFront();
};


