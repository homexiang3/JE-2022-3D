#pragma once

#include "entity.h"

class Scene
{
public:
	Scene();
	~Scene();

	Entity* root;
	std::vector<Entity*> s_to_destroy;//destroy vector
	//poner todas las entities
	//metodos como entidades dentro de area..

};

Scene::Scene()
{
}

Scene::~Scene()
{
}