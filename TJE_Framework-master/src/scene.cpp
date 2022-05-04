#include "scene.h"
#include "game.h"
#include "input.h"

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::addEntityOnFront() { //no va del todo fino

	Vector2 mouse = Input::mouse_position;
	Game* g = Game::instance;
	Camera* cam = g->camera;
	Vector3 dir = cam->getRayDirection(mouse.x, mouse.y, g->window_width, g->window_height);
	Vector3 rayOrigin = cam->eye;
	

	Vector3 spawnPos = RayPlaneCollision(Vector3(), Vector3(0,1,0), rayOrigin, dir);
	EntityMesh* entity = new EntityMesh();

	entity->model.scale(5.0f, 5.0f, 5.0f);
	entity->model.translate(spawnPos.x, spawnPos.y, spawnPos.z);

	entity->mesh = Mesh::Get("data/sphere.obj");
	entity->texture = Texture::Get("data/texture.tga");
	entity->shader  = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
	entity->color = Vector4(1, 1, 1, 1);

	entities.push_back(entity);
	
}