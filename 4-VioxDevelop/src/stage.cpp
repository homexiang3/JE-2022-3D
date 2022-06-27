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

void IntroStage::Render() {
	
	Scene* scene = Game::instance->scene;

	scene->intro->Render();
}


void IntroStage::Update(float seconds_elapsed) {
	Scene* scene = Game::instance->scene;

	scene->intro->Update(seconds_elapsed);
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
	Scene* scene = Game::instance->scene;

	scene->end->Render();
}

void EndStage::Update(float seconds_elapsed) {
	Scene* scene = Game::instance->scene;

	scene->end->Update(seconds_elapsed);
}
