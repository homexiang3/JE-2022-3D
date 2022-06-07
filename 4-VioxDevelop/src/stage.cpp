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
	
	//TO DO
}


void IntroStage::Update(float seconds_elapsed) {
	//TO DO
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

	Camera* camera = Camera::current;
	Scene* scene = Game::instance->scene;


	Matrix44 playerModel = scene->player.getModel();
	scene->player.character_mesh->model = playerModel;

	if (scene->cameraLocked) {
		//camera following plane
		Vector3 desiredEye = playerModel * Vector3(0, 3, 3);
		Vector3 eye = scene->Lerp(camera->eye, desiredEye, 5.f * Game::instance->elapsed_time);
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

	scene->player.character_mesh->render();//player

	for (size_t i = 0; i < scene->entities.size(); i++)//entities added
	{
		EntityMesh* entity = scene->entities[i];
		entity->render();
	}
}

void PlayStage::Update(float seconds_elapsed) {
	Scene* scene = Game::instance->scene;

		//movement code

		bool isRunning = false;
		float walk_speed = 10.0f * seconds_elapsed;
		float run_speed = 30.0f * seconds_elapsed;

		scene->player.jumpLock = max(0.0f, scene->player.jumpLock - seconds_elapsed);

		//sprint
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) isRunning = true;
		float playerSpeed = (isRunning == true) ? run_speed : walk_speed;


		//escalar tiempo para que vayan mas lentas las particulas
		if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) {
			seconds_elapsed *= 0.2f;
		}


		if (scene->cameraLocked) {

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

			scene->player.playerMovement(scene->entities, seconds_elapsed, rotSpeed,  playerSpeed);

		}
	
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
	//TO DO

}

void MultiStage::Update(float seconds_elapsed) {
	//TO DO
}

//END STAGE

void EndStage::Render() {
	//TO DO
}

void EndStage::Update(float seconds_elapsed) {
	//TO DO
}
