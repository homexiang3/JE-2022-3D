#include "entity.h"
#include "camera.h"
#include "game.h"

Entity::Entity() {

}
Entity::~Entity() {

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

Matrix44 Entity::getGlobalMatrix() { //recursive get matrix of parents
	if (parent)
		return model * parent->getGlobalMatrix();
	return model;
}

void EntityMesh::render() {

	Shader* a_shader = this->shader;
	Mesh* a_mesh = this->mesh;
	Texture* tex = this->texture;
	Vector4 color = this->color;

	assert(a_mesh != NULL, "mesh in Entity Render was null"); //debug
	assert(tex != NULL, "tex in Entity Render was null");
	assert(a_shader != NULL, "shader in Entity Render was null");

	if (!a_shader) return;

	//get the last camera that was activated
	Camera* camera = Camera::current;
	Matrix44 model = this->model;

	//enable shader and pass uniforms
	a_shader->enable();
	a_shader->setUniform("u_model", model);
	a_shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	a_shader->setUniform("u_texture", tex, 0);

	a_shader->setUniform("u_color", color);
	a_shader->setUniform("u_time", Game::instance->time);

	//render the mesh using the shader
	a_mesh->render(GL_TRIANGLES);

	//disable the shader after finishing rendering
	a_shader->disable();
}

void EntityMesh::update(float dt) {

};