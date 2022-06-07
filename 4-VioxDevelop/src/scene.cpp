#include "scene.h"
#include "game.h"
#include "input.h"
#include <fstream>
#include <string>
#include <iostream>
#include "level.h"

Scene::Scene()
{
	//init stages
	InitStages(stages);

	//init levels
	InitLevels(levels, editor, multi);
	
	//charge audio
	this->audio = new Audio();
	//load entities
	this->player.character_mesh = new EntityMesh(GL_TRIANGLES, "data/skelleton.obj", "data/minichar_tex.png", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));

	//get objects
	ImportMap("data/maps/map1.scene", this->editor->entities, this->editor->groundMesh, this->editor->skyMesh);
	
	
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


Vector3 Scene::Lerp(Vector3 a, Vector3 b, float t) {
	t = clamp(t, 0.0f, 1.0f);
	Vector3 ab = b - a;
	return a + (ab * t);
}


//map functions
Vector3 ReadVector3(std::stringstream& ss) {
	Vector3 vec;
	ss >> vec.x;
	if (ss.peek() == ',') ss.ignore();
	ss >> vec.y;
	if (ss.peek() == ',') ss.ignore();
	ss >> vec.z;
	return vec;
}
void ImportMap(const char* path, std::vector<EntityMesh*>& entities, EntityMesh*& groundMesh, EntityMesh*& skyMesh) {
	//clear entities before import new map
	//this->removeEntities();

	//prepare file content
	std::string content = "";
	readFile(path, content);
	std::stringstream ss(content);
	
	//read ground mesh
	std::string groundTex;
	ss >> groundTex;
	float tiling;
	ss >> tiling;
	EntityMesh* ground = new EntityMesh(GL_TRIANGLES, "", groundTex, "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	ground->tiling = tiling;
	ground->mesh = new Mesh();
	ground->mesh->createPlane(1000); //un poco hardcoded pero funcional
	groundMesh = ground;

	//read sky mesh
	std::string skyMeshPath;
	ss >> skyMeshPath;
	std::string skyTex;
	ss >> skyTex;
	Vector3 pos = ReadVector3(ss);
	Vector3 rot = ReadVector3(ss);
	//create entity
	EntityMesh* sky = new EntityMesh(GL_TRIANGLES, skyMeshPath, skyTex, "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	Matrix44 model;
	model.translate(pos.x, pos.y, pos.z);
	model.setFrontAndOrthonormalize(rot);
	sky->model = model;
	skyMesh = sky;

	while (!ss.eof()) {
		//read values
		std::string meshPath;
		ss >> meshPath;
		std::string texPath;
		ss >> texPath;
		Vector3 pos = ReadVector3(ss);
		Vector3 rot = ReadVector3(ss);
		//create entity
		EntityMesh* entity = new EntityMesh(GL_TRIANGLES, meshPath, texPath, "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
		Matrix44 model;
		model.translate(pos.x, pos.y, pos.z);
		model.setFrontAndOrthonormalize(rot);
		entity->model = model;

		entities.push_back(entity);
	}
	std::cout << " + Success: Level imported" << std::endl;
}
void ExportMap(std::vector<EntityMesh*>& entities, EntityMesh* groundMesh, EntityMesh* skyMesh) {
	std::ofstream myfile;
	std::string prefix = "data/maps/";
	std::string mapPath = "";
	std::cout << " + Please enter where do you want to export the file: ";
	std::cin >> mapPath;
	mapPath = prefix + mapPath;
	std::cout << " + Exporting to "<< mapPath << std::endl;

	myfile.open(mapPath);

	if (!myfile) { // file couldn't be opened
		std::cout << " + Error: exported map could not be opened" << std::endl;
		return;
	}

	if (groundMesh == NULL) {
		std::cout << " + Error: exported map needs a ground" << std::endl;
		return;
	}
	else {
		EntityMesh* ground = groundMesh;
		std::string texPath = ground->texturePath;
		float tiling = ground->tiling;

		if (entities.size() == 0) {
			myfile << texPath << " " << tiling;
		}
		else {
			myfile << texPath << " " << tiling << "\n";
		}
	}

	if (skyMesh == NULL) {
		std::cout << " + Error: exported map needs a sky" << std::endl;
		return;
	}
	else {

		EntityMesh* sky = skyMesh;
		std::string meshPath = sky->meshPath;
		std::string texPath = sky->texturePath;

		Vector3 pos = sky->model.getTranslation();
		//esta bien la rotacion?
		Vector3 rot = sky->model.frontVector().normalize();


		if (entities.size() == 0) {
			myfile << meshPath << " " << texPath << " " << pos.x << "," << pos.y << "," << pos.z << " " << rot.x << "," << rot.y << "," << rot.z;
		}
		else {
			myfile << meshPath << " " << texPath << " " << pos.x << "," << pos.y << "," << pos.z << " " << rot.x << "," << rot.y << "," << rot.z << "\n";
		}
	}

	for (size_t i = 0; i < entities.size(); i++)
	{
		EntityMesh* entity = entities[i];

		std::string meshPath = entity->meshPath;
		std::string texPath = entity->texturePath;

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
	std::cout << " + Success: Level exported" << std::endl;
	myfile.close();
}