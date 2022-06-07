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
	bool cameraLocked = true; //util para debug
	Audio* audio = NULL;
	bool firstPerson = false; //cambiar a true para vista en primera persona
	//props of our game
	Prop props[20];
	//Entity* root;
	STAGE_ID currentStage = STAGE_ID::EDITOR;
	std::vector<Stage*> stages;
	std::vector<PlayLevel*> levels;
	EditorLevel* editor;
	MultiLevel* multi;
	//cosas que van en level
	std::vector<EntityMesh*> entities;
	std::vector<Entity*> s_to_destroy;//destroy vector
	EntityMesh* skyMesh = NULL;
	EntityMesh* groundMesh = NULL;

	sPlayer player;

	
	//utils
	Vector3 Lerp(Vector3 a, Vector3 b, float t);


};
//cargar mapa
Vector3 ReadVector3(std::stringstream& ss);
void ImportMap(const char* path, std::vector<EntityMesh*>& entities, EntityMesh*& groundMesh, EntityMesh*& skyMesh);
void ExportMap(std::vector<EntityMesh*>& entities, EntityMesh* groundMesh, EntityMesh* skyMesh);


