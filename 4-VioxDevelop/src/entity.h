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
	EntityMesh(int primitive, std::string meshPath, std::string texturePath, char* shaderPath, char* shaderPath2, Vector4 color);
	//Attributes of this class 
	int primitive;
	Mesh* mesh;
	Texture* texture;
	Shader* shader; //si acabamos utilizando uno se podria opitimizar mucho quitandolo de aqui y llamarlo solo en el render de game
	Vector4 color;
	std::string meshPath;
	std::string texturePath;
	float tiling;

	//methods overwritten 
	void render();
	void update(float dt);
};

//estructura para el player
struct sPlayer {
	Vector3 pos;
	Vector3 vel;
	float yaw = 0.0f;
	float pitch = 0.0f; //para el first person
	float radius = 10.0f; //por si queremos hacer bounding con colisions
	int health;
	EntityMesh* character_mesh;
	Vector2 dash_direction;
	float jumpLock;

	Matrix44 getModel() {
		Matrix44 model;
		model.translate(pos.x, pos.y, pos.z);
		model.rotate(yaw * DEG2RAD, Vector3(0, 1, 0));
		this->character_mesh->model = model;
		return  model;
	}
	
	Vector3 playerCollision(std::vector<EntityMesh*> entities, Vector3 nextPos, float seconds_elapsed) {
		//TEST COLLISIONS, HABRIA QUE TENER DINAMICAS - ESTATICAS, DINAMICAS - DINAMICAS, PLAYER - COSAS ETC...
		//calculamos el centro de la esfera de colisión del player elevandola hasta la cintura
		Vector3 character_center = nextPos + Vector3(0, 1, 0);

		//para cada objecto de la escena...
		for (size_t i = 0; i < entities.size(); i++)
		{
			EntityMesh* currentEntity = entities[i];
			//comprobamos si colisiona el objeto con la esfera (radio 3)
			Vector3 coll;
			Vector3 collnorm;
			if (!currentEntity->mesh->testSphereCollision(currentEntity->model, character_center, 0.5f, coll, collnorm))
				continue; //si no colisiona, pasamos al siguiente objeto

			//si la esfera está colisionando muevela a su posicion anterior alejandola del objeto
			Vector3 push_away = normalize(coll - character_center) * seconds_elapsed;
			nextPos = this->pos - push_away; //move to previous pos but a little bit further

			//cuidado con la Y, si nuestro juego es 2D la ponemos a 0
			nextPos.y = 0;

			//reflejamos el vector velocidad para que de la sensacion de que rebota en la pared
			//velocity = reflect(velocity, collnorm) * 0.95;

			return nextPos;
		}
		
		return nextPos;
	}
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