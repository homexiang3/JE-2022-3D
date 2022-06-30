#include "entity.h"
#include "camera.h"
#include "game.h"
#include "input.h"

Entity::Entity() {

}
Entity::~Entity() {

}

//constructor de entity mesh para guardarlo de manera mas comoda (crear EntityMesh* y luego hacerle un new EntityMesh(valores), para simplificar solo necesita los paths)
EntityMesh::EntityMesh(int primitive, std::string meshPath, std::string texturePath, char* shaderPath1, char* shaderPath2, Vector4 color) {
	
	this->primitive = primitive;

	if (!meshPath.empty()) {
		this->mesh = Mesh::Get(meshPath.c_str());
		this->meshPath = meshPath;
	}
	if (!texturePath.empty()) {
		this->texture = Texture::Get(texturePath.c_str());
		this->texturePath = texturePath;
	}
	
	this->shader = Shader::Get(shaderPath1, shaderPath2);
	this->color = color;
	this->tiling = 1.0f;
	this->texturePath = texturePath;
}

void Entity::render(Camera* cam) {
	//if it was entity mesh, Matrix44 model = getGlobalMatrix(), render code...
	//Camera* camera = Camera::current;
	Matrix44 model = this->model;


	for (int i = 0; i < children.size(); i++)
	{
		children[i]->render(cam); //repeat for each child
	}
}

void Entity::update(float seconds_elapsed) {

};

Vector3 Entity::getPosition() {
	return Vector3(0, 0, 0); //only test
}

void Entity::destroy(std::vector<Entity*> s_to_destroy) {
	s_to_destroy.push_back(this);
	//for every children call destroy
}

Matrix44 Entity::getGlobalMatrix() { //recursive get matrix of parents
	if (parent)
		return model * parent->getGlobalMatrix();
	return model;
}

//Render del entitymesh para pintar el objeto

void EntityMesh::render(Camera* camera) {

	Shader* a_shader = this->shader;
	Mesh* a_mesh = this->mesh;
	Texture* tex = this->texture;
	Vector4 color = this->color;
	float tiling = this->tiling;

	//comprueba que esten cargados
 	assert(a_mesh != NULL, "mesh in Entity Render was null"); //debug
	assert(a_shader != NULL, "shader in Entity Render was null");

	if (!a_shader) return;

	//get the last camera that was activated
	Matrix44 model = this->model;
	BoundingBox aabb = this->aabb;
	int primitive = this->primitive;
	aabb = transformBoundingBox(model, a_mesh->box);

	/*distance check for flod

	Vector3 planePos = model.getTranslation();
	Vector3 camPos = camera->eye;
	float dist = planePos.distance(camPos);

	if (dist > flod_dist) render low quality mesh
	*/

	//frustrum
	if (camera->testBoxInFrustum(aabb.center, aabb.halfsize)) {

		//enable shader and pass uniforms optimizar todo lo que se pueda
		a_shader->enable(); //esta parte se puede optimizar si son iguales los shaders..
		a_shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
		
		if (tex != NULL) {
			a_shader->setUniform("u_texture", tex, 0);
			a_shader->setUniform("u_tex_tiling", tiling);
		}

		a_shader->setUniform("u_color", color);
		a_shader->setUniform("u_time", Game::instance->time);

		mesh->enableBuffers(a_shader);
		//parte dinamica que se tendria q pintar diferente en cada objeto
		a_shader->setUniform("u_model", model);
		
		//render the mesh using the shader
		//a_mesh->render(primitive);

		//disable the shader after finishing rendering
		mesh->disableBuffers(a_shader);
		//anims
		if (!this->anim == NULL) {
			a_mesh->renderAnimated(GL_TRIANGLES, &this->anim->skeleton);//los players la tendran no null
		}
		else a_mesh->render(primitive);

		a_shader->disable();

		//debug to see bounding box 
		//a_mesh->renderBounding(model); //añade 2 DC adicionales a cada mesh
	}
}

void EntityMesh::update(float dt) {

};
//collider
void Collider::updateCollider(EntityMesh* playerMesh, Camera* camera) {
	if (!playerMesh->anim == NULL) {
		Matrix44 neckLocalMatrix = playerMesh->anim->skeleton.getBoneMatrix(this->name, false);
		Matrix44 localToWorldMatrix = neckLocalMatrix * playerMesh->model;
		localToWorldMatrix.scale(0.1, 0.1, 0.1);
		this->colliderMesh->model = localToWorldMatrix;
		//this->colliderMesh->render(camera); //solo activar para debug
	}
	else {
		std::cout << "Player mesh didn't have animation" << std::endl;
	}
}
//sPlayer functions 
sPlayer::sPlayer(const char* meshPath, const char* texPath, Vector3 spawn) {
	initAnims();
	initColliders();
	this->spawnPos = spawn;
	this->pos = spawn;
	this->character_mesh = new EntityMesh(GL_TRIANGLES, meshPath, texPath, "data/shaders/skinning.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	this->character_mesh->anim = this->anims[0];
	this->shield = new EntityMesh(GL_TRIANGLES, "data/shield.obj", "data/T_shield2.jpg", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));

}

Matrix44 sPlayer::getModel() {
	Matrix44 model;
	model.translate(pos.x, pos.y, pos.z);
	model.rotate(yaw * DEG2RAD, Vector3(0, 1, 0));
	this->character_mesh->model = model;
	return  model;
}
void sPlayer::initAnims() {

	char* names[8] = { "data/anims/idle.skanim" ,"data/anims/walk.skanim","data/anims/run.skanim",
					"data/anims/left_punch.skanim","data/anims/kick.skanim","data/anims/dash.skanim",
					"data/anims/block.skanim","data/anims/hit.skanim" };

	this->anims.reserve(8);

	for (int i = 0; i < 8; i++)
	{
		std::cout << names[i] << std::endl;
		Animation* now = Animation::Get(names[i]);
		this->anims.push_back(now);
	}

}

void sPlayer::initColliders() {

	Collider* rightHand = new Collider();
	rightHand->name = "mixamorig_RightHandThumb3";
	rightHand->colliderMesh = new EntityMesh(GL_TRIANGLES, "data/sphere.obj", "data/minichar_tex.png", "data/shaders/skinning.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));

	Collider* leftHand = new Collider();
	leftHand->name = "mixamorig_LeftHandThumb3";
	leftHand->colliderMesh = new EntityMesh(GL_TRIANGLES, "data/sphere.obj", "data/minichar_tex.png", "data/shaders/skinning.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));

	Collider* rightToe = new Collider();
	rightToe->name = "mixamorig_LeftToe_End";
	rightToe->colliderMesh = new EntityMesh(GL_TRIANGLES, "data/sphere.obj", "data/minichar_tex.png", "data/shaders/skinning.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));

	Collider* leftToe = new Collider();
	leftToe->name = "mixamorig_RightToe_End";
	leftToe->colliderMesh = new EntityMesh(GL_TRIANGLES, "data/sphere.obj", "data/minichar_tex.png", "data/shaders/skinning.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));

	this->colliders.reserve(4);
	this->colliders.push_back(rightHand);
	this->colliders.push_back(leftHand);
	this->colliders.push_back(rightToe);
	this->colliders.push_back(leftToe);
}
Vector3 sPlayer::playerCollision(std::vector<sPlayer*> enemies,  std::vector<EntityMesh*> entities, Vector3 nextPos, float seconds_elapsed, EntityMesh* boss) {
	//TEST COLLISIONS, HABRIA QUE TENER DINAMICAS - ESTATICAS, DINAMICAS - DINAMICAS, PLAYER - COSAS ETC...
	//calculamos el centro de la esfera de colisión del player elevandola hasta la cintura
	Vector3 character_center = nextPos + Vector3(0, 1, 0);

	//para cada objecto de la escena...
	for (size_t i = 0; i < entities.size(); i++)
	{
		EntityMesh* currentEntity = entities[i];
		//comprobamos si colisiona el objeto con la esfera (radio 3)
		Vector3 coll;
		Vector3 collnorm;
		if (!currentEntity->mesh->testSphereCollision(currentEntity->model, character_center, this->radius , coll, collnorm))
			continue; //si no colisiona, pasamos al siguiente objeto

		//si la esfera está colisionando muevela a su posicion anterior alejandola del objeto
		Vector3 push_away = normalize(coll - character_center) * seconds_elapsed;
		nextPos = this->pos - push_away; //move to previous pos but a little bit further

		//cuidado con la Y, si nuestro juego es 2D la ponemos a 0
		nextPos.y = 0;

		//reflejamos el vector velocidad para que de la sensacion de que rebota en la pared
		//velocity = reflect(velocity, collnorm) * 0.95;

		return nextPos;
	}

	//para cada enemigo...
	for (size_t i = 0; i < enemies.size(); i++)
	{
		sPlayer* currentEnemy = enemies[i];

		//comprobamos si colisiona el objeto con la esfera (radio 3)
		Vector3 coll;
		Vector3 collnorm;
		EntityMesh* enemyEntity = currentEnemy->character_mesh;

		if (!enemyEntity->mesh->testSphereCollision(enemyEntity->model, character_center, this->radius , coll, collnorm))
			continue; //si no colisiona, pasamos al siguiente objeto

		//si la esfera está colisionando muevela a su posicion anterior alejandola del objeto
		Vector3 push_away = normalize(coll - character_center) * seconds_elapsed;
		nextPos = this->pos - push_away;

		

		//cuidado con la Y, si nuestro juego es 2D la ponemos a 0
		nextPos.y = 0;

		//reflejamos el vector velocidad para que de la sensacion de que rebota en la pared
		//velocity = reflect(velocity, collnorm) * 0.95;
		return nextPos;
	}


	if (boss != NULL) {
		Vector3 coll;
		Vector3 collnorm;
		if (boss->mesh->testSphereCollision(boss->model, character_center, this->radius * 80, coll, collnorm)) {
			
			//si la esfera está colisionando muevela a su posicion anterior alejandola del objeto
			Vector3 push_away = normalize(coll - character_center) * seconds_elapsed;
			nextPos = this->pos - push_away;

			//cuidado con la Y, si nuestro juego es 2D la ponemos a 0
			nextPos.y = 0;

			//reflejamos el vector velocidad para que de la sensacion de que rebota en la pared
			//velocity = reflect(velocity, collnorm) * 0.95;
			return nextPos;
		}
	}
	


	return nextPos;
}
void sPlayer::attackCollision(std::vector<sPlayer*> enemies) {

	for (size_t i = 0; i < enemies.size(); i++)
	{
		sPlayer* currentEnemy = enemies[i];

		//calcular si esta enfrente o detras
		Matrix44 model = this->getModel();
		Vector3 forward = model.rotateVector(Vector3(0, 0, -1)).normalize();

		Matrix44 enemyModel = currentEnemy->getModel();
		Vector3 enemyForward = enemyModel.rotateVector(Vector3(0, 0, -1)).normalize();

		float frontOrBack = enemyForward.dot(forward);
	   //if + back - front

		//comprobamos si colisiona el objeto con la esfera (radio 3)
		Vector3 coll;
		Vector3 collnorm;
		EntityMesh* enemyEntity = currentEnemy->character_mesh;

		//testeo collision
		for (size_t i = 0; i < this->colliders.size(); i++)
		{
			Collider* currentCollider = this->colliders[i];
			Vector3 coll_center = currentCollider->colliderMesh->model.getTranslation();
			int attack_dmg = this->attack_dmg;

			if (currentEnemy->invulnerability_time > 0)
				continue;

			if (currentEnemy->Protected && frontOrBack <= 0.3) //block
				continue;
			
			if (frontOrBack > 0.3) //si te ataca por la espalda te saca el doble
				attack_dmg *= 2;

			if (enemyEntity->mesh->testSphereCollision(enemyEntity->model, coll_center, 1, coll, collnorm)) {
				Scene* s = Game::instance->scene;
				s->audio->PlayGameSound(1, 1);
				currentEnemy->invulnerability_time = currentEnemy->max_invulnerability_time;
				currentEnemy->character_mesh->color = Vector4(1, 0, 0, 1);
				currentEnemy->health -= attack_dmg;
				//currentEnemy->ChangeAnim(7, this->anims[7]->duration * 0.5);
				std::cout << "Enemy Health " << currentEnemy->health << std::endl;
				
			}
		}

	}
}


void sPlayer::playerMovement(std::vector<sPlayer*> enemies,std::vector<EntityMesh*> entities,float seconds_elapsed, bool multi, EntityMesh* boss) {
	Scene* scene = Game::instance->scene;

	bool isRunning = false;
	float walk_speed = 10.0f * seconds_elapsed;
	float run_speed = 20.0f * seconds_elapsed;

	this->animTimer = max(0.0f, this->animTimer - seconds_elapsed);

	//definir teclas para multi o solo

	int upKey = SDL_SCANCODE_W;
	int downKey = SDL_SCANCODE_S;
	int rightRotKey = SDL_SCANCODE_D;
	int leftRotKey = SDL_SCANCODE_A;
	int punchKey = SDL_SCANCODE_Z;
	int kickKey = SDL_SCANCODE_V;
	int runKey = SDL_SCANCODE_LSHIFT;
	int dashKey = SDL_SCANCODE_X;
	int shield = SDL_SCANCODE_E;

	if (multi) {
		upKey = SDL_SCANCODE_UP;
		downKey = SDL_SCANCODE_DOWN;
		rightRotKey = SDL_SCANCODE_RIGHT;
		leftRotKey = SDL_SCANCODE_LEFT;
		punchKey = SDL_SCANCODE_M;
		kickKey = SDL_SCANCODE_N;
		runKey = SDL_SCANCODE_RSHIFT;
		dashKey = SDL_SCANCODE_K;
		shield = SDL_SCANCODE_J;
	}

	//punch 
	if (Input::wasKeyPressed(punchKey) && this->animTimer <= 0.2f && this->ctr != 7) {
		this->side = this->side * -1;
		this->ChangeAnim(3, this->anims[3]->duration / 1.8 - 0.05);
		scene->audio->PlayGameSound(4, 1);
	}

	//kick
	if (Input::wasKeyPressed(kickKey) && this->animTimer <= 0.2f && this->ctr != 7) {
		this->side = this->side * -1;
		this->ChangeAnim(4, this->anims[4]->duration / 1.8 - 0.2);
		scene->audio->PlayGameSound(5, 1);
	}


	//return to idle on was key anims
	if (this->animTimer <= 0.0f) ChangeAnim(0, NULL);

	//sprint
	if (Input::isKeyPressed(runKey)) isRunning = true;
	this->playerVel = (isRunning == true) ? run_speed : walk_speed;

	//shield
	this->shield->model.setTranslation(this->pos.x, 0, this->pos.z);
	this->shield->model.rotate(180.0f * DEG2RAD * Game::instance->time, Vector3(0, 1, 0));
	if (Input::isKeyPressed(shield) && this->ctr != 7 ) {
		this->Protected = true;
		this->ChangeAnim(6, NULL);
	}
	if (this->ctr != 6) this->Protected = false;

	//rotation
	float rotSpeed = 120.0f * seconds_elapsed;

	//player movement code
	Matrix44 playerRotation;
	playerRotation.rotate(this->yaw * DEG2RAD, Vector3(0, 1, 0));

	Vector3 forward = playerRotation.rotateVector(Vector3(0, 0, -1));
	Vector3 right = playerRotation.rotateVector(Vector3(1, 0, 0));
	Vector3 up = playerRotation.rotateVector(Vector3(0, 1, 0));

	Vector3 playerVel;

	if (Input::isKeyPressed(upKey) && this->ctr != 5 && this->ctr != 6 && this->ctr != 7)
	{
		playerVel = playerVel + (forward * this->playerVel);
		this->ChangeAnim(1, NULL);
	}
	if (Input::isKeyPressed(downKey) && this->ctr != 5 && this->ctr != 6 && this->ctr != 7)
	{
		playerVel = playerVel - (forward * this->playerVel);
		this->ChangeAnim(1, NULL);
	}
	if (Input::isKeyPressed(rightRotKey) && this->ctr != 7) this->yaw += rotSpeed;
	if (Input::isKeyPressed(leftRotKey) && this->ctr != 7) this->yaw -= rotSpeed;

	
	if (Input::isKeyPressed(runKey)) ChangeAnim(2, NULL);// run anim

	Vector3 nextPos = this->pos + playerVel;

	//dash
	Vector2 dash_dir = Vector2(this->pos.x - nextPos[0], this->pos.z - nextPos[2]).normalize();
	if (dash_dir.x != 0 || dash_dir.y != 0)this->dash_direction = dash_dir;

	//std::cout << scene->player.dash_direction.x << "  " << scene->player.dash_direction.y << std::endl;

	Vector3 playerVec = this->character_mesh->model.frontVector().normalize();
	float modifier = (isRunning == true) ? 0.65 : 2; // to fix the dash disparity
	float sumX = (15.0 * this->dash_direction.x) * modifier;
	float sumZ = (15.0 * this->dash_direction.y) * modifier;

	if (this->ctr == 5) {
		playerVel[0] -= sumX;
		playerVel[2] -= sumZ;
	}
	if (Input::wasKeyPressed(dashKey) && this->ctr != 6 && this->ctr != 7 ) {
		std::cout << sumX << "  " << sumZ << std::endl;
		playerVel[0] -= sumX;
		playerVel[2] -= sumZ;
		this->ChangeAnim(5, 0.25f);
		scene->audio->PlayGameSound(3, 1);
	}

	nextPos = this->pos + playerVel;

	nextPos = this->playerCollision(enemies, entities, nextPos, seconds_elapsed, boss);

	this->pos = nextPos;
}

void sPlayer::npcMovement(std::vector<sPlayer*> enemies, std::vector<EntityMesh*> entities, sPlayer* player, float seconds_elapsed, EntityMesh* boss) {

	this->animTimer = max(0.0f, this->animTimer - seconds_elapsed);

	Matrix44 npcModel = this->getModel();
	Vector3 side = npcModel.rotateVector(Vector3(1, 0, 0)).normalize();
	Vector3 forward = npcModel.rotateVector(Vector3(0, 0, -1)).normalize();


	Vector3 toTarget = player->pos - this->pos;
	float dist = toTarget.length();
	toTarget.normalize();

	float sideDot = side.dot(toTarget);
	float forwardDot = forward.dot(toTarget);


	if (forwardDot < 0.98f) {
		this->yaw += 100.0f * sign(sideDot) * seconds_elapsed;
	}

	if (dist > 2.0f) {
		this->ChangeAnim(2, NULL);
		Vector3 nextPos = this->pos + (forward * this->playerVel * seconds_elapsed);
		nextPos = this->playerCollision(enemies, entities, nextPos, seconds_elapsed, boss);
		this->pos = nextPos;

	}
	else {
		
		this->ChangeAnim(3, this->anims[3]->duration / 1.8 - 0.05);
		std::vector<sPlayer*> playerEnemy;
		playerEnemy.push_back(player);
		if (this->ctr == 3) { //punch
			this->attackCollision(playerEnemy);
		}

	}
}
void sPlayer::ChangeAnim( int i, float time)
{
	if (this->animTimer <= 0.0f) this->ctr = i;
	if (time != NULL) this->animTimer = time;
}

Animation* sPlayer::renderAnim() {
	
	Matrix44 playerModel = this->getModel();

	playerModel.scale(this->side * 0.015, 0.015, 0.015);
	playerModel.rotate(180 * DEG2RAD, Vector3(0, 1, 0)); //quitar si se gira la mesh
	this->character_mesh->model = playerModel;

	//animations 
	float time = getTime() * 0.001;
	float t = fmod(time, this->anims[0]->duration) / this->anims[2]->duration;
	/*
	scene->player.walk->assignTime(t * scene->player.walk->duration);
	scene->player.run->assignTime(t * scene->player.run->duration);
	scene->player.idle->assignTime(time * scene->player.idle->duration);
	*/
	for (int i = 0; i < this->anims.size(); i++) {
		this->anims[i]->assignTime(time);
		if (i == 3 || i == 6 || i == 4) this->anims[i]->assignTime(time * 1.8);
		else this->anims[i]->assignTime(time);
	}


	Animation* FinalAnim = this->anims[this->ctr];
	return FinalAnim;
}

void sPlayer::updateInvulnerabilityTime(float seconds_elapsed) {
	if (this->invulnerability_time > 0) {
		this->invulnerability_time -= seconds_elapsed;
	}
	else {
		this->invulnerability_time = 0;
		this->character_mesh->color = Vector4(1, 1, 1, 1);
	}
}


//sBoss functions 
sBoss::sBoss(const char* meshPath, const char* texPath, Vector3 spawn) {


	initAnims();
	this->spawnPos = spawn;
	this->pos = spawn;
	this->max_health = 20;
	this->health = 20;

	this->character_mesh = new EntityMesh(GL_TRIANGLES, meshPath, texPath, "data/shaders/skinning.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	this->character_mesh->anim = this->anims[0];

	this->shuriken_mesh = new EntityMesh(GL_TRIANGLES, "data/boss/shuriken.obj", "data/boss/shuriken_tex.png", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	this->shuriken_mesh->model.setTranslation(4, 4, 0);
	//this->shuriken_mesh->model.scale(10, 10, 10);

	EntityMesh* plane1 = new EntityMesh(GL_TRIANGLES, "data/boss/rectangleAttack.obj", "data/boss/shuriken_tex.png", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	plane1->model.scale(0.008, 0.008, 0.008);

	EntityMesh* plane2 = new EntityMesh(GL_TRIANGLES, "data/boss/circleAttack.obj", "data/boss/shuriken_tex.png", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	plane2->model.scale(0.008, 0.008, 0.008);

	EntityMesh* plane3 = new EntityMesh(GL_TRIANGLES, "data/boss/coneAttack.obj", "data/boss/shuriken_tex.png", "data/shaders/basic.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
	plane3->model.scale(0.01, 0.01, 0.01);

	this->planes.push_back(plane1);
	this->planes.push_back(plane2);
	this->planes.push_back(plane3);
}

Matrix44 sBoss::getModel() {
	Matrix44 model;
	model.translate(pos.x, pos.y, pos.z);
	model.rotate(yaw * DEG2RAD, Vector3(0, 1, 0));
	this->character_mesh->model = model;
	return  model;
}
void sBoss::initAnims() {

	char* names[9] = { "data/boss/anim/idle.skanim" ,"data/boss/anim/running.skanim","data/boss/anim/rest.skanim",
					"data/boss/anim/death.skanim" , "data/boss/anim/charging.skanim" ,"data/boss/anim/shurikenSpawn.skanim" ,
					"data/boss/anim/attack1Rectangle.skanim" ,"data/boss/anim/attack2Circular.skanim","data/boss/anim/attack3Cone.skanim" };

	this->anims.reserve(9);

	for (int i = 0; i < 9; i++)
	{
		std::cout << names[i] << std::endl;
		Animation* now = Animation::Get(names[i]);
		this->anims.push_back(now);
	}

	
}

void sBoss::npcMovement(std::vector<sPlayer*> enemies, std::vector<EntityMesh*> entities,sPlayer* player, float seconds_elapsed, EntityMesh* boss) {

	this->animTimer = max(0.0f, this->animTimer - seconds_elapsed);
	this->invulnerability_time = max(0.0f, this->invulnerability_time - seconds_elapsed);

	Matrix44 npcModel = this->getModel();
	Vector3 side = npcModel.rotateVector(Vector3(1, 0, 0)).normalize();
	Vector3 forward = npcModel.rotateVector(Vector3(0, 0, -1)).normalize();

	Vector3 toTarget = player->pos - this->pos;
	if (this->state > 0) toTarget = this->attackTo;
	float dist = toTarget.length();
	toTarget.normalize();

	float sideDot = side.dot(toTarget);
	float forwardDot = forward.dot(toTarget);

	if (forwardDot < 0.98f) {
		this->yaw += 90.0f * sign(sideDot) * seconds_elapsed;
	}
	if (this->state <= 0) {
		if (dist > 2.0f) {

			Vector3 nextPos = this->pos + (forward * this->playerVel * seconds_elapsed);
			nextPos = this->playerCollision(enemies, entities, nextPos, seconds_elapsed, boss);
			this->pos = nextPos;
		}
		else {
			//attack
			this->attackTimer += 0.02;
		}
	}


}

void sBoss::ChangeAnim(int i, float time)
{
	if (this->animTimer <= 0.0f) this->ctr = i;
	if (time != NULL) this->animTimer = time;
}

Animation* sBoss::renderAnim() {

	Matrix44 bossModel = this->getModel();

	bossModel.scale(this->side * 0.03, 0.03, 0.03);
	bossModel.rotate(180 * DEG2RAD, Vector3(0, 1, 0));
	this->character_mesh->model = bossModel;

	float time = getTime() * 0.001;
	//float t = fmod(time, this->idle->duration) / this->run->duration;

		for (int i = 0; i < this->anims.size(); i++) {
			this->anims[i]->assignTime(time);
		}


		Animation* FinalAnim = this->anims[this->ctr];
		return FinalAnim;
	}

	float distance(int x1, int y1, int x2, int y2)
	{
		// Calculating distance
		//std::cout << sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0) << std::endl;
		return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0);
	}

	// boss attacks
	void sBoss::shurikenAttack(Camera * cam, Vector3 playerpos)
	{
		const int projectiles_number = 10;
		float t = getTime() * 0.03;

		for (size_t i = 0; i < projectiles_number; i++)
		{
			if (this->shurkikens[i]) continue;
			float rad = (360 / projectiles_number) * (i + t * 0.03) * DEG2RAD;
			float sin_pos = 10 * sin(rad) + this->pos.x;
			float cos_pos = 10 * cos(rad) + this->pos.z;
			this->shuriken_mesh->model.setTranslation(sin_pos, 1.5, cos_pos);
			this->shuriken_mesh->model.rotate(20.0 * DEG2RAD * t, Vector3(0, 1, 0));
			this->shuriken_mesh->model.scale(10, 10, 10);
			this->shuriken_mesh->render(cam);

			if (distance(sin_pos, cos_pos, playerpos.x, playerpos.z) < 2 && this->invulnerability_time <= 0.0f) {
				this->shurkikens[i] = true;
				this->hit = true;
				this->invulnerability_time = this->max_invulnerability_time;
			}

		}
		//std::cout << "osi" <<this->pos.x << "   " << this->pos.y << "  " << this->pos.z << std::endl;
		//std::cout << this->character_mesh->model.getTranslation().x << "   " << character_mesh->model.getTranslation().y << "  " <<character_mesh->model.getTranslation().z << std::endl;

	}
	void sBoss::katanaRender(Camera * cam)
	{
		//Matrix44 neckLocalMatrix = this->anim->skeleton.getBoneMatrix(texts[i], false);
		Matrix44 neckLocalMatrix = this->character_mesh->anim->skeleton.getBoneMatrix("mixamorig_RightHandThumb1", false);//mixamorig_LeftHandIndex2

		Matrix44 localToWorldMatrix = neckLocalMatrix * this->character_mesh->model;
		//localToWorldMatrix.scale(0.03, 0.03, 0.03);
		EntityMesh* rightHand = new EntityMesh(GL_TRIANGLES, "data/boss/naginata.obj", "data/boss/tex2.png", "data/shaders/skinning.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
		localToWorldMatrix.translate(10, 0, 10);
		//localToWorldMatrix.rotate(180 * DEG2RAD, Vector3(1, 0, 0));//x
		rightHand->model = localToWorldMatrix;
		//rightHand->model.rotate((-47+90) * DEG2RAD, Vector3(1, 0, 0));//x
		//rightHand->model.rotate((22) * DEG2RAD, Vector3(0, 1, 0));//y
		//rightHand->model.rotate((+16+90) * DEG2RAD, Vector3(0, 0, -1)); //z
		//rightHand->model.rotate(90 * DEG2RAD, Vector3(1, 0, 0));//x
		//rightHand->model.rotate((22) * DEG2RAD, Vector3(0, 1, 0));//y
		//rightHand->model.rotate((+16+90) * DEG2RAD, Vector3(0, 0, -1)); //z

		rightHand->render(cam);
		//RenderMesh(GL_TRIANGLES, localToWorldMatrix, mesh, tex, shader, camera);
	}

	void sBoss::Attack(Camera * cam, Vector3 playerpos) {
		//std::cout << this->character_mesh->model.frontVector().x << "   " << this->character_mesh->model.frontVector().y << "  " << this->character_mesh->model.frontVector().z << std::endl;

		this->katanaRender(cam);

		if (this->health < this->max_health/2) {
			if (!this->animShurikens) {
				this->ChangeAnim(5, this->anims[5]->duration);
				this->animShurikens = true;

			}
			else if (this->ctr == 5 && this->animTimer <= 0.0f) {
				this->ctr = 1;

			}

			this->shurikenAttack(cam, playerpos);
		}

		if (this->attackTimer >= 2.0f && this->ctr != 5) {
			if (this->ctr != 4 && this->ctr < 6 && this->ctr != 2) {
				this->ChangeAnim(4, this->anims[4]->duration * 0.75); // si las animaciones son distintas de cargar o atacar
				this->attackNumb = (rand() * rand()) % 3;

				if (this->attackNumb == 1) this->attackTo = Vector3(0, 0, 0.000001);
				else {
					this->attackTo = playerpos - this->pos;
					this->attackTo = this->attackTo.normalize();
				}
				this->state = 1;
				this->attackCounter++;
			}
			if (this->ctr == 4 && this->animTimer > 0.0f) {
				EntityMesh* plane = this->planes[this->attackNumb];

				float clr = 1.4 - (this->animTimer / this->anims[this->ctr]->duration);
				//clr = clr % 255;

				plane->color = Vector4(clr, 0, 0, 1);
				plane->model.setTranslation(this->pos.x + (5 * this->attackTo.x), 0.1, this->pos.z + (5 * this->attackTo.z));
				plane->model.setFrontAndOrthonormalize(this->attackTo);
				plane->model.scale(0.008, 0.008, 0.008);
				//plane->model.rotate(90 * DEG2RAD, Vector3(1, 0, 0));
				//plane->model.rotate(90 * DEG2RAD, Vector3(0, 0, 1));
				plane->render(cam);
			}
			else if ((this->ctr >= 6 || this->ctr == 2) && this->animTimer <= 0.0f) {//reset
				std::cout << this->attackCounter << std::endl;
				if (this->attackCounter >= 3) {
					this->ChangeAnim(2, 5.0f);
					this->attackCounter = 0;
					this->attackNumb = 0;
					this->state = 1;
				}
				else {
					this->ctr = 1;
					this->attackTimer = 0.0f;
					this->state = 0;
				}
			}
			else if (this->ctr != 2) {

				if (this->ctr < 5)this->ChangeAnim(this->attackNumb + 6, this->anims[this->attackNumb + 1]->duration * 0.5); // attack anim el +5 es porque los ataques 0, 1, 2 son el numero en el vector de planos y +5 es en vector de animaciones

				EntityMesh* plane = this->planes[this->attackNumb];
				plane->color = Vector4(1.4, 0, 0, 1);
				float x = this->pos.x + (5 * this->attackTo.x);
				float z = this->pos.z + (5 * this->attackTo.z);
				plane->model.setTranslation(x, 0.1, z);
				plane->model.setFrontAndOrthonormalize(this->attackTo);
				plane->model.scale(0.008, 0.008, 0.008);
				plane->render(cam);

				if (distance(x, z, playerpos.x, playerpos.z) <= 5.0f && this->invulnerability_time <= 0.0f) {
					this->hit = true;
					this->invulnerability_time = this->max_invulnerability_time;
				}

			}
		}
	}

	void sPlayer::reset(float yaw) {
		this->invulnerability_time = 0.0f;
		this->pos = this->spawnPos;
		this->yaw = yaw;
		this->health = this->max_health;
		this->animTimer = 0;
	}
