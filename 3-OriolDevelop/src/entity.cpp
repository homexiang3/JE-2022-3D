#include "entity.h"
#include "camera.h"
#include "game.h"
#include "texture.h"
#include "mesh.h"

Entity::Entity() {

}
Entity::~Entity() {

}

//constructor de entity mesh para guardarlo de manera mas comoda (crear EntityMesh* y luego hacerle un new EntityMesh(valores), para simplificar solo necesita los paths)
EntityMesh::EntityMesh(int primitive, const char* meshPath, const char* texturePath, char* shaderPath1, char* shaderPath2, Vector4 color) { 
	
	this->primitive = primitive;

	if (meshPath != NULL) {
		this->mesh = Mesh::Get(meshPath);
	}
	this->texture = Texture::Get(texturePath);
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

void EntityMesh::render(Animation* animSK) {
	
	Shader* a_shader = this->shader;
	Mesh* a_mesh = this->mesh;
	Texture* tex = this->texture;
	Vector4 color = this->color;
	float tiling = this->tiling;


	//comprueba que esten cargados
	assert(a_mesh != NULL, "mesh in Entity Render was null"); //debug
	assert(tex != NULL, "tex in Entity Render was null");
	assert(a_shader != NULL, "shader in Entity Render was null");
	
	//if (!a_shader) return;

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
		//
		
		//disable the shader after finishing rendering
		mesh->disableBuffers(a_shader);
		//a_mesh->renderAnimated(primitive, &animSK->skeleton);

		//debug to see bounding box 
		
		
		
		//add animation
		if (animSK !=NULL) a_mesh->renderAnimated(GL_TRIANGLES, &animSK->skeleton);
		else a_mesh->render(primitive);
		a_mesh->renderBounding(model); //añade 2 DC adicionales a cada mesh
		a_shader->disable();
	}
}

void EntityMesh::update(float dt) {

};