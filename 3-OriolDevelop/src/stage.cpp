#include "stage.h"
#include "game.h"
#include "input.h"

//STAGE UTIL FUNCTIONS

Stage* GetStage(STAGE_ID id, std::vector<Stage*>& stages) { return stages[(int)id]; };
Stage* GetCurrentStage(STAGE_ID currentStage, std::vector<Stage*>& stages) { return GetStage(currentStage, stages); };
void SetStage(STAGE_ID id, STAGE_ID &currentStage) { currentStage = id; };



void InitStages(std::vector<Stage*>& stages) {
	stages.reserve(6);
	stages.push_back(new IntroStage());
	stages.push_back(new TutorialStage());
	stages.push_back(new PlayStage());
	stages.push_back(new EditorStage());
	stages.push_back(new MultiStage());
	stages.push_back(new EndStage());
}

//INTRO STAGE

void IntroStage::movePtr() {
	int window_width = Game::instance->window_width;
	int window_height = Game::instance->window_height;

	int i = this->menuPointer;
	float x = window_width / 2;
	float w1 = window_width / 1.5;
	float w = 100 * window_width / 800;
	float h = 75 * window_height / 600;

	float center_x = x -( w1 / 1.7);
	float center_y = window_height / 3 + i * window_height / 8;
	
	
	this->menuPtr_mesh.vertices.clear();
	
	//create six vertices (3 for upperleft triangle and 3 for lowerright)

	this->menuPtr_mesh.vertices.push_back(Vector3(center_x + w * 0.5f, center_y + h * 0.5f, 0.0f));
	this->menuPtr_mesh.vertices.push_back(Vector3(center_x - w * 0.5f, center_y - h * 0.5f, 0.0f));
	this->menuPtr_mesh.vertices.push_back(Vector3(center_x + w * 0.5f, center_y - h * 0.5f, 0.0f));
	this->menuPtr_mesh.vertices.push_back(Vector3(center_x - w * 0.5f, center_y + h * 0.5f, 0.0f));
	this->menuPtr_mesh.vertices.push_back(Vector3(center_x - w * 0.5f, center_y - h * 0.5f, 0.0f));
	this->menuPtr_mesh.vertices.push_back(Vector3(center_x + w * 0.5f, center_y + h * 0.5f, 0.0f));
}

void IntroStage::initQuads()
{

	this->quadTex = Texture::Get("data/menu/menubu.tga");
	this->menuPointer_tex = Texture::Get("data/menu/boxingPointer.png");
	char* texts[5] = { "Tutorial","Single player", "Editor mode", "Multiplayer versus", "Exit" };
	int window_width = Game::instance->window_width;
	int window_height = Game::instance->window_height;
	this->cam2D.setOrthographic(0, window_width, window_height, 0, -1, 1);
	for (int i = 0; i < 5; i++)
	{
		float x = window_width / 2;
		float y = window_height / 3 + i * window_height / 8;
		float w = window_width / 1.5;
		float h = window_height / 12;

		Mesh quad;
		quad.createQuad(x, y, w, h, true);
		this->quads.push_back(quad);

		textStruct txt;
		txt.txt = texts[i];
		txt.scale = window_width / 400;
		txt.pos = Vector2(x - strlen(txt.txt) * (3 + txt.scale), y - 10);
		this->tags.push_back(txt);
	}
	float x = window_width / 2 - (window_width / 1.5) / 1.7;
	float y = window_height / 3;
	float w = 100 * window_width / 800;
	float h = 75 * window_height / 600;
	this->menuPtr_mesh.createQuad(x, y, w, h, true);
}

void IntroStage::renderQuad(Mesh quad, Texture* tex, Matrix44 anim = Matrix44())
{
	Shader* a_shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");


	if (!a_shader) return;
	a_shader->enable();

	a_shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	a_shader->setUniform("u_viewprojection", cam2D.viewprojection_matrix);
	if (tex != NULL) {
		a_shader->setUniform("u_texture", tex, 0);
	}
	a_shader->setUniform("u_time", time);
	a_shader->setUniform("u_tex_tiling", 1.0f);
	a_shader->setUniform("u_model", anim);
	quad.render(GL_TRIANGLES);
	a_shader->disable();
}

void IntroStage::Render() {
	
	int window_width = Game::instance->window_width;
	int window_height = Game::instance->window_height;

	if (this->initSize != window_height) { //resize window
		this->quads.clear();
		this->tags.clear();
		this->initSize = window_height;
		initQuads();
	}

	Texture* tex = this->quadTex;
	for (int  i = 0; i < this->quads.size(); i++)
	{
		//a_shader->enable();
		Mesh quad = this->quads[i];
		renderQuad(quad, tex, Matrix44());
	}
	tex = this->menuPointer_tex;
	Matrix44 gloveModel;
	gloveModel.translate(sin(Game::instance->time*4) * 6, 0, 0);
	renderQuad(this->menuPtr_mesh, tex, gloveModel);
	for (int i = 0; i < this->tags.size(); i++)
	{
		textStruct curr = this->tags[i];
		drawText(curr.pos.x, curr.pos.y, curr.txt, Vector3(1, 1, 1), curr.scale);
	}
	
	
}


void IntroStage::Update(float seconds_elapsed) {
	Scene* scene = Game::instance->scene;

	if (!scene->music_Playing) { 
		scene->audio->PlayGameSound(0, 2); 
		scene->music_Playing == true;
	}

	//std::cout << this->menuPointer << std::endl;
	if (Input::wasKeyPressed(SDL_SCANCODE_S)) {
		
		scene->audio->PlayGameSound(1,2);
		this->menuPointer = (this->menuPointer +1) % 5;
		movePtr();
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_W)) {
		scene->audio->PlayGameSound(1, 2);
		this->menuPointer =( this->menuPointer +(this->quads.size() -1 )) % 5;
		movePtr();
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_X)) {
		scene->audio->ResetAudio();
		scene->audio->PlayGameSound(2, 2);
		if (this->menuPointer == 4) Game::instance->must_exit = true;
		SetStage((STAGE_ID)(this->menuPointer+1), Game::instance->scene->currentStage );
	}
}

//TUTORIAL STAGE

void TutorialStage::Render() {

	//TO DO

}

void TutorialStage::Update(float seconds_elapsed) {
	//TO DO
}

//PLAY STAGE

void PlayStage::Render() {

	Scene* scene = Game::instance->scene;

	scene->levels[scene->currentLevel]->Render();


}

void PlayStage::Update(float seconds_elapsed) {

	Scene* scene = Game::instance->scene;

	scene->levels[scene->currentLevel]->Update(seconds_elapsed);

	
}

//EDITOR STAGE
void EditorStage::Render() {


	Scene* scene = Game::instance->scene;

	scene->editor->Render();

}

void EditorStage::Update(float seconds_elapsed) {

	Scene* scene = Game::instance->scene;

	scene->editor->Update(seconds_elapsed);

}

//MULTI STAGE
void MultiStage::Render() {

	Scene* scene = Game::instance->scene;
	scene->multi->Render();
}

void MultiStage::Update(float seconds_elapsed) {

	Scene* scene = Game::instance->scene;
	scene->multi->Update(seconds_elapsed);
}

//END STAGE

void EndStage::Render() {
	//TO DO
}

void EndStage::Update(float seconds_elapsed) {
	//TO DO
}
