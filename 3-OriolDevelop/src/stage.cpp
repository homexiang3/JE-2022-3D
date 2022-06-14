#include "stage.h"
#include "game.h"
#include "input.h"
#include <math.h>  //fmod for anims
#include "texture.h"
#include "mesh.h"

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
	//rescale model for animation (it was a giant ) 
	playerModel.scale(scene->player.side * 0.015, 0.015, 0.015);
	//playerModel.rotate(180 * DEG2RAD, Vector3(0,1,0));
	scene->player.character_mesh->model = playerModel;

	//animations 
	float time = getTime() * 0.001;
	
	float t = fmod(time, scene->player.idle->duration) / scene->player.run->duration;
	/*
	scene->player.walk->assignTime(t * scene->player.walk->duration);
	scene->player.run->assignTime(t * scene->player.run->duration);
	scene->player.idle->assignTime(time * scene->player.idle->duration);
	*/
	for (int i = 0; i < scene->player.anims.size(); i++) {
		if (i==3 || i==6 ) scene->player.anims[i]->assignTime(time * 1.8);
		else scene->player.anims[i]->assignTime(time);
	}
	
	Animation* FinalAnim = scene->player.anims[scene->player.ctr];
	//blendSkeleton(&scene->player.anims[0]->skeleton, &scene->player.anims[scene->player.ctr]->skeleton, 0.3f, &FinalAnim->skeleton);
	
	//float velFactor = scene->player.playerVel

	//Render
	//scene->skyMesh->model.setTranslation(camera->eye.x, camera->eye.y - 20.0f, camera->eye.z); solo usarlo si el mapa fuera muy grande
	glDisable(GL_DEPTH_TEST);
	scene->skyMesh->render(FinalAnim->skeleton, false);//cielo
	glEnable(GL_DEPTH_TEST);

	scene->groundMesh->render(FinalAnim->skeleton, false); //suelo

	scene->player.character_mesh->render(FinalAnim->skeleton, true);//player

	for (size_t i = 0; i < scene->entities.size(); i++)//entities added
	{
		EntityMesh* entity = scene->entities[i];
		entity->render(FinalAnim->skeleton, false);
	}
}

void PlayStage::Update(float seconds_elapsed) {
	Scene* scene = Game::instance->scene;

		//movement code

		float playerX = scene->player.pos[0];
		float playerY = scene->player.pos[1];
		float playerZ = scene->player.pos[2];

		bool isRunning = false;
		float walk_speed = 10.0f * seconds_elapsed;
		float run_speed = 30.0f * seconds_elapsed;

		scene->player.jumpLock = max(0.0f, scene->player.jumpLock - seconds_elapsed);
		scene->player.animTimer = max(0.0f, scene->player.animTimer - seconds_elapsed);


		//punch 
		if (Input::wasKeyPressed(SDL_SCANCODE_Z) && scene->player.animTimer <= 0.2f) {
			scene->player.side = scene->player.side * -1;
			ChangeAnim(3 , scene->player.anims[3]->duration / 1.8 - 0.05);
			scene->PlayGameSound(0);
		}

		if (Input::wasKeyPressed(SDL_SCANCODE_V) && scene->player.animTimer <= 0.2f) {
			scene->player.side = scene->player.side * -1;
			ChangeAnim(4 , scene->player.anims[4]->duration);
		}
		
		if (scene->player.animTimer <= 0.0f) ChangeAnim(0, NULL);
		//sprint
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) isRunning = true;
			
		
		scene->player.playerVel = (isRunning == true) ? run_speed : walk_speed;


		//escalar tiempo para que vayan mas lentas las particulas
		if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) {
			seconds_elapsed *= 0.2f;
		}


		if (scene->cameraLocked) {
			//float playerSpeed = 20.0f * elapsed_time;
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
			Vector3 up = playerRotation.rotateVector(Vector3(0, 1, 0));

			Vector3 playerVel;

			if (Input::isKeyPressed(SDL_SCANCODE_W) && scene->player.ctr != 5)
			{
				playerVel = playerVel + (forward * scene->player.playerVel);
				ChangeAnim(1, NULL);
			}
			if (Input::isKeyPressed(SDL_SCANCODE_S) && scene->player.ctr != 5) {
				playerVel = playerVel - (forward * scene->player.playerVel);
				ChangeAnim(1, NULL);
			}
			if (Input::isKeyPressed(SDL_SCANCODE_D)) playerVel = playerVel + (right * scene->player.playerVel);
			if (Input::isKeyPressed(SDL_SCANCODE_A)) playerVel = playerVel - (right * scene->player.playerVel);

			if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) ChangeAnim(2, NULL);// run anim

			//player movement code
			//jump
			if (Input::wasKeyPressed(SDL_SCANCODE_C) && playerY <= 0.0f) {
				scene->player.jumpLock = 0.3f;
				ChangeAnim(6, 0.8f);
			}

			if (scene->player.jumpLock != 0.0f) {
				playerVel[1] += 0.15f;
			}

			if (playerY > 0.0f) {
				playerVel[1] -= seconds_elapsed * 3;
			}

			Vector3 nextPos = scene->player.pos + playerVel;

			//dash
			Vector2 dash_dir = Vector2(playerX - nextPos[0], playerZ - nextPos[2]).normalize();
			if (dash_dir.x != 0 || dash_dir.y != 0)scene->player.dash_direction = dash_dir;

			//std::cout << scene->player.dash_direction.x << "  " << scene->player.dash_direction.y << std::endl;

			Vector3 playerVec = scene->player.character_mesh->model.frontVector().normalize();
			float sumX = 20.0 * scene->player.dash_direction.x;
			float sumZ = 20.0 * scene->player.dash_direction.y;
			if (scene->player.ctr == 5) {
				playerVel[0] -= sumX;
				playerVel[2] -= sumZ;
			}
			if (Input::wasKeyPressed(SDL_SCANCODE_X)) {
				playerVel[0] -= sumX;
				playerVel[2] -= sumZ;
				ChangeAnim(5, 0.25f);
			}

			nextPos = scene->player.pos + playerVel;

		nextPos = scene->player.playerCollision(scene->entities, nextPos, seconds_elapsed);

		scene->player.pos = nextPos;
		}
	
}

void ChangeAnim(int i , float time) {
	Scene* scene = Game::instance->scene;
	if(scene->player.animTimer <=0.0f) scene->player.ctr = i;
	if(time != NULL) scene->player.animTimer =time;
}

//EDITOR STAGE
void EditorStage::Render() {

	Scene* scene = Game::instance->scene;

	glDisable(GL_DEPTH_TEST);
	//scene->skyMesh->render();//cielo
	glEnable(GL_DEPTH_TEST);

	//scene->groundMesh->render(); //suelo

	for (size_t i = 0; i < scene->entities.size(); i++)//entities added
	{
		EntityMesh* entity = scene->entities[i];
		///entity->render();
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
	
	if (Input::wasKeyPressed(SDL_SCANCODE_F2)) {
		scene->ExportMap(scene->entities);
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_F3)) {
		scene->ImportMap("data/maps/map1.scene",scene->entities);
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
