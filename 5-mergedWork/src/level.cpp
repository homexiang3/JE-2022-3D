#include "level.h"
#include "game.h"
#include "input.h"
//UTILS

void InitLevels(std::vector<PlayLevel*>& levels, EditorLevel*& editor, MultiLevel*& multi) {
	//we defined just 3 levels
	levels.reserve(3);
	levels.push_back(new PlayLevel("data/maps/map1.scene", "data/map1/enemies.txt"));
	levels.push_back(new PlayLevel("data/maps/map2.scene", "data/map2/enemies.txt"));
	levels.push_back(new PlayLevel("data/maps/map3.scene", "data/map3/enemies.txt"));
	editor = new EditorLevel();
	multi = new MultiLevel();

}

void RenderMinimap(int widthStart, sPlayer*& player, std::vector<sPlayer*>& enemies, EntityMesh* groundMesh, std::vector<EntityMesh*>& entities)
{
	int window_width = Game::instance->window_width;
	int window_height = Game::instance->window_height;

	//player 
	glViewport(widthStart - window_width * 0.15f, window_height - window_height * 0.15f, window_width * 0.15f, window_height * 0.15f);
	glClear(GL_DEPTH_BUFFER_BIT);

	Camera cam;
	cam.setPerspective(60, 1, 0.1f, 1000.f);
	Vector3 eye = player->pos + Vector3(0, 100, 0);
	Vector3 center = player->pos;
	Matrix44 playerModel = player->getModel();
	Vector3 up = playerModel.rotateVector(Vector3(0, 0, -1));
	cam.lookAt(eye, center, up);

	//minimap
	groundMesh->render(&cam);

	//Use flat.fs since no texture is provided

	//we as a green point
	EntityMesh* playerPoint = new EntityMesh(GL_TRIANGLES, "data/sphere.obj", "", "data/shaders/basic.vs", "data/shaders/flat.fs", Vector4(0, 1, 0, 1));
	playerPoint->model = playerModel;
	playerPoint->model.scale(3, 3, 3);
	playerPoint->render(&cam);

	//enemies as a red point
	for (size_t i = 0; i < enemies.size(); i++)
	{
		EntityMesh* enemyPoint = new EntityMesh(GL_TRIANGLES, "data/sphere.obj", "", "data/shaders/basic.vs", "data/shaders/flat.fs", Vector4(1, 0, 0, 1));
		Matrix44 enemyModel = enemies[i]->getModel();
		enemyPoint->model = enemyModel;
		enemyPoint->model.scale(3, 3, 3);
		enemyPoint->render(&cam);

	}

	//objects of the map
	for (size_t i = 0; i < entities.size(); i++)
	{
		//optional
		EntityMesh* entityPoint = new EntityMesh(GL_TRIANGLES, "data/sphere.obj", "", "data/shaders/basic.vs", "data/shaders/flat.fs", Vector4(1, 0.5, 0.2, 1));
		Matrix44 entityModel = entities[i]->model;
		entityPoint->model = entityModel;
		entityPoint->model.scale(6, 6, 6);
		entityPoint->render(&cam);

	}


	//restore viewport
	glViewport(0, 0, window_width, window_height);
}


void SetupCam(Matrix44& playerModel, Camera* cam)
{
	//if (scene->cameraLocked) {
	//camera following plane
	Vector3 desiredEye = playerModel * Vector3(0, 4, 6);
	Vector3 eye = Lerp(cam->eye, desiredEye, 5.f * Game::instance->elapsed_time);
	Vector3 center = playerModel * Vector3(0.0f, 0.0f, -5.0f);
	Vector3 up = Vector3(0.0f, 1.0f, 0.0f);

	//set the camera 
	cam->lookAt(eye, center, up);
	//}
}

void drawHP(Mesh quad, Texture* tex, Matrix44 anim, Camera cam2D)
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

void updateHealthBar(float centerStart, Mesh& playerHP_quad, sPlayer* player) {
	int window_width = Game::instance->window_width;
	int window_height = Game::instance->window_height;

	const int max_health = player->max_health;

	int health = player->health;
	float center_x = centerStart - 1.5 * (max_health - health);
	float center_y = 90;
	float w = 30 - 3 * (max_health - health);
	float h = 5;

	playerHP_quad.vertices.clear();

	//create six vertices (3 for upperleft triangle and 3 for lowerright)

	playerHP_quad.vertices.push_back(Vector3(center_x + w * 0.5f, center_y + h * 0.5f, 0.0f));
	playerHP_quad.vertices.push_back(Vector3(center_x - w * 0.5f, center_y - h * 0.5f, 0.0f));
	playerHP_quad.vertices.push_back(Vector3(center_x + w * 0.5f, center_y - h * 0.5f, 0.0f));
	playerHP_quad.vertices.push_back(Vector3(center_x - w * 0.5f, center_y + h * 0.5f, 0.0f));
	playerHP_quad.vertices.push_back(Vector3(center_x - w * 0.5f, center_y - h * 0.5f, 0.0f));
	playerHP_quad.vertices.push_back(Vector3(center_x + w * 0.5f, center_y + h * 0.5f, 0.0f));

}


//EDITOR

EditorLevel::EditorLevel() {
	
}

void EditorLevel::Render() {
	Camera* cam = Game::instance->camera;

	if (this->skyMesh != NULL) {
		glDisable(GL_DEPTH_TEST);
		this->skyMesh->render(cam);//cielo
		glEnable(GL_DEPTH_TEST);
	}

	if (this->groundMesh != NULL ) this->groundMesh->render(cam); //suelo

	for (size_t i = 0; i < this->entities.size(); i++)
	{
		EntityMesh* entity = this->entities[i];
		entity->render(cam);
	}
	//Draw the floor grid
	drawGrid();

}

void EditorLevel::Update(float seconds_elapsed) {

	float mouse_speed = 100.0f;
	float speed = seconds_elapsed * mouse_speed; //the speed is defined by the seconds_elapsed so it goes constant


	Scene* scene = Game::instance->scene;
	Camera* camera = Game::instance->camera;

	//change mode 
	if (Input::wasKeyPressed(SDL_SCANCODE_F1)) {
		std::cout << " + Editor: Ground mode active" << std::endl;
		this->currentOption = GROUND;
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_F2)) {
		std::cout << " + Editor: Sky mode active" << std::endl;
		this->currentOption = SKY;
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_F3)) {
		std::cout << " + Editor: Object mode active" << std::endl;
		this->currentOption = OBJECT;
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_F4)) {
		std::cout << " + Editor: Edit mode active" << std::endl;
		this->currentOption = EDIT;
	}
	//exportar mapa (escribir nombre por consola)
	if (Input::wasKeyPressed(SDL_SCANCODE_F5)) {
		ExportMap(this->entities, this->groundMesh, this->skyMesh);
	}
	//cargar los mapas ya existentes para cambiar cosas
	if (Input::wasKeyPressed(SDL_SCANCODE_F6)) {
		clearEditor();
		ImportMap("data/maps/map1.scene", this->entities, this->groundMesh, this->skyMesh);
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_F7)) {
		clearEditor();
		ImportMap("data/maps/map2.scene", this->entities, this->groundMesh, this->skyMesh);
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_F8)) {
		clearEditor();
		ImportMap("data/maps/map3.scene", this->entities, this->groundMesh, this->skyMesh);
	}
	//opciones del editor 
	//cargar diferentes suelos
	if (this->currentOption == GROUND) {
		if (Input::wasKeyPressed(SDL_SCANCODE_1)) {
			EntityMesh* ground = new EntityMesh(GL_TRIANGLES, "", "data/map1/suelo.tga", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
			ground->tiling = 200.0f;
			ground->mesh = new Mesh();
			ground->mesh->createPlane(1000); 
			this->groundMesh = ground;
		}
		if (Input::wasKeyPressed(SDL_SCANCODE_2)) {
			EntityMesh* ground = new EntityMesh(GL_TRIANGLES, "", "data/map2/suelo2.png", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
			ground->tiling = 200.0f;
			ground->mesh = new Mesh();
			ground->mesh->createPlane(1000); 
			this->groundMesh = ground;
		}
		if (Input::wasKeyPressed(SDL_SCANCODE_3)) {
			EntityMesh* ground = new EntityMesh(GL_TRIANGLES, "", "data/map3/suelo3.png", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
			ground->tiling = 200.0f;
			ground->mesh = new Mesh();
			ground->mesh->createPlane(1000); 
			this->groundMesh = ground;

		}
	}
	//cargar diferentes cielos
	if (this->currentOption == SKY) {
		if (Input::wasKeyPressed(SDL_SCANCODE_1)) this->skyMesh = new EntityMesh(GL_TRIANGLES, "data/cielo.ASE", "data/map1/cielo.tga", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
		if (Input::wasKeyPressed(SDL_SCANCODE_2)) this->skyMesh = new EntityMesh(GL_TRIANGLES, "data/cielo.ASE", "data/map2/cielo2.tga", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
		if (Input::wasKeyPressed(SDL_SCANCODE_3)) this->skyMesh = new EntityMesh(GL_TRIANGLES, "data/cielo.ASE", "data/map3/cielo3.tga", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	}
	//cargar diferentes entities
	if (this->currentOption == OBJECT) {
		if (Input::wasKeyPressed(SDL_SCANCODE_1)) this->addEntityOnFront("data/map1/building-shop-china.obj", "data/map1/color-atlas.png");
		if (Input::wasKeyPressed(SDL_SCANCODE_2)) this->addEntityOnFront("data/map1/building-shop-china.obj", "data/map1/color-atlas.png");
		if (Input::wasKeyPressed(SDL_SCANCODE_3)) this->addEntityOnFront("data/map1/building-shop-china.obj", "data/map1/color-atlas.png");
		if (Input::wasKeyPressed(SDL_SCANCODE_4)) this->addEntityOnFront("data/map1/building-shop-china.obj", "data/map1/color-atlas.png");
		if (Input::wasKeyPressed(SDL_SCANCODE_5)) this->addEntityOnFront("data/map1/building-shop-china.obj", "data/map1/color-atlas.png");
		if (Input::wasKeyPressed(SDL_SCANCODE_6)) this->addEntityOnFront("data/map1/building-shop-china.obj", "data/map1/color-atlas.png");
		if (Input::wasKeyPressed(SDL_SCANCODE_7)) this->addEntityOnFront("data/map1/building-shop-china.obj", "data/map1/color-atlas.png");
		if (Input::wasKeyPressed(SDL_SCANCODE_8)) this->addEntityOnFront("data/map1/building-shop-china.obj", "data/map1/color-atlas.png");
		if (Input::wasKeyPressed(SDL_SCANCODE_9)) this->addEntityOnFront("data/map1/building-shop-china.obj", "data/map1/color-atlas.png");
	}
	//editar los objetos
	if (this->currentOption == EDIT) {
		if (Input::wasKeyPressed(SDL_SCANCODE_1)) this->testCollisionOnFront(); //select with ray collision
		if (Input::wasKeyPressed(SDL_SCANCODE_2)) this->removeSelected();//remove selected entity
		if (Input::wasKeyPressed(SDL_SCANCODE_3)) this->rotateSelected(10.0f);  //rotate selected item 
		if (Input::wasKeyPressed(SDL_SCANCODE_4)) this->rotateSelected(-10.0f);
		//posible opcion de mover en los axis... problema para coger la rotacion (cuando se hace el setTranslation vuelve a su posicion inicial)
		if (Input::wasKeyPressed(SDL_SCANCODE_9)) this->removeEntities();//remove all entities (9 para no darle por error)
	}

	//movement dentro del editor
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

void EditorLevel::clearEditor() {

	this->removeEntities();
	this->skyMesh = NULL;
	this->groundMesh = NULL;
}

//funciones reusables

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


//MULTIPLAYER 

MultiLevel::MultiLevel() {


	//players
	this->player1 = new sPlayer("data/skelly.mesh", "data/minichar_tex.png", Vector3(0,0,-10));
	this->player1->yaw = 180;
	this->player2 = new sPlayer("data/skelly.mesh", "data/minichar_tex.png", Vector3(0,0,10));
	
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
	
	//HPs
	this->player1HP_quad.createQuad(-78, 90, 30, 5, true);
	this->player2HP_quad.createQuad(22, 90, 30, 5, true);
	this->quadTex = Texture::Get("data/menu/healthbar.tga");
}

void MultiLevel::resetLevel() {
	this->player1->pos = this->player1->spawnPos;
	this->player1->yaw = 180;
	this->player1->health = this->player1->max_health;
	updateHealthBar(-78, player1HP_quad, this->player1);
	this->player2->pos = this->player2->spawnPos;
	this->player2->yaw = 0;
	this->player1->health = this->player1->max_health;
	updateHealthBar(22, player2HP_quad, this->player2);
}

void MultiLevel::Render() {

	int window_width = Game::instance->window_width;
	int window_height = Game::instance->window_height;
	float half_width = window_width * 0.5f;

	//CAM 1
	glViewport(0, 0, half_width, window_height);
	cam1->aspect = half_width / window_height;

	Matrix44 player1Model = this->player1->getModel();
	SetupCam(player1Model, this->cam1);
	

	cam1->enable();
	RenderWorld(cam1);
	//CAM 2

	glViewport(half_width, 0, half_width , window_height);
	cam2->aspect = half_width / window_height;

	Matrix44 player2Model = this->player2->getModel();
	SetupCam(player2Model, this->cam2);
	
	
	cam2->enable();
	RenderWorld(cam2);
	//to reuse global renderworld
	std::vector<sPlayer*> enemies;
	enemies.push_back(this->player2);
	RenderMinimap(half_width, this->player1, enemies, this->groundMesh, this->entities );
	enemies.pop_back();
	enemies.push_back(this->player1);
	RenderMinimap(window_width, this->player2, enemies, this->groundMesh, this->entities);

	//Hp bars
	drawHP(this->player1HP_quad, this->quadTex, Matrix44(),this->cam2D);
	drawHP(this->player2HP_quad, this->quadTex, Matrix44(),this->cam2D);
	//hp txt
	drawText(2.8 * window_width/800, 22 * window_height/600, "HP", Vector3(1, 1, 1), window_width / 400);
	drawText(402.8  *window_width / 800, 22 * window_height / 600, "HP", Vector3(1, 1, 1), window_width / 400);
}


void MultiLevel::Update(float seconds_elapsed) {
	this->player1->updateInvulnerabilityTime(seconds_elapsed);
	this->player2->updateInvulnerabilityTime(seconds_elapsed);

	std::vector<sPlayer*> enemies;
	enemies.push_back(this->player2);
	this->player1->playerMovement(enemies, this->entities, seconds_elapsed, false);
	enemies.pop_back();
	enemies.push_back(this->player1);
	this->player2->playerMovement(enemies,this->entities, seconds_elapsed, true);

	if (Input::wasKeyPressed(SDL_SCANCODE_T)) {
		this->player1->health--;
		this->player2->health--;
		updateHealthBar(-78, this->player1HP_quad, this->player1);
		updateHealthBar(22, this->player2HP_quad, this->player2);
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_P)) {
		this->resetLevel();
	}
}

void MultiLevel::RenderWorld(Camera* cam) {
	//gigantismo
	this->player1->character_mesh->anim = this->player1->renderAnim();
	this->player2->character_mesh->anim = this->player2->renderAnim();

	//this->skyMesh->model.setTranslation(cam->eye.x, cam->eye.y - 20.0f, cam->eye.z);
	glDisable(GL_DEPTH_TEST);
	this->skyMesh->render(cam);//cielo
	glEnable(GL_DEPTH_TEST);

	this->groundMesh->render(cam); //suelo

	this->player1->character_mesh->render(cam);//player
	for (size_t i = 0; i < this->player1->colliders.size(); i++)
	{
		EntityMesh* playerMesh = this->player1->character_mesh;
		this->player1->colliders[i]->updateCollider(playerMesh, cam);
	}

	this->player2->character_mesh->render(cam);//player
	for (size_t i = 0; i < this->player2->colliders.size(); i++)
	{
		EntityMesh* playerMesh = this->player2->character_mesh;
		this->player2->colliders[i]->updateCollider(playerMesh, cam);
	}

	for (size_t i = 0; i < this->entities.size(); i++)//entities added
	{
		EntityMesh* entity = this->entities[i];
		entity->render(cam);
	}
}

 //PLAYABLE LEVEL

PlayLevel::PlayLevel(const char* map, const char* enemiesPath) {

	//player
	this->player = new sPlayer("data/skelly.mesh", "data/minichar_tex.png", Vector3(0,0,0));
	
	//get objects
	ImportMap(map, this->entities, this->groundMesh, this->skyMesh);
	ImportEnemies(enemiesPath, this->enemies);

	//camera
	int window_width = Game::instance->window_width;
	int window_height = Game::instance->window_height;

	this->cam = new Camera();
	this->cam->lookAt(Vector3(0.f, 100.f, 100.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0
	this->cam->setPerspective(70.f, window_width / (float)window_height, 0.1f, 10000.f); //set the projection, we want to be perspective

	//player hp
	this->playerHP_quad.createQuad(-78, 90, 30, 5,true);
	this->quadTex = Texture::Get("data/menu/healthbar.tga");
	//boss 
	this->boss = new sBoss("data/boss/boss.mesh", "data/minichar_tex.png");
	this->entities.push_back(this->boss->character_mesh);
	//this->entities.push_back(this->boss->shuriken_mesh);
	this->boss->pos = Vector3(10, 0, 10);
	
}

void PlayLevel::resetLevel() {
	this->player->pos = this->player->spawnPos;
	this->player->health = this->player->max_health;
	updateHealthBar(-78, playerHP_quad, this->player);
	this->player->yaw = 0;
	//mover a los derrotados de nuevo al array de vivos
	for (size_t i = 0; i < this->enemies_defeated.size(); i++)
	{
		sPlayer* enemy = enemies_defeated[i];
		this->enemies.push_back(enemy);
	}
	//limpiar array de derrotados
	this->enemies_defeated.clear();
	//setear de nuevo los vivos
	for (size_t i = 0; i < this->enemies.size(); i++)
	{
		sPlayer* enemy = enemies[i];
		enemy->pos = enemy->spawnPos;
		enemy->health = enemy->max_health;
		enemy->yaw = 0;
		enemy->invulnerability_time = 0.0f;

	}
}


void PlayLevel::Render() {

	int window_width = Game::instance->window_width;
	int window_height = Game::instance->window_height;

	Matrix44 playerModel = this->player->getModel();
	SetupCam(playerModel, this->cam);

	if (this->player->health > 0) {


		this->player->character_mesh->anim = this->player->renderAnim(); //cojo la anim directamente desde la mesh para no tener que ir pasandola
		this->boss->character_mesh->anim = this->boss->renderAnim();//lo mismo para el boss

		//blendSkeleton(&scene->player.anims[0]->skeleton, &scene->player.anims[scene->player.ctr]->skeleton, 0.3f, &FinalAnim->skeleton);

		//float velFactor = scene->player.playerVel

		this->cam->enable();
		//Render
		//scene->skyMesh->model.setTranslation(camera->eye.x, camera->eye.y - 20.0f, camera->eye.z); solo usarlo si el mapa fuera muy grande
		glDisable(GL_DEPTH_TEST);
		this->skyMesh->render(this->cam);//cielo
		glEnable(GL_DEPTH_TEST);

		this->groundMesh->render(this->cam); //suelo

		this->player->character_mesh->render(this->cam);//player
		for (size_t i = 0; i < this->player->colliders.size(); i++)
		{
			EntityMesh* playerMesh = this->player->character_mesh;
			this->player->colliders[i]->updateCollider(playerMesh, this->cam);
		}

		this->boss->character_mesh->render(this->cam);//boss
		this->boss->Attack(this->cam, this->player->pos);
		if (this->boss->hit) {
			this->boss->hit = false;
			if (this->player->invulnerability_time <= 0.0f) {
				this->player->health--;
				updateHealthBar(-78, this->playerHP_quad, this->player);
				this->player->invulnerability_time = this->player->max_invulnerability_time;
				std::cout << "collison with boss" << std::endl;
			}
		}

		for (size_t i = 0; i < this->entities.size(); i++)//entities added
		{
			EntityMesh* entity = this->entities[i];
			entity->render(this->cam);
		}

		for (size_t i = 0; i < this->enemies.size(); i++)//enemies
		{
			sPlayer* enemy = this->enemies[i];

			if (enemy->health <= 0) {
				this->enemies_defeated.push_back(enemy);
				this->enemies.erase(this->enemies.begin() + i);
			}

			enemy->character_mesh->anim = enemy->renderAnim();

			for (size_t i = 0; i < enemy->colliders.size(); i++)
			{
				EntityMesh* playerMesh = enemy->character_mesh;
				enemy->colliders[i]->updateCollider(playerMesh, this->cam);
			}

			enemy->character_mesh->render(this->cam);

		}

		RenderMinimap(window_width, this->player, this->enemies, this->groundMesh, this->entities);

		//hp
		drawHP(this->playerHP_quad, this->quadTex, Matrix44(), this->cam2D);
		drawText(2.8 * window_width / 800, 22 * window_height / 600, "HP", Vector3(1, 1, 1), window_width / 400);
	}
	else {
		drawText(window_width - window_width * 0.88, window_height - window_height * 0.7, "YOU DIED", Vector3(1, 1, 1), window_width / 60);
		if (int(Game::instance->time) % 2 == 0) {
			drawText(window_width - window_width * 0.94, window_height - window_height * 0.3, "PRESS C TO CONTINUE OR X TO RETURN MENU", Vector3(1, 1, 1), window_width / 250);
		}
	}
}

void PlayLevel::Update(float seconds_elapsed) {
	Scene* s = Game::instance->scene;

	if (this->player->health > 0) {
		this->player->playerMovement(this->enemies, this->entities, seconds_elapsed, false);
		this->player->updateInvulnerabilityTime(seconds_elapsed);
		for (size_t i = 0; i < this->enemies.size(); i++)//enemies
		{

			sPlayer* enemy = this->enemies[i];
			enemy->updateInvulnerabilityTime(seconds_elapsed);
			//borrarse a el mismo de la lista de enemigos a colisionar
			std::vector<sPlayer*> e = this->enemies;
			e.erase(e.begin() + i);
			enemy->npcMovement(e, this->entities, this->player, seconds_elapsed);
		}
		this->boss->npcMovement(this->player, seconds_elapsed);//boss

		//update level when no enemies left
		if (this->enemies.size() <= 0) {
			int nextLevel = (s->currentLevel + 1) % s->levels.size();
			this->resetLevel();
			if (s->currentLevel + 1 < s->levels.size()) {
				s->currentLevel = nextLevel;
			}
			else {

				SetStage(STAGE_ID::END, Game::instance->scene->currentStage);
			}
		}
	}
	else {
		if (Input::wasKeyPressed(SDL_SCANCODE_C)) {
			this->resetLevel();
		}
		if (Input::wasKeyPressed(SDL_SCANCODE_X)) {
			this->resetLevel();
			SetStage(STAGE_ID::INTRO, Game::instance->scene->currentStage);
		}
	}
	//debug levels
	if (Input::wasKeyPressed(SDL_SCANCODE_RIGHT)) { 
		int nextLevel = (s->currentLevel + 1) % s->levels.size();
		this->resetLevel();
		s->currentLevel = nextLevel;
		std::cout << "going to level " << s->currentLevel << std::endl; 
	}
	//debug HP
	if (Input::wasKeyPressed(SDL_SCANCODE_T)) {
		this->player->health --;
		updateHealthBar(-78, this->playerHP_quad, this->player);
	}
	//debug resetLevel
	if (Input::wasKeyPressed(SDL_SCANCODE_P)) {
		this->resetLevel();
	}

}

