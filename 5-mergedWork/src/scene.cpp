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
	audio->LoadSample("data/music/sfx_enter.mp3", 1); // 1 to be loaded in the in-game samples //0

	audio->LoadSample("data/music/sfx_hit.mp3", 1);//1
	audio->LoadSample("data/music/sfx_youwin.mp3", 1);//2

	audio->LoadSample("data/music/dash.wav", 1);//3
	audio->LoadSample("data/music/punch.wav", 1);//4
	audio->LoadSample("data/music/kick.wav", 1);//5

	audio->LoadSample("data/music/Level4Song.mp3", 1);//6
	audio->LoadSample("data/music/Level3Song.mp3", 1);//7
	audio->LoadSample("data/music/Level2Song.mp3", 1);//8
	audio->LoadSample("data/music/Level1Song.mp3", 1);//9
	
	// menu samples 
	audio->LoadSample("data/music/intro_music.mp3", 2);// 2 to be loaded into the menu sounds
	audio->LoadSample("data/music/sfx_menu.mp3", 2);
	audio->LoadSample("data/music/sfx_enter.mp3", 2);
	
		
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

void ImportEnemies(const char* path, std::vector<sPlayer*>& enemies, bool& isBoss) {
	
	//prepare file content
	std::string content = "";
	readFile(path, content);
	std::stringstream ss(content);

	//read if boss
	int boss;
	ss >> boss;
	if (boss >= 1) {
		isBoss = true;
	}

	std::cout << isBoss << std::endl;
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
		Vector3 pos = ReadVector3(ss);
		//create entity
		sPlayer* enemy = new sPlayer(meshPath.c_str(), texPath.c_str(),pos);
		enemy->playerVel = speed; //modify char default speed
		enemy->max_health = health; //modify char default health
		enemy->health = health;
		enemies.push_back(enemy);
	}
	std::cout << " + Success: enemies imported from "<< path << std::endl;

}

void renderGUI(float x, float y, float w, float h, Texture* tex, bool flipYV) {
	int windowWidth = Game::instance->window_width;
	int windowHeight = Game::instance->window_height;
	Mesh quad;
	quad.createQuad(x, y, w, h, flipYV);

	Camera cam2D;
	cam2D.setOrthographic(0, windowWidth, windowHeight, 0, -1, 1);
	//assert(mesh != NULL, "mesh in renderMesh was null");

	Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	if (!shader) return;

	//enable shader
	shader->enable();

	//upload uniforms
	shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	shader->setUniform("u_viewprojection", cam2D.viewprojection_matrix);
	if (tex != NULL)
	{
		shader->setUniform("u_texture", tex, 0);
	}
	shader->setUniform("u_time", Game::instance->time);
	shader->setUniform("u_text_tiling", 1.0f);
	Matrix44 quadModel;
	//quadModel.translate(sin(Game::instance->time ) * 20, 0, 0);
	shader->setUniform("u_model", quadModel);
	//do the draw call
	quad.render(GL_TRIANGLES);

	//disable shader
	shader->disable();
}

void Scene::saveProgress()
{

}

void Scene::loadProgress()
{

}