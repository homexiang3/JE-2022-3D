#pragma once

#include "entity.h"
#include "stage.h"
#include "audio.h"
#include "camera.h"
#include "level.h"


//Clase para guardar diferentes tipos de texturas y meshes utiles para reutilizar dentro de nuestro juego por ejemplo cosas del entorno que se repiten
class Prop
{
public:
	Prop();
	~Prop();
	int id;
	Mesh* mesh;
	Texture* texture;

};

//Clase similar a "world" que se encarga de guardar todos los elementos del mundo (ej. variables globales ) para despues pintarlos en pantalla 
class Scene
{
public:
	Scene();
	~Scene();
	//globals movidos de game (quizas se acaban borrando)
	//bool cameraLocked = true; //util para debug
	Audio* audio = NULL;
	//props of our game
	Prop props[20];
	//Entity* root;
	STAGE_ID currentStage = STAGE_ID::INTRO;
	int currentLevel = 0;
	std::vector<Stage*> stages;
	std::vector<PlayLevel*> levels;
	EditorLevel* editor;
	MultiLevel* multi;

	//audio
	bool music = false;
	bool music_Playing = false;

	void saveProgress();
	void loadProgress();

};
//cargar mapa
Vector3 ReadVector3(std::stringstream& ss);
void ImportMap(const char* path, std::vector<EntityMesh*>& entities, EntityMesh*& groundMesh, EntityMesh*& skyMesh);
void ExportMap(std::vector<EntityMesh*>& entities, EntityMesh* groundMesh, EntityMesh* skyMesh);
void ImportEnemies(const char* path, std::vector<sPlayer*>& enemies, bool& isBoss);
//utils
Vector3 Lerp(Vector3 a, Vector3 b, float t);
float sign(float value);
void renderGUI(float x, float y, float w, float h, Texture* tex, bool flipXV);

