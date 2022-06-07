#include "level.h"
#include "game.h"
#include "input.h"

void InitLevels(std::vector<PlayLevel*>& levels, EditorLevel*& editor, MultiLevel*& multi) {
	levels.reserve(3);
	levels.push_back(new PlayLevel());
	levels.push_back(new PlayLevel());
	levels.push_back(new PlayLevel());
	editor = new EditorLevel();
	multi = new MultiLevel();

}

void EditorLevel::Render() {

	glDisable(GL_DEPTH_TEST);
	this->skyMesh->render();//cielo
	glEnable(GL_DEPTH_TEST);

	this->groundMesh->render(); //suelo

	for (size_t i = 0; i < this->entities.size(); i++)
	{
		EntityMesh* entity = this->entities[i];
		entity->render();
	}

}

void EditorLevel::Update(float seconds_elapsed) {

	float mouse_speed = 100.0f;
	float speed = seconds_elapsed * mouse_speed; //the speed is defined by the seconds_elapsed so it goes constant

	Scene* scene = Game::instance->scene;
	Camera* camera = Game::instance->camera;


	if (Input::wasKeyPressed(SDL_SCANCODE_F2)) {
		ExportMap(this->entities, this->groundMesh, this->skyMesh);
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_F3)) {
		ImportMap("data/maps/map1.scene", this->entities, this->groundMesh, this->skyMesh);
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_1)) this->addEntityOnFront();  //debug to add sphere on front
	if (Input::wasKeyPressed(SDL_SCANCODE_2)) this->testCollisionOnFront(); //select with ray collision
	if (Input::wasKeyPressed(SDL_SCANCODE_3)) this->removeSelected(); //remove selected entity
	if (Input::wasKeyPressed(SDL_SCANCODE_4)) this->rotateSelected(10.0f);  //rotate selected item 
	if (Input::wasKeyPressed(SDL_SCANCODE_5)) this->rotateSelected(-10.0f);

	//movement
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
	if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_E)) camera->move(Vector3(0.0f, -1.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_Q)) camera->move(Vector3(0.0f, 1.0f, 0.0f) * speed);
}

void EditorLevel::addEntityOnFront() {

	Vector3 dir = getRayDir();
	Vector3 rayOrigin = getRayOrigin();

	Vector3 spawnPos = RayPlaneCollision(Vector3(), Vector3(0, 1, 0), rayOrigin, dir);
	Matrix44 model;
	model.translate(spawnPos.x, spawnPos.y, spawnPos.z);

	EntityMesh* entity = new EntityMesh(GL_TRIANGLES, "data/map1/building-shop-china.obj", "data/map1/color-atlas.png", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
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

void MultiLevel::Render() {

	//TO DO

}



void MultiLevel::Update(float seconds_elapsed) {
	//TO DO
}

void PlayLevel::Render() {

	//TO DO

}

void PlayLevel::Update(float seconds_elapsed) {
	//TO DO
}