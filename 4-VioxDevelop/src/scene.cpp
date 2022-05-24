#include "scene.h"
#include "game.h"
#include "input.h"


Scene::Scene()
{
	//init stages
	InitStages(stages);
	//load entities
	this->player.entity = new EntityMesh(GL_TRIANGLES, "data/skelleton.obj", "data/minichar_tex.png", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	this->skyMesh = new EntityMesh(GL_TRIANGLES, "data/cielo.ASE", "data/cielo.tga", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	//load grass/ground
	this->groundMesh = new EntityMesh(GL_TRIANGLES, NULL, "data/grass.tga", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	this->groundMesh->tiling = 200.0f;
	this->groundMesh->mesh = new Mesh();
	this->groundMesh->mesh->createPlane(1000);
	
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

void Scene::addEntityOnFront() { 

	Vector3 dir = getRayDir();
	Vector3 rayOrigin = getRayOrigin();
	
	Vector3 spawnPos = RayPlaneCollision(Vector3(), Vector3(0,1,0), rayOrigin, dir);
	Matrix44 model; 
	model.translate(spawnPos.x, spawnPos.y, spawnPos.z);

	EntityMesh* entity = new EntityMesh(GL_TRIANGLES,"data/building-shop-china.obj","data/color-atlas.png", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	entity->model = model;
	

	entities.push_back(entity);
	
}

void Scene::testCollisionOnFront() {

	Vector3 dir = getRayDir();
	Vector3 rayOrigin = getRayOrigin();

	for (size_t i = 0; i < this->entities.size(); i++)
	{
		EntityMesh* entity = entities[i];
		Vector3 pos;
		Vector3 normal;
		if (entity->mesh->testRayCollision(entity->model, rayOrigin, dir, pos, normal)) {
			std::cout << "col" << std::endl;
			selectedEntity = entity;
			break;
		}
													
	}
}

void Scene::rotateSelected(float angleDegree) {
	if (this->selectedEntity == NULL) {
		return;
	}

	this->selectedEntity->model.rotate(angleDegree * DEG2RAD, Vector3(0, 1, 0));
}

Vector3 Scene::getRayDir() {

	Vector2 mouse = Input::mouse_position;
	Game* g = Game::instance;
	Camera* cam = g->camera;
	Vector3 dir = cam->getRayDirection(mouse.x, mouse.y, g->window_width, g->window_height);

	return dir;
}

Vector3 Scene::getRayOrigin() {

	Game* g = Game::instance;
	Camera* cam = g->camera;
	Vector3 rayOrigin = cam->eye;

	return rayOrigin;
}