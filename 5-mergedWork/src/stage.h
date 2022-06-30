#ifndef STAGE_H
#define STAGE_H

#include <vector>
#include "mesh.h"
#include "camera.h"
#include "texture.h"
#include "framework.h"

enum STAGE_ID {
	INTRO = 0,
	TUTORIAL = 1,
	PLAY = 2,
	EDITOR = 3,
	MULTI = 4,
	END = 5
};


class Stage {
public:
	virtual STAGE_ID GetId() = 0;
	virtual void Render() = 0;
	virtual void Update(float seconds_elapsed) = 0;
};

struct textStruct {
	char* txt;
	Vector2 pos;
	float scale;
};

class IntroStage : public Stage {
public:
	STAGE_ID GetId() { return STAGE_ID::INTRO; };
	
	void Render();
	void Update(float seconds_elapsed);
	
	//Mesh xd= xd.createQuad(100,100,100,100,true);
	
	int initSize =0;

	//GUI
	//void moveQuad(Mesh* quad, float center_x, float center_y, float w, float h);

	int menuPointer=0;
	Texture* quadTex;
	Texture* menuPointer_tex;
	Mesh menuPtr_mesh;
	std::vector<Mesh> quads;
	Camera cam2D;
	std::vector<textStruct> tags;

	Texture* bgtex = Texture::Get("data/bg2.png");
	Texture* texIntro = Texture::Get("data/menu/Intro.png");

	void initQuads();
	IntroStage() {
		initQuads();
	};

	void renderQuad(Mesh quad, Texture* tex, Matrix44 anim);
	void movePtr();
};

class TutorialStage : public Stage {
public:
	Texture* tex = Texture::Get("data/tuto.png");
	Texture* texBack = Texture::Get("data/menu/back.png");

	STAGE_ID GetId() { return STAGE_ID::TUTORIAL; };
	void Render();
	void Update(float seconds_elapsed);

};

class PlayStage : public Stage {
public:
	STAGE_ID GetId() { return STAGE_ID::PLAY; };
	void Render();
	void Update(float seconds_elapsed);

};

class EditorStage : public Stage {
public:
	STAGE_ID GetId() { return STAGE_ID::EDITOR; };
	void Render();
	void Update(float seconds_elapsed);

};

class MultiStage : public Stage {
public:
	STAGE_ID GetId() { return STAGE_ID::MULTI; };
	void Render();
	void Update(float seconds_elapsed);

};

class EndStage : public Stage {
public:
	STAGE_ID GetId() { return STAGE_ID::END; };
	void Render();
	void Update(float seconds_elapsed);

};

Stage* GetStage(STAGE_ID id, std::vector<Stage*>& stages);
Stage* GetCurrentStage(STAGE_ID currentStage, std::vector<Stage*>& stages);
void SetStage(STAGE_ID id, STAGE_ID &currentStage);

void InitStages(std::vector<Stage*>& stages); 

#endif
