#include "scene.h"
#include "game.h"
#include "input.h"


Scene::Scene()
{
}

Scene::~Scene()
{
}

Prop::Prop()
{

}
Prop::~Prop()
{

}

void Scene::addEntityOnFront() { //no va del todo fino

	Vector2 mouse = Input::mouse_position;
	Game* g = Game::instance;
	Camera* cam = g->camera;
	Vector3 dir = cam->getRayDirection(mouse.x, mouse.y, g->window_width, g->window_height);
	Vector3 rayOrigin = cam->eye;
	

	Vector3 spawnPos = RayPlaneCollision(Vector3(), Vector3(0,1,0), rayOrigin, dir);
	Matrix44 model; 
	model.translate(spawnPos.x, spawnPos.y, spawnPos.z);
	model.scale(3.0f, 3.0f, 3.0f);
	
	EntityMesh* entity = new EntityMesh("data/sphere.obj","data/texture.tga", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	entity->model = model;
	

	entities.push_back(entity);
	
}