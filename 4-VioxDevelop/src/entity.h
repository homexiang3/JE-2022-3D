#pragma once

#include "framework.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "camera.h"

//clase base de la entidad

class Entity
{
public:
	Entity(); //constructor
	virtual ~Entity(); //destructor

	//some attributes
	std::string name;
	Matrix44 model;
	Matrix44 global_model;
	BoundingBox aabb;
	//bool recalculate; para recalcular el bounding box cuando se mueva el objeto
	//bool destroyed = false if we want to ignore actions for destroyed entities on destroyed vectors

	//methods overwritten
	virtual void render(Camera* camera);
	virtual void update(float elapsed_time);

	//useful methods
	Vector3 getPosition();
	void destroy(std::vector<Entity*> s_to_destroy);
	

	//arbol de escena
	Entity* parent;
	std::vector<Entity*> children;//vector of children
	void addChild(Entity* ent);
	void removeChild(Entity* ent);
	Matrix44 getGlobalMatrix();
};

//clase para de momento guardar los meshes que vayamos cargando

class EntityMesh : public Entity {
public:
	EntityMesh(int primitive, std::string meshPath, std::string texturePath, char* shaderPath, char* shaderPath2, Vector4 color);
	//Attributes of this class 
	int primitive;
	Mesh* mesh = NULL;
	Texture* texture = NULL;
	Shader* shader = NULL; //si acabamos utilizando uno se podria opitimizar mucho quitandolo de aqui y llamarlo solo en el render de game
	Vector4 color;
	std::string meshPath;
	std::string texturePath;
	float tiling;

	//methods overwritten 
	void render(Camera* camera);
	void update(float dt);
};

//estructura para el player
struct sPlayer {
	Vector3 spawnPos;
	Vector3 pos;
	Vector3 vel;
	float yaw = 0.0f;
	float pitch = 0.0f; //para el first person
	float radius = 10.0f; //por si queremos hacer bounding con colisions
	int health;
	EntityMesh* character_mesh;
	Vector2 dash_direction;
	float jumpLock;

	Matrix44 getModel();
	void playerMovement(std::vector<EntityMesh*> entities, float seconds_elapsed, float rotSpeed, float playerSpeed);
	Vector3 playerCollision(std::vector<EntityMesh*> entities, Vector3 nextPos, float seconds_elapsed);
	void npcMovement(sPlayer* player, float seconds_elapsed);
	
};

/*otros ejemplos

class EntityCamera : public Entity {};
class EntityLight : public Entity {};
class EntitySound : public Entity {};
class EntityTrigger : public Entity {};

class Airplane : public Entity
{
	 public:
	static std::vector<Airplane*> planes;

	char type;

	Airplane() {
planes.push_back(this);
}


~Airplane() {
auto it = planes.find(this);
planes.remove(it);
}

static void renderAll();
static void updateAll( float dt );
};

//guardar informacion de cada "material" dentro de un vector

class PropType {
	int index;
	Mesh* mesh;
	Texture* texture;
	//...
};

//array of types (max 32 types)
PropType proptypes[32];
class Prop {
	public:
		int type;	//index to the array
		Matrix44 model;
};
*/