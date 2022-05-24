#pragma once

#include "entity.h"
#include "stage.h"


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
	EntityMesh* skyMesh = NULL;
	EntityMesh* groundMesh = NULL;
	bool cameraLocked = true; //util para debug
	sPlayer player;
	bool firstPerson = false; //cambiar a true para vista en primera persona
	//props of our game
	Prop props[20];
	//Entity* root;
	EntityMesh* selectedEntity = NULL;
	STAGE_ID currentStage = STAGE_ID::PLAY;
	std::vector<Stage*> stages;
	std::vector<EntityMesh*> entities;
	std::vector<Entity*> s_to_destroy;//destroy vector
	//poner todas las entities
	// 
	//metodos como entidades dentro de area..
	void addEntityOnFront();
	void testCollisionOnFront();
	void rotateSelected(float angleDegree);
	Vector3 getRayDir();
	Vector3 getRayOrigin();
};


