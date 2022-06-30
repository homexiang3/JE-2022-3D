#pragma once
#include "entity.h"
#include "camera.h"

enum EDITOR_ID {
	GROUND = 0,
	SKY = 1,
	OBJECT = 2,
	EDIT = 3
};

class Level {
public:
	
	EntityMesh* groundMesh = NULL;
	EntityMesh* skyMesh = NULL;
	std::vector<EntityMesh*> entities;

	virtual void Render() = 0;
	virtual void Update(float seconds_elapsed) = 0;
};

class EditorLevel : public Level{
public:
	EditorLevel();
	EntityMesh* selectedEntity = NULL;
	int selectedEntityPos = 0;
	EDITOR_ID currentOption = GROUND;

	void Render();
	void Update(float seconds_elapsed);
	void removeEntities();
	void clearEditor();
	void removeSelected();
	void addEntityOnFront(const char* mesh, const char* tex);
	void testCollisionOnFront();
	void rotateSelected(float angleDegree);
	Vector3 getRayDir();
	Vector3 getRayOrigin();
};

class PlayLevel : public Level {
public:
	PlayLevel(const char* map, const char* enemiesPath);

	sPlayer* player = NULL;
	std::vector<sPlayer*> enemies;
	std::vector<sPlayer*> enemies_defeated;
	Camera* cam = NULL;
	// boss
	sBoss* boss = NULL;
	bool hasBoss = false;
	bool isOver = false;
	Texture* quadTex;
	Mesh playerHP_quad;
	Camera cam2D;
	void Render();
	void Update(float seconds_elapsed);
	void resetLevel();

};

class MultiLevel : public Level{
public:
	MultiLevel();

	sPlayer* player1 = NULL;
	sPlayer* player2 = NULL;
	Camera* cam1 = NULL;
	Camera* cam2 = NULL;

	void Render();
	void Update(float seconds_elapsed);
	void RenderWorld(Camera* cam);
	void resetLevel();

	//hp bars
	Texture* quadTex;
	Mesh player1HP_quad;
	Mesh player2HP_quad;
	Camera cam2D;

	bool isOver = false;

};


void InitLevels(std::vector<PlayLevel*>& levels, EditorLevel*& editor, MultiLevel*& multi);
void RenderMinimap(int widthStart, sPlayer*& player, std::vector<sPlayer*>& enemies, EntityMesh* groundMesh, std::vector<EntityMesh*>& entities);
void SetupCam(Matrix44& playerModel, Camera* cam);
void drawHP(Mesh quad, Texture* tex, Matrix44 anim, Camera cam2D);
void updateHealthBar(float centerStart, Mesh& playerHP_quad, sPlayer* player);
