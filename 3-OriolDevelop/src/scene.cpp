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

	if (BASS_Init(-1, 44100, 0, 0, NULL) == false) { //-1 significa usar el por defecto del sistema operativo
		std::cout << "ERROR initializing audio" << std::endl;
	}
	audio->LoadSample("data/music/sfx_enter.wav",1); // 1 to be loaded in the in-game samples
	
	// menu samples 
	audio->LoadSample("data/music/intro_music.wav", 2);// 2 to be loaded into the menu sounds
	audio->LoadSample("data/music/sfx_menu.wav", 2);
	audio->LoadSample("data/music/sfx_enter.wav", 2);
	
		
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


Vector3 Lerp(Vector3 a, Vector3 b, float t) {
	t = clamp(t, 0.0f, 1.0f);
	Vector3 ab = b - a;
	return a + (ab * t);
}

float sign(float value) {
	return value >= 0.0f ? 1.0f : -1.0f;
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

	//read entities
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
	std::cout << " + Success: Level imported from " << path << std::endl;
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

void ImportEnemies(const char* path, std::vector<sPlayer*>& enemies) {
	
	//prepare file content
	std::string content = "";
	readFile(path, content);
	std::stringstream ss(content);

	//read entities
	while (!ss.eof()) {
		//read values
		std::string meshPath;
		ss >> meshPath;
		std::string texPath;
		ss >> texPath;
		float speed;
		ss >> speed;
		int health;
		ss >> health;
		//create entity
		sPlayer* enemy = new sPlayer("data/boss/boss.mesh", texPath.c_str());
		enemy->playerVel = speed;
		enemy->health = health;
		//definir la pos random?

		enemies.push_back(enemy);
	}
	std::cout << " + Success: enemies imported from "<< path << std::endl;

}