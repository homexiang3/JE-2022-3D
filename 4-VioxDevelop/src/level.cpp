#include "level.h"
#include "game.h"
#include "input.h"
//UTILS

void InitLevels(std::vector<PlayLevel*>& levels, EditorLevel*& editor, MultiLevel*& multi) {
	levels.reserve(3);
	levels.push_back(new PlayLevel("data/maps/map1.scene"));
	levels.push_back(new PlayLevel("data/maps/map1.scene"));
	levels.push_back(new PlayLevel("data/maps/map1.scene"));
	editor = new EditorLevel();
	multi = new MultiLevel();

}

//EDITOR

EditorLevel::EditorLevel() {
	//get objects
	ImportMap("data/maps/map1.scene", this->entities, this->groundMesh, this->skyMesh);
}

void EditorLevel::Render() {
	Camera* cam = Game::instance->camera;

	glDisable(GL_DEPTH_TEST);
	this->skyMesh->render(cam);//cielo
	glEnable(GL_DEPTH_TEST);

	this->groundMesh->render(cam); //suelo

	for (size_t i = 0; i < this->entities.size(); i++)
	{
		EntityMesh* entity = this->entities[i];
		entity->render(cam);
	}

}

void EditorLevel::Update(float seconds_elapsed) {

	float mouse_speed = 100.0f;
	float speed = seconds_elapsed * mouse_speed; //the speed is defined by the seconds_elapsed so it goes constant


	Scene* scene = Game::instance->scene;
	Camera* camera = Game::instance->camera;

	//change mode 
	if (Input::wasKeyPressed(SDL_SCANCODE_F1)) {
		this->currentOption = GROUND;
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_F2)) {
		this->currentOption = SKY;
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_F3)) {
		this->currentOption = OBJECT;
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_F4)) {
		this->currentOption = EDIT;
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_F5)) {
		ExportMap(this->entities, this->groundMesh, this->skyMesh);
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_F6)) {
		ImportMap("data/maps/map1.scene", this->entities, this->groundMesh, this->skyMesh);
	}

	if (this->currentOption == GROUND) {

	}
	if (this->currentOption == SKY) {

	}
	if (this->currentOption == OBJECT) {

	}
	if (this->currentOption == EDIT) {

	}

	//this->moveSelected(speed);

		if (Input::wasKeyPressed(SDL_SCANCODE_1)) this->addEntityOnFront("data/map1/building-shop-china.obj", "data/map1/color-atlas.png");  //debug to add sphere on front
		if (Input::wasKeyPressed(SDL_SCANCODE_2)) this->testCollisionOnFront(); //select with ray collision
		if (Input::wasKeyPressed(SDL_SCANCODE_3)) this->removeSelected(); //remove selected entity
		if (Input::wasKeyPressed(SDL_SCANCODE_4)) this->rotateSelected(10.0f);  //rotate selected item 
		if (Input::wasKeyPressed(SDL_SCANCODE_5)) this->rotateSelected(-10.0f);



	//movement
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
	if (Input::isKeyPressed(SDL_SCANCODE_W) ) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_S) ) camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_A) ) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_D) ) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_E)) camera->move(Vector3(0.0f, -1.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_Q)) camera->move(Vector3(0.0f, 1.0f, 0.0f) * speed);
}

void EditorLevel::addEntityOnFront(const char* mesh, const char* tex) {

	Vector3 dir = getRayDir();
	Vector3 rayOrigin = getRayOrigin();

	Vector3 spawnPos = RayPlaneCollision(Vector3(), Vector3(0, 1, 0), rayOrigin, dir);
	Matrix44 model;
	model.translate(spawnPos.x, spawnPos.y, spawnPos.z);

	EntityMesh* entity = new EntityMesh(GL_TRIANGLES, mesh, tex , "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	entity->model = model;


	this->entities.push_back(entity);

}

void EditorLevel::testCollisionOnFront() { //revisar?

	Vector3 dir = getRayDir();
	Vector3 rayOrigin = getRayOrigin();
	float min_dis = 10000.0f;
	Camera* cam = Game::instance->camera;

	for (size_t i = 0; i < this->entities.size(); i++)
	{
		EntityMesh* entity = entities[i];
		Vector3 pos;
		Vector3 normal;
		if (entity->mesh->testRayCollision(entity->model, rayOrigin, dir, pos, normal)) {

			Vector3 entityPos = entity->model.getTranslation();
			float dis = cam->eye.distance(entityPos);
			if (dis < min_dis) {
				selectedEntity = entity;
				selectedEntityPos = i;
			}

		}

	}
}

void EditorLevel::rotateSelected(float angleDegree) {
	if (this->selectedEntity == NULL) {
		return;
	}

	this->selectedEntity->model.rotate(angleDegree * DEG2RAD, Vector3(0, 1, 0));
}

void EditorLevel::removeSelected() {

	if (this->selectedEntity == NULL) {
		return;
	}

	this->entities.erase(this->entities.begin() + this->selectedEntityPos);
	this->selectedEntity = NULL;
}

void EditorLevel::removeEntities() {

	while (!this->entities.empty())
	{
		this->entities.pop_back();
	}
}

Vector3 EditorLevel::getRayDir() {

	Vector2 mouse = Input::mouse_position;
	Game* g = Game::instance;
	Camera* cam = g->camera;
	Vector3 dir = cam->getRayDirection(mouse.x, mouse.y, g->window_width, g->window_height);

	return dir;
}

Vector3 EditorLevel::getRayOrigin() {

	Game* g = Game::instance;
	Camera* cam = g->camera;
	Vector3 rayOrigin = cam->eye;

	return rayOrigin;
}

void EditorLevel::moveSelected(float speed) { //falla rotacion
	if (this->selectedEntity == NULL) {
		return;
	}

	Matrix44 entityRotation;
	

	Vector3 forward = entityRotation.rotateVector(Vector3(0, 0, -1));
	Vector3 right = entityRotation.rotateVector(Vector3(1, 0, 0));
	

	Matrix44 model = this->selectedEntity->model;

	Vector3 pos = model.getTranslation();


	Vector3 entityVel;

	if (Input::isKeyPressed(SDL_SCANCODE_UP)) entityVel = entityVel + (forward * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_DOWN)) entityVel = entityVel - (forward * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_RIGHT)) entityVel = entityVel + (right * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_LEFT)) entityVel = entityVel - (right * speed);

	Vector3 nextPos = pos + entityVel;


	this->selectedEntity->model.setTranslation(nextPos.x, nextPos.y, nextPos.z);


}

//MULTIPLAYER 
MultiLevel::MultiLevel() {


	//players
	this->player1 = new sPlayer();
	this->player1->character_mesh = new EntityMesh(GL_TRIANGLES, "data/skelleton.obj", "data/minichar_tex.png", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));

	this->player2 = new sPlayer();
	this->player2->character_mesh = new EntityMesh(GL_TRIANGLES, "data/skelleton.obj", "data/minichar_tex.png", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));

	//cams
	int window_width = Game::instance->window_width;
	int window_height = Game::instance->window_height;

	this->cam1 = new Camera();
	this->cam1->lookAt(Vector3(0.f, 100.f, 100.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0
	this->cam1->setPerspective(70.f, window_width / (float)window_height, 0.1f, 10000.f); //set the projection, we want to be perspective

	this->cam2 = new Camera();
	this->cam2->lookAt(Vector3(0.f, 100.f, 100.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0
	this->cam2->setPerspective(70.f, window_width / (float)window_height, 0.1f, 10000.f); //set the projection, we want to be perspective

	//get objects
	ImportMap("data/maps/map1.scene", this->entities, this->groundMesh, this->skyMesh);
}

void MultiLevel::Render() {


	Scene* scene = Game::instance->scene;
	int window_width = Game::instance->window_width;
	int window_height = Game::instance->window_height;

	//CAM 1
	glViewport(0, 0, window_width * 0.5f, window_height);
	cam1->aspect = window_width * 0.5f / window_height;

	Matrix44 player1Model = this->player1->getModel();

	if (scene->cameraLocked) {
		//cam1
		Vector3 desiredEye1 = player1Model * Vector3(0, 3, 3);
		Vector3 eye1 = Lerp(this->cam1->eye, desiredEye1, 5.f * Game::instance->elapsed_time);
		Vector3 center1 = player1Model * Vector3(0.0f, 0.0f, -5.0f);
		Vector3 up1 = Vector3(0.0f, 1.0f, 0.0f);
		//set the cameras 
		cam1->lookAt(eye1, center1, up1);
	}

	cam1->enable();
	RenderWorld(cam1);
	//CAM 2

	glViewport(window_width * 0.5f, 0, window_width * 0.5f , window_height);
	cam2->aspect = window_width * 0.5f / window_height;

	Matrix44 player2Model = this->player2->getModel();

	if (scene->cameraLocked) {
		//cam2
		Vector3 desiredEye2 = player2Model * Vector3(0, 3, 3);
		Vector3 eye2 = Lerp(this->cam2->eye, desiredEye2, 5.f * Game::instance->elapsed_time);
		Vector3 center2 = player2Model * Vector3(0.0f, 0.0f, -5.0f);
		Vector3 up2 = Vector3(0.0f, 1.0f, 0.0f);
		//set the cameras 
		cam2->lookAt(eye2, center2, up2);
	}
	
	cam2->enable();
	RenderWorld(cam2);

	RenderMinimap();


}


void MultiLevel::Update(float seconds_elapsed) {

	Scene* scene = Game::instance->scene;

	bool isRunning = false;
	float walk_speed = 10.0f * seconds_elapsed;
	float run_speed = 30.0f * seconds_elapsed;

	this->player1->jumpLock = max(0.0f, this->player1->jumpLock - seconds_elapsed);

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
			this->player1->pitch += -Input::mouse_delta.y * 10.0f * seconds_elapsed;
			this->player1->yaw += -Input::mouse_delta.x * 10.0f * seconds_elapsed;
			Input::centerMouse();
			SDL_ShowCursor(false);
		}
		else {
			if (Input::isKeyPressed(SDL_SCANCODE_E)) this->player1->yaw += rotSpeed;
			if (Input::isKeyPressed(SDL_SCANCODE_Q)) this->player1->yaw -= rotSpeed;
		}

		this->player1->playerMovement(this->entities, seconds_elapsed, rotSpeed, playerSpeed);

	}
}

void MultiLevel::RenderWorld(Camera* cam) {
	//this->skyMesh->model.setTranslation(cam->eye.x, cam->eye.y - 20.0f, cam->eye.z);
	glDisable(GL_DEPTH_TEST);
	this->skyMesh->render(cam);//cielo
	glEnable(GL_DEPTH_TEST);

	this->groundMesh->render(cam); //suelo

	this->player1->character_mesh->render(cam);//player

	this->player2->character_mesh->render(cam);//player

	for (size_t i = 0; i < this->entities.size(); i++)//entities added
	{
		EntityMesh* entity = this->entities[i];
		entity->render(cam);
	}
}

void MultiLevel::RenderMinimap() {
	int window_width = Game::instance->window_width;
	int window_height = Game::instance->window_height;
	//player1 side 
	glViewport(window_width*0.5f - window_width * 0.15f, window_height - window_height * 0.15f, window_width * 0.15f, window_height * 0.15f);
	glClear(GL_DEPTH_BUFFER_BIT);

	Camera cam;
	cam.setPerspective(60, 1, 0.1f, 1000.f);
	Vector3 eye = this->player1->pos + Vector3(0, 100, 0);
	Vector3 center = this->player1->pos;
	Matrix44 player1Model = player1->getModel();
	Vector3 up = player1Model.rotateVector(Vector3(0, 0, -1));
	cam.lookAt(eye, center, up);

	//minimap
	this->groundMesh->render(&cam);

	//we as a green point
	EntityMesh* p1 = new EntityMesh(GL_TRIANGLES, "data/sphere.obj", "", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(0, 1, 0, 1));
	p1->model = player1Model;
	p1->model.scale(3, 3, 3);
	p1->render(&cam);

	EntityMesh* p2 = new EntityMesh(GL_TRIANGLES, "data/sphere.obj", "", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 0, 0, 1));
	Matrix44 player2Model = player2->getModel();
	p2->model = player2Model;
	p2->model.scale(3, 3, 3);
	p2->render(&cam);


	glViewport(0, 0, window_width, window_height);

	/*player2 side
	glViewport(window_width - window_width * 0.1f, window_height - window_height * 0.1f, window_width * 0.1f, window_height * 0.1f);
	cam->setPerspective(60, 1, 0.1f, 1000.f);*/


}

 //PLAYABLE LEVEL

PlayLevel::PlayLevel(const char* map) {

	//player
	this->player = new sPlayer();
	this->player->character_mesh = new EntityMesh(GL_TRIANGLES, "data/skelleton.obj", "data/minichar_tex.png", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	//get objects
	ImportMap(map, this->entities, this->groundMesh, this->skyMesh);
}

void PlayLevel::Render() {

	Camera* camera = Camera::current;
	Scene* scene = Game::instance->scene;


	Matrix44 playerModel = this->player->getModel();

	if (scene->cameraLocked) {
		//camera following plane
		Vector3 desiredEye = playerModel * Vector3(0, 3, 3);
		Vector3 eye = Lerp(camera->eye, desiredEye, 5.f * Game::instance->elapsed_time);
		Vector3 center = playerModel * Vector3(0.0f, 0.0f, -5.0f);
		Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
		if (scene->firstPerson) {
			//si es first person no renderizar el personaje o solo manos/piernas
			Matrix44 firstPersonModel = playerModel;
			firstPersonModel.rotate(player->pitch * DEG2RAD, Vector3(1, 0, 0));

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
	this->skyMesh->render(camera);//cielo
	glEnable(GL_DEPTH_TEST);

	this->groundMesh->render(camera); //suelo

	this->player->character_mesh->render(camera);//player

	for (size_t i = 0; i < this->entities.size(); i++)//entities added
	{
		EntityMesh* entity = this->entities[i];
		entity->render(camera);
	}

}

void PlayLevel::Update(float seconds_elapsed) {

	Scene* scene = Game::instance->scene;

	bool isRunning = false;
	float walk_speed = 10.0f * seconds_elapsed;
	float run_speed = 30.0f * seconds_elapsed;

	this->player->jumpLock = max(0.0f, this->player->jumpLock - seconds_elapsed);

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
			this->player->pitch += -Input::mouse_delta.y * 10.0f * seconds_elapsed;
			this->player->yaw += -Input::mouse_delta.x * 10.0f * seconds_elapsed;
			Input::centerMouse();
			SDL_ShowCursor(false);
		}
		else {
			if (Input::isKeyPressed(SDL_SCANCODE_E)) this->player->yaw += rotSpeed;
			if (Input::isKeyPressed(SDL_SCANCODE_Q)) this->player->yaw -= rotSpeed;
		}

		this->player->playerMovement(this->entities, seconds_elapsed, rotSpeed, playerSpeed);

	}
}