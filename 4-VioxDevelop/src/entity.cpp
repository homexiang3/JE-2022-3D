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

			/*Balls on hitpoints(not sure how to compute collisions)
			char* texts[4] = { "mixamorig_RightHandThumb3","mixamorig_LeftHandThumb3", "mixamorig_LeftToe_End","mixamorig_RightToe_End" };
			for (int i = 0; i < sizeof(texts)/sizeof(texts[0]); i++)
			{
				Matrix44 neckLocalMatrix = this->anim->skeleton.getBoneMatrix(texts[i], false);
				Matrix44 localToWorldMatrix = neckLocalMatrix * model;
				localToWorldMatrix.scale(0.1, 0.1, 0.1);
				EntityMesh* rightHand = new EntityMesh(GL_TRIANGLES, "data/sphere.obj", "data/minichar_tex.png", "data/shaders/skinning.vs", "data/shaders/texture.fs", Vector4(1, 1, 1, 1));
				rightHand->model = localToWorldMatrix;
				rightHand->render(camera);
				//RenderMesh(GL_TRIANGLES, localToWorldMatrix, mesh, tex, shader, camera);
			}*/
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
		this->colliderMesh->render(camera);
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

}

Matrix44 sPlayer::getModel() {
	Matrix44 model;
	model.translate(pos.x, pos.y, pos.z);
	model.rotate(yaw * DEG2RAD, Vector3(0, 1, 0));
	this->character_mesh->model = model;
	return  model;
}
void sPlayer::initAnims() {

	this->idle = Animation::Get("data/anims/idle.skanim");
	this->walk = Animation::Get("data/anims/walk.skanim");
	this->run = Animation::Get("data/anims/run.skanim");

	this->left_puch = Animation::Get("data/anims/left_punch.skanim");
	this->kick = Animation::Get("data/anims/kick.skanim");
	this->dash = Animation::Get("data/anims/dash.skanim");
	this->jump = Animation::Get("data/anims/jump.skanim");

	this->anims.reserve(7);
	this->anims.push_back(this->idle);
	this->anims.push_back(this->walk);
	this->anims.push_back(this->run);
	this->anims.push_back(this->left_puch);
	this->anims.push_back(this->kick);
	this->anims.push_back(this->dash);
	this->anims.push_back(this->jump);
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
Vector3 sPlayer::playerCollision(std::vector<sPlayer*> enemies,  std::vector<EntityMesh*> entities, Vector3 nextPos, float seconds_elapsed) {
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

	return nextPos;
}
void sPlayer::punchCollision(std::vector<sPlayer*> enemies) {

	for (size_t i = 0; i < enemies.size(); i++)
	{
		sPlayer* currentEnemy = enemies[i];

		//comprobamos si colisiona el objeto con la esfera (radio 3)
		Vector3 coll;
		Vector3 collnorm;
		EntityMesh* enemyEntity = currentEnemy->character_mesh;

		//testeo collision
		for (size_t i = 0; i < this->colliders.size() / 2; i++)
		{
			Collider* currentCollider = this->colliders[i];
			Vector3 coll_center = currentCollider->colliderMesh->model.getTranslation();
			if (currentEnemy->invulnerability_time > 0)
				continue;

			if (enemyEntity->mesh->testSphereCollision(enemyEntity->model, coll_center, 1, coll, collnorm)) {
				std::cout << "collision on " << currentCollider->name << std::endl;
				currentEnemy->invulnerability_time = currentEnemy->max_invulnerability_time;
				currentEnemy->character_mesh->color = Vector4(1, 0, 0, 1);
				currentEnemy->health -= 1;
				std::cout << "Enemy Health " << currentEnemy->health << std::endl;
				
			}
		}

	}
}
void sPlayer::kickCollision(std::vector<sPlayer*> enemies) {

	for (size_t i = 0; i < enemies.size(); i++)
	{
		sPlayer* currentEnemy = enemies[i];

		//comprobamos si colisiona el objeto con la esfera (radio 3)
		Vector3 coll;
		Vector3 collnorm;
		EntityMesh* enemyEntity = currentEnemy->character_mesh;

		//testeo collision
		for (size_t i = 2; i < this->colliders.size(); i++)
		{
			Collider* currentCollider = this->colliders[i];
			Vector3 coll_center = currentCollider->colliderMesh->model.getTranslation();
			if (currentEnemy->invulnerability_time > 0)
				continue;

			if (enemyEntity->mesh->testSphereCollision(enemyEntity->model, coll_center, 1, coll, collnorm)) {
				std::cout << "collision on " << currentCollider->name << std::endl;
				currentEnemy->invulnerability_time = currentEnemy->max_invulnerability_time;
				currentEnemy->character_mesh->color = Vector4(1, 0, 0, 1);
				currentEnemy->health -= 1;
				std::cout << "Enemy Health " << currentEnemy->health << std::endl;
			}
		}

	}
}

void sPlayer::playerMovement(std::vector<sPlayer*> enemies,std::vector<EntityMesh*> entities,float seconds_elapsed, bool multi) {
	Scene* scene = Game::instance->scene;

	bool isRunning = false;
	float walk_speed = 10.0f * seconds_elapsed;
	float run_speed = 20.0f * seconds_elapsed;

	this->jumpLock = max(0.0f, this->jumpLock - seconds_elapsed);
	this->animTimer = max(0.0f, this->animTimer - seconds_elapsed);

	//definir teclas para multi o solo

	int upKey = SDL_SCANCODE_W;
	int downKey = SDL_SCANCODE_S;
	int rightRotKey = SDL_SCANCODE_D;
	int leftRotKey = SDL_SCANCODE_A;
	int punchKey = SDL_SCANCODE_Z;
	int kickKey = SDL_SCANCODE_V;
	int runKey = SDL_SCANCODE_LSHIFT;
	int jumpKey = SDL_SCANCODE_C;
	int dashKey = SDL_SCANCODE_X;

	if (multi) {
		upKey = SDL_SCANCODE_UP;
		downKey = SDL_SCANCODE_DOWN;
		rightRotKey = SDL_SCANCODE_RIGHT;
		leftRotKey = SDL_SCANCODE_LEFT;
		punchKey = SDL_SCANCODE_M;
		kickKey = SDL_SCANCODE_N;
		runKey = SDL_SCANCODE_RSHIFT;
		jumpKey = SDL_SCANCODE_L;
		dashKey = SDL_SCANCODE_K;
	}

	//punch 
	if (Input::wasKeyPressed(punchKey) && this->animTimer <= 0.2f) {
		this->side = this->side * -1;
		this->ChangeAnim(3, this->anims[3]->duration / 1.8 - 0.05);
		scene->audio->PlayGameSound(0,1);
	}

	//kick
	if (Input::wasKeyPressed(kickKey) && this->animTimer <= 0.2f) {
		this->side = this->side * -1;
		this->ChangeAnim(4, this->anims[4]->duration);
	}

	if (this->animTimer <= 0.0f) ChangeAnim(0, NULL);

	//sprint
	if (Input::isKeyPressed(runKey)) isRunning = true;
	this->playerVel = (isRunning == true) ? run_speed : walk_speed;


	/*
	if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) {
		seconds_elapsed *= 0.2f;
	}*/

	//rotation
	float rotSpeed = 120.0f * seconds_elapsed;

	//player movement code
	Matrix44 playerRotation;
	playerRotation.rotate(this->yaw * DEG2RAD, Vector3(0, 1, 0));

	Vector3 forward = playerRotation.rotateVector(Vector3(0, 0, -1));
	Vector3 right = playerRotation.rotateVector(Vector3(1, 0, 0));
	Vector3 up = playerRotation.rotateVector(Vector3(0, 1, 0));

	Vector3 playerVel;
	
	if (Input::isKeyPressed(upKey) && this->ctr != 5)
	{
		playerVel = playerVel + (forward * this->playerVel);
		this->ChangeAnim(1, NULL);
	}
	if (Input::isKeyPressed(downKey) && this->ctr != 5)
	{
		playerVel = playerVel - (forward * this->playerVel);
		this->ChangeAnim(1, NULL);
	}
	if (Input::isKeyPressed(rightRotKey)) this->yaw += rotSpeed;
	if (Input::isKeyPressed(leftRotKey)) this->yaw -= rotSpeed;

	
	if (Input::isKeyPressed(runKey)) ChangeAnim(2, NULL);// run anim

	//jump
	if (Input::wasKeyPressed(jumpKey) && this->pos.y <= 0.0f) {
		this->jumpLock = 0.15f;
		this->ChangeAnim(6, 0.8f);
	}

	if (this->jumpLock != 0.0f) {
		playerVel[1] += 0.1f;
	}

	if (this->pos.y > 0.0f) {
		playerVel[1] -= seconds_elapsed * 3;
	}

	Vector3 nextPos = this->pos + playerVel;

	//dash
	Vector2 dash_dir = Vector2(this->pos.x - nextPos[0], this->pos.z - nextPos[2]).normalize();
	if (dash_dir.x != 0 || dash_dir.y != 0)this->dash_direction = dash_dir;

	//std::cout << scene->player.dash_direction.x << "  " << scene->player.dash_direction.y << std::endl;

	Vector3 playerVec = this->character_mesh->model.frontVector().normalize();
	float sumX = 20.0 * this->dash_direction.x;
	float sumZ = 20.0 * this->dash_direction.y;
	if (this->ctr == 5) {
		playerVel[0] -= sumX;
		playerVel[2] -= sumZ;
	}
	if (Input::wasKeyPressed(dashKey)) {
		playerVel[0] -= sumX;
		playerVel[2] -= sumZ;
		this->ChangeAnim(5, 0.25f);
	}

	nextPos = this->pos + playerVel;

	nextPos = this->playerCollision(enemies, entities, nextPos, seconds_elapsed);

	//test attack collisions
	if (this->ctr == 3) { //kick
		this->punchCollision(enemies);
	}
	if (this->ctr == 4) { //punch
		this->kickCollision(enemies);
	}

	this->pos = nextPos;
}

void sPlayer::npcMovement(std::vector<sPlayer*> enemies, std::vector<EntityMesh*> entities, sPlayer* player, float seconds_elapsed) {


	Matrix44 npcModel = this->getModel();
	Vector3 side = npcModel.rotateVector(Vector3(1, 0, 0)).normalize();
	Vector3 forward = npcModel.rotateVector(Vector3(0, 0, -1)).normalize();

	Vector3 toTarget = player->pos - this->pos;
	float dist = toTarget.length();
	toTarget.normalize();

	float sideDot = side.dot(toTarget);
	float forwardDot = forward.dot(toTarget);

	if (forwardDot < 0.98f) {
		this->yaw += 90.0f * sign(sideDot) * seconds_elapsed;
	}

	if (dist > 2.0f) {
		this->ChangeAnim(1, NULL);
		Vector3 nextPos = this->pos + (forward * this->playerVel * seconds_elapsed);
		nextPos = this->playerCollision(enemies, entities, nextPos, seconds_elapsed);
		this->pos = nextPos;

	}
	else {
		//como se puede cambiar el side aqui?
		//como se puede parar una vez esta en dist > 2.0f?
		this->ChangeAnim(3, this->anims[3]->duration / 1.8 - 0.05);

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
	float t = fmod(time, this->idle->duration) / this->run->duration;
	/*
	scene->player.walk->assignTime(t * scene->player.walk->duration);
	scene->player.run->assignTime(t * scene->player.run->duration);
	scene->player.idle->assignTime(time * scene->player.idle->duration);
	*/
	for (int i = 0; i < this->anims.size(); i++) {
		this->anims[i]->assignTime(time);
		if (i == 3 || i == 6) this->anims[i]->assignTime(time * 1.8);
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

