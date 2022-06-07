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
	}
	this->texture = Texture::Get(texturePath.c_str());
	this->shader = Shader::Get(shaderPath1, shaderPath2);
	this->color = color;
	this->tiling = 1.0f;
	this->meshPath = meshPath;
	this->texturePath = texturePath;
}

void Entity::render() {
	//if it was entity mesh, Matrix44 model = getGlobalMatrix(), render code...
	Camera* camera = Camera::current;
	Matrix44 model = this->model;


	for (int i = 0; i < children.size(); i++)
	{
		children[i]->render(); //repeat for each child
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

void EntityMesh::render() {

	Shader* a_shader = this->shader;
	Mesh* a_mesh = this->mesh;
	Texture* tex = this->texture;
	Vector4 color = this->color;
	float tiling = this->tiling;

	//comprueba que esten cargados
	assert(a_mesh != NULL, "mesh in Entity Render was null"); //debug
	assert(tex != NULL, "tex in Entity Render was null");
	assert(a_shader != NULL, "shader in Entity Render was null");

	if (!a_shader) return;

	//get the last camera that was activated
	Camera* camera = Camera::current;
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
		}

		a_shader->setUniform("u_color", color);
		a_shader->setUniform("u_time", Game::instance->time);

		mesh->enableBuffers(a_shader);
		//parte dinamica que se tendria q pintar diferente en cada objeto
		a_shader->setUniform("u_model", model);
		a_shader->setUniform("u_tex_tiling", tiling);
		//render the mesh using the shader
		a_mesh->render(primitive);

		//disable the shader after finishing rendering
		mesh->disableBuffers(a_shader);
		a_shader->disable();

		//debug to see bounding box 
		a_mesh->renderBounding(model); //a�ade 2 DC adicionales a cada mesh
	}
}

void EntityMesh::update(float dt) {

};

//sPlayer functions 

Matrix44 sPlayer::getModel() {
	Matrix44 model;
	model.translate(pos.x, pos.y, pos.z);
	model.rotate(yaw * DEG2RAD, Vector3(0, 1, 0));
	this->character_mesh->model = model;
	return  model;
}

Vector3 sPlayer::playerCollision(std::vector<EntityMesh*> entities, Vector3 nextPos, float seconds_elapsed) {
	//TEST COLLISIONS, HABRIA QUE TENER DINAMICAS - ESTATICAS, DINAMICAS - DINAMICAS, PLAYER - COSAS ETC...
	//calculamos el centro de la esfera de colisi�n del player elevandola hasta la cintura
	Vector3 character_center = nextPos + Vector3(0, 1, 0);

	//para cada objecto de la escena...
	for (size_t i = 0; i < entities.size(); i++)
	{
		EntityMesh* currentEntity = entities[i];
		//comprobamos si colisiona el objeto con la esfera (radio 3)
		Vector3 coll;
		Vector3 collnorm;
		if (!currentEntity->mesh->testSphereCollision(currentEntity->model, character_center, 0.5f, coll, collnorm))
			continue; //si no colisiona, pasamos al siguiente objeto

		//si la esfera est� colisionando muevela a su posicion anterior alejandola del objeto
		Vector3 push_away = normalize(coll - character_center) * seconds_elapsed;
		nextPos = this->pos - push_away; //move to previous pos but a little bit further

		//cuidado con la Y, si nuestro juego es 2D la ponemos a 0
		nextPos.y = 0;

		//reflejamos el vector velocidad para que de la sensacion de que rebota en la pared
		//velocity = reflect(velocity, collnorm) * 0.95;

		return nextPos;
	}

	return nextPos;
}

void sPlayer::playerMovement(std::vector<EntityMesh*> entities,float seconds_elapsed, float rotSpeed, float playerSpeed) {


	Matrix44 playerRotation;
	playerRotation.rotate(this->yaw * DEG2RAD, Vector3(0, 1, 0));

	Vector3 forward = playerRotation.rotateVector(Vector3(0, 0, -1));
	Vector3 right = playerRotation.rotateVector(Vector3(1, 0, 0));
	Vector3 up = playerRotation.rotateVector(Vector3(0, 1, 0));

	Vector3 playerVel;

	if (Input::isKeyPressed(SDL_SCANCODE_W)) playerVel = playerVel + (forward * playerSpeed);
	if (Input::isKeyPressed(SDL_SCANCODE_S)) playerVel = playerVel - (forward * playerSpeed);
	if (Input::isKeyPressed(SDL_SCANCODE_D)) playerVel = playerVel + (right * playerSpeed);
	if (Input::isKeyPressed(SDL_SCANCODE_A)) playerVel = playerVel - (right * playerSpeed);

	//player movement code
	//jump
	if (Input::wasKeyPressed(SDL_SCANCODE_C) && this->pos.y <= 0.0f) {

		this->jumpLock = 0.3f;
	}

	if (this->jumpLock != 0.0f) {
		playerVel[1] += 0.15f;
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
	float sumX = 200.0 * this->dash_direction.x;
	float sumZ = 200.0 * this->dash_direction.y;
	if (Input::wasKeyPressed(SDL_SCANCODE_X)) {
		playerVel[0] -= sumX;
		playerVel[2] -= sumZ;
	}

	nextPos = this->pos + playerVel;

	nextPos = this->playerCollision(entities, nextPos, seconds_elapsed);

	this->pos = nextPos;
}