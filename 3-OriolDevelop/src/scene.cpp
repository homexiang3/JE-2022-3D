#include "scene.h"
#include "game.h"
#include "input.h"
#include <fstream>
#include <string>
#include <iostream>


Scene::Scene()
{
	//init stages
	InitStages(stages);
	this->player.initAnims();
	//load entities
	this->player.character_mesh = new EntityMesh(GL_TRIANGLES, "data/skelly.mesh", "data/minichar_tex.png", "data/shaders/skinning.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	this->skyMesh = new EntityMesh(GL_TRIANGLES, "data/cielo.ASE", "data/cielo.tga", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	//load grass/ground
	this->groundMesh = new EntityMesh(GL_TRIANGLES, NULL, "data/grass.tga", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	this->groundMesh->tiling = 200.0f;
	this->groundMesh->mesh = new Mesh();
	this->groundMesh->mesh->createPlane(1000);

	//get objects
	ImportMap("data/maps/map1.scene", entities);
	
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

void Scene::testCollisionOnFront() { //revisar?

	Vector3 dir = getRayDir();
	Vector3 rayOrigin = getRayOrigin();
	float min_dis = 10000.0f;
	Camera* cam = Game::instance->camera;

	for (size_t i = 0; i < this->entities.size(); i++)
	{
		EntityMesh* entity = entities[i];
		Vector3 pos;
		Vector3 normal;
		if (entity->mesh->testRayCollision(entity->model, rayOrigin, dir, pos, normal)) {

			Vector3 entityPos = entity->model.getTranslation();
			float dis = cam->eye.distance(entityPos);
			if(dis < min_dis) selectedEntity = entity;

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

//map functions
Vector3 Scene::ReadVector3(std::stringstream& ss) {
	Vector3 vec;
	ss >> vec.x;
	if (ss.peek() == ',') ss.ignore();
	ss >> vec.y;
	if (ss.peek() == ',') ss.ignore();
	ss >> vec.z;
	return vec;
}
void Scene::ImportMap(const char* path, std::vector<EntityMesh*>& entities) {
	std::string content = "";
	readFile(path, content);
	std::stringstream ss(content);

	while (!ss.eof()) {
		//read values
		std::string meshPath;
		ss >> meshPath;
		std::string texPath;
		ss >> texPath;
		Vector3 pos = ReadVector3(ss);
		Vector3 rot = ReadVector3(ss);
		//create entity
		EntityMesh* entity = new EntityMesh(GL_TRIANGLES, meshPath.c_str() , texPath.c_str(), "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
		Matrix44 model;
		model.translate(pos.x, pos.y, pos.z);
		model.setFrontAndOrthonormalize(rot);
		entity->model = model;

		entities.push_back(entity);
 	}
}
void Scene::ExportMap(std::vector<EntityMesh*>& entities) {
	std::ofstream myfile;
	myfile.open("data/maps/map1.scene");

	if (!myfile) { // file couldn't be opened
		std::cout << "Error: exported map could not be opened" << std::endl;
		return;
	}

	for (size_t i = 0; i < entities.size(); i++)
	{
		EntityMesh* entity = entities[i];

		const char* meshPath = entity->meshPath;
		const char* texPath = entity->texturePath;

		Vector3 pos = entity->model.getTranslation();
		//esta bien la rotacion?
		Vector3 rot = entity->model.frontVector().normalize();
		

		if (i == entities.size() - 1) {
			myfile << meshPath << " " << texPath << " " << pos.x << "," << pos.y << "," << pos.z << " " << rot.x << "," << rot.y << "," << rot.z;
		}
		else {
			myfile << meshPath << " " << texPath << " " << pos.x << "," << pos.y << "," << pos.z << " " << rot.x << "," << rot.y << "," << rot.z << "\n";
		}
	}

	myfile.close();
}

Vector3 Scene::Lerp(Vector3 a, Vector3 b, float t) {
	t = clamp(t, 0.0f, 1.0f);
	Vector3 ab = b - a;
	return a + (ab * t);
}