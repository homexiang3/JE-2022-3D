#pragma once

#include "framework.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "camera.h"
#include "animation.h"

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

	//anims
	Animation* anim = NULL;

	//methods overwritten 
	void render( Camera* camera );
	void update(float dt);
};
//estructura para guardar las zonas de collision
struct Collider {
	const char* name = NULL;
	EntityMesh* colliderMesh = NULL;

	void updateCollider(EntityMesh* playerMesh, Camera* camera);
};
//estructura para el player
struct sPlayer {
	sPlayer(const char* meshPath, const char* texPath, Vector3 spawn);

	Vector3 spawnPos;
	Vector3 pos;
	//Vector3 vel;
	float playerVel;
	float yaw = 0.0f;
	//float pitch = 0.0f; //para el first person
	//colliders
	std::vector<Collider*> colliders;
	float radius = 0.5f; //por si queremos hacer bounding con collisions (se usa en player collision y colliders)
	int max_health = 10;
	int health = 10;
	EntityMesh* character_mesh = NULL;
	Vector2 dash_direction;
	float max_invulnerability_time = 1.5f;
	float invulnerability_time = 0.0f;
	//anims
	std::vector<Animation*> anims;
	int ctr = 0;
	float animTimer = 0.0f;
	int side = 1;

	//escudo
	EntityMesh* shield = NULL;
	bool Protected = false;
	//bool shield = true;

	//animations
	Animation* idle = NULL;
	Animation* walk = NULL;
	Animation* run = NULL;
	Animation* left_puch = NULL;
	Animation* kick = NULL;
	Animation* dash = NULL;
	Animation* jump = NULL;
	Animation* protect = NULL;
	Animation* hit = NULL;


	Matrix44 getModel();
	void initAnims();
	void initColliders();
	void playerMovement(std::vector<sPlayer*> enemies, std::vector<EntityMesh*> entities, float seconds_elapsed, bool multi);
	Vector3 playerCollision(std::vector<sPlayer*> enemies, std::vector<EntityMesh*> entities, Vector3 nextPos, float seconds_elapsed);
	void npcMovement(std::vector<sPlayer*> enemies, std::vector<EntityMesh*> entities,sPlayer* player, float seconds_elapsed);
	void ChangeAnim(int i, float time);
	Animation* renderAnim();
	void punchCollision(std::vector<sPlayer*> enemies);
	void kickCollision(std::vector<sPlayer*> enemies);
	void updateInvulnerabilityTime(float seconds_elapsed);
	
};

struct sBoss {
	sBoss(const char* meshPath, const char* texPath);

	Vector3 spawnPos;
	Vector3 pos;
	//Vector3 vel;
	float playerVel = 4.0f;
	float yaw = 0.0f;
	//float pitch = 0.0f; //para el first person
	float radius = 0.5f; //por si queremos hacer bounding con collisions (se usa en player collision)
	int health = 10;
	EntityMesh* character_mesh;

	//anims
	std::vector<Animation*> anims;
	int ctr = 1;
	float animTimer = 0.0f;
	int side = 1;

	//weapons
	EntityMesh* shuriken_mesh;

	Animation* idle = NULL;
	Animation* rest = NULL;
	Animation* run = NULL;

	Animation* die = NULL;
	Animation* charge = NULL;

	Animation* attackRect = NULL;
	Animation* attackCone = NULL;
	Animation* attackCircle = NULL;

	Animation* shurikenSpawn = NULL;

	bool animShurikens = false;

	float testPositioner = 1.0f;
	float testPositioner2 = 1.0f;
	float testPositioner3 = 1.0f;
	bool xd = true;

	float attackTimer = 0.0f; //timer to start attack
	int state = 0; //flag for boss state: 0 for free , 1 for attacking 
	int attackNumb = 0;
	Vector3 attackTo;

	//attacks 
	std::vector<EntityMesh*> planes;
	int attackCounter = 0;//every 3 atatcks the boss will rest

	//destroyed shurikens
	bool shurkikens[10] = { false,false,false,false,false,false,false,false,false, false };

	//to comunnicate hits to the level class ? 
	bool hit = false;

	//player invencibility timer
	float invencibility = 0.0f;

	Matrix44 getModel();
	void initAnims();
	//void playerMovement(std::vector<sPlayer*> enemies, std::vector<EntityMesh*> entities, float seconds_elapsed, bool multi);
	//Vector3 playerCollision(std::vector<sPlayer*> enemies, std::vector<EntityMesh*> entities, Vector3 nextPos, float seconds_elapsed);
	void npcMovement(sPlayer* player, float seconds_elapsed);
	void ChangeAnim(int i, float time);
	Animation* renderAnim();

	void shurikenAttack(Camera* cam, Vector3 playerpos);

	void katanaRender(Camera* cam);

	void Attack(Camera* cam, Vector3 playerpos);

};