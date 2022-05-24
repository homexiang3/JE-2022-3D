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
	
	
}


void IntroStage::Update(float seconds_elapsed) {
	
}

//TUTORIAL STAGE

void TutorialStage::Render() {

	

}

void TutorialStage::Update(float seconds_elapsed) {
	
}

//PLAY STAGE

void PlayStage::Render() {

	Camera* camera = Camera::current;
	Scene* scene = Game::instance->scene;

	Matrix44 playerModel = scene->player.getModel();
	scene->player.entity->model = playerModel;

	if (scene->cameraLocked) {
		//camera following plane
		Vector3 eye = playerModel * Vector3(0.0f, 3.0f, 3.0f);
		Vector3 center = playerModel * Vector3(0.0f, 0.0f, -5.0f);
		Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
		if (scene->firstPerson) {
			//si es first person no renderizar el personaje o solo manos/piernas
			Matrix44 firstPersonModel = playerModel;
			firstPersonModel.rotate(scene->player.pitch * DEG2RAD, Vector3(1, 0, 0));

			eye = playerModel * Vector3(0.0f, 1.0f, -0.5f);
			center = eye + firstPersonModel.rotateVector(Vector3(0.0f, 0.0f, -1.0f));
			up = firstPersonModel.rotateVector(Vector3(0.0f, 1.0f, 0.0f));
		}
		//set the camera 
		camera->lookAt(eye, center, up);
	}

	//Render
	//scene->skyMesh->model.setTranslation(camera->eye.x, camera->eye.y - 20.0f, camera->eye.z); solo usarlo si el mapa fuera muy grande
	glDisable(GL_DEPTH_TEST);
	scene->skyMesh->render();//cielo
	glEnable(GL_DEPTH_TEST);

	scene->groundMesh->render(); //suelo

	scene->player.entity->render();//player

	for (size_t i = 0; i < scene->entities.size(); i++)//entities added
	{
		EntityMesh* entity = scene->entities[i];
		entity->render();
	}
}

void PlayStage::Update(float seconds_elapsed) {
	Scene* scene = Game::instance->scene;

	if (scene->cameraLocked) {
		float playerSpeed = 20.0f * seconds_elapsed;
		float rotSpeed = 120.0f * seconds_elapsed;

		if (scene->firstPerson) {
			scene->player.pitch += -Input::mouse_delta.y * 10.0f * seconds_elapsed;
			scene->player.yaw += -Input::mouse_delta.x * 10.0f * seconds_elapsed;
			Input::centerMouse();
			SDL_ShowCursor(false);
		}
		else {
			if (Input::isKeyPressed(SDL_SCANCODE_E)) scene->player.yaw += rotSpeed;
			if (Input::isKeyPressed(SDL_SCANCODE_Q)) scene->player.yaw -= rotSpeed;
		}

		Matrix44 playerRotation;
		playerRotation.rotate(scene->player.yaw * DEG2RAD, Vector3(0, 1, 0));

		Vector3 forward = playerRotation.rotateVector(Vector3(0, 0, -1));
		Vector3 right = playerRotation.rotateVector(Vector3(1, 0, 0));

		Vector3 playerVel;

		if (Input::isKeyPressed(SDL_SCANCODE_W)) playerVel = playerVel + (forward * playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_S)) playerVel = playerVel - (forward * playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_D)) playerVel = playerVel + (right * playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_A)) playerVel = playerVel - (right * playerSpeed);

		Vector3 nextPos = scene->player.pos + playerVel;
		
		nextPos = scene->player.playerCollision(scene->entities, nextPos, seconds_elapsed);

		scene->player.pos = nextPos;
	}
	
}

//EDITOR STAGE
void EditorStage::Render() {

	Scene* scene = Game::instance->scene;

	for (size_t i = 0; i < scene->entities.size(); i++)//entities added
	{
		EntityMesh* entity = scene->entities[i];
		entity->render();
	}
}

void EditorStage::Update(float seconds_elapsed) {
	
	float mouse_speed = 100.0f;
	float speed = seconds_elapsed * mouse_speed; //the speed is defined by the seconds_elapsed so it goes constant

	Scene* scene = Game::instance->scene;
	Camera* camera = Game::instance->camera;

	if (Input::wasKeyPressed(SDL_SCANCODE_TAB)) {
		scene->cameraLocked = !scene->cameraLocked;
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_1)) scene->addEntityOnFront();  //debug to add sphere on front
	if (Input::wasKeyPressed(SDL_SCANCODE_2)) scene->testCollisionOnFront(); //debug to see collision point on front (col in console)
	if (Input::wasKeyPressed(SDL_SCANCODE_3)) scene->rotateSelected(10.0f);  //rotate selected item with 3
	if (Input::wasKeyPressed(SDL_SCANCODE_4)) scene->rotateSelected(-10.0f);

	//movement
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
	if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_E)) camera->move(Vector3(0.0f, -1.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_Q)) camera->move(Vector3(0.0f, 1.0f, 0.0f) * speed);


}

//MULTI STAGE
void MultiStage::Render() {


}

void MultiStage::Update(float seconds_elapsed) {

}

//END STAGE

void EndStage::Render() {
	
}

void EndStage::Update(float seconds_elapsed) {
	
}
