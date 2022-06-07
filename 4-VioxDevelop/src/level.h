#pragma once
#include "entity.h"
#include "camera.h"

enum EDITOR_ID {
	GROUND = 0,
	SKY = 1,
	OBJECT = 2,
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
	
	EntityMesh* selectedEntity = NULL;
	int selectedEntityPos = 0;
	EDITOR_ID currentOption = GROUND;

	void Render();
	void Update(float seconds_elapsed);
	void removeEntities();
	void removeSelected();
	void addEntityOnFront();
	void testCollisionOnFront();
	void rotateSelected(float angleDegree);
	Vector3 getRayDir();
	Vector3 getRayOrigin();
};

class PlayLevel : public Level {
public:
	
	sPlayer* player;
	std::vector<sPlayer*> enemies;

	void Render();
	void Update(float seconds_elapsed);

};

class MultiLevel : public Level{
public:
	
	sPlayer* player1;
	sPlayer* player2;
	Camera* cam1;
	Camera* cam2;

	void Render();
	void Update(float seconds_elapsed);

};

void InitLevels(std::vector<PlayLevel*>& levels, EditorLevel*& editor, MultiLevel*& multi);