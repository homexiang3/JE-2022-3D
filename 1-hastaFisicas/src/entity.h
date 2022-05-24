#pragma once

#include "framework.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"

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
	virtual void render();
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
	EntityMesh(int primitive, char* meshPath, char* texturePath, char* shaderPath, char* shaderPath2, Vector4 color);
	//Attributes of this class 
	int primitive;
	Mesh* mesh;
	Texture* texture;
	Shader* shader; //si acabamos utilizando uno se podria opitimizar mucho quitandolo de aqui y llamarlo solo en el render de game
	Vector4 color;
	float tiling;

	//methods overwritten 
	void render();
	void update(float dt);
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