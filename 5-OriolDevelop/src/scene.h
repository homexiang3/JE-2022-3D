#pragma once

#include "entity.h"
//estructura para particula (fisicas)
struct sParticle {
	Vector3 pos;
	Vector3 vel;
	float radius = 10.0f;
};
//estructura para el player
struct sPlayer {
	Vector3 pos;
	float yaw = 0.0f;
	float pitch = 0.0f; //para el first person
	int health;
	EntityMesh* character_mesh;
	Vector2 dash_direction;
	float jumpLock;
};
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
	EntityMesh* playerEntity = NULL;
	Matrix44 bombOffset;
	EntityMesh* bombMesh = NULL;
	EntityMesh* skyMesh = NULL;
	EntityMesh* groundMesh = NULL;
	bool cameraLocked = true; //util para debug
	bool bombAttached = true;
	sPlayer player;
	bool firstPerson = false; //cambiar a true para vista en primera persona
	const int numParticles = 100; //particulas para testear fisicas
	std::vector<sParticle*> particles;
	std::vector<EntityMesh*> particlesMesh;
	//props of our game
	Prop props[20];
	//Entity* root;
	EntityMesh* selectedEntity = NULL;
	std::vector<EntityMesh*> entities;
	std::vector<Entity*> s_to_destroy;//destroy vector
	//poner todas las entities
	//metodos como entidades dentro de area..
	void addEntityOnFront();
	void testCollisionOnFront();
	void rotateSelected(float angleDegree);
	Vector3 getRayDir();
	Vector3 getRayOrigin();
};
