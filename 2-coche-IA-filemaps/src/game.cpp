#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"

#include <cmath>

//some globals
Shader* shader = NULL;
Animation* anim = NULL;
float angle = 0;
float mouse_speed = 100.0f;
FBO* fbo = NULL;
bool cameraLocked = true;

Game* Game::instance = NULL;

float lod_distance = 200.0f;
float no_render_distance = 1000.0f;

class Entity {
public:
	Matrix44 model;
	Mesh* mesh;
	Texture* texture;

};
struct sParticle {
	Vector3 pos;
	Vector3 vel;
	float yaw;
	float angular_vel;
	
	Matrix44 getModel() {
		Matrix44 model;
		model.translate(pos.x, pos.y, pos.z);
		model.rotate(yaw * DEG2RAD, Vector3(0, 1, 0));
		return model;
	}
};

sParticle car;

std::vector<Entity*> entities;
Entity* selectedEntity;

Vector3 Lerp(Vector3 a, Vector3 b, float t){
	
	Vector3 ab = b - a;
	return a + (ab * t);
}
Vector3 LerpClamped(Vector3 a, Vector3 b, float t) {
	t = clamp(t, 0.0f, 1.0f);
	Vector3 ab = b - a;
	return a + (ab * t);
}

Game::Game(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;
	

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;

	//OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	//create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f,100.f, 100.f),Vector3(0.f,0.f,0.f), Vector3(0.f,1.f,0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective

	// example of shader loading using the shaders manager
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	//fbo test
	fbo = new FBO();
	fbo->create(window_width, window_height);  //si modificas los valores sale pixelado

	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}
void RenderMesh(int primitive, Matrix44& model, Mesh* a_mesh, Texture* tex, Shader* a_shader, Camera* cam, float tiling = 1.0f) {

	if (!a_shader) return;
	a_shader->enable();
	
		//enable shader
	a_shader->enable();

		//upload uniforms
	a_shader->setUniform("u_color", Vector4(1,1,1,1));
	a_shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
	if (tex != NULL) {
		a_shader->setUniform("u_texture", tex, 0);
	}
	a_shader->setUniform("u_model", model);
	a_shader->setUniform("u_time", time);
	a_shader->setUniform("u_tex_tiling", tiling);

	//do the draw call
	a_mesh->render(primitive);

	//disable shader
	a_shader->disable();
	
}

void AddEntityInFront(Camera* cam, const char* meshName, const char* texName) {
	Vector2 mouse = Input::mouse_position;
	Game* g = Game::instance;
	Vector3 dir = cam->getRayDirection(mouse.x, mouse.y, g->window_width, g->window_height);
	Vector3 rayOrigin = cam->eye;

	Vector3 spawnPos = RayPlaneCollision(Vector3(), Vector3(0, 1, 0), rayOrigin, dir);
	Matrix44 model;
	model.translate(spawnPos.x, spawnPos.y, spawnPos.z);
	model.scale(1.0f, 1.0f, 1.0f); //tocar para escalar

	Entity* entity = new Entity();
	entity->model = model;
	entity->mesh = Mesh::Get(meshName);
	entity->texture = Texture::Get(texName);
	entities.push_back(entity);
}

void RayPick(Camera* cam) {
	Vector2 mouse = Input::mouse_position;
	Game* g = Game::instance;
	Vector3 dir = cam->getRayDirection(mouse.x, mouse.y, g->window_width, g->window_height);
	Vector3 rayOrigin = cam->eye;

	for (size_t i = 0; i < entities.size(); i++)
	{
		Entity* entity = entities[i];
		Vector3 pos;
		Vector3 normal;
		if (entity->mesh->testRayCollision(entity->model, rayOrigin, dir, pos, normal)) {
			selectedEntity = entity;
			break;
		}
	}
}

void RotateSelected(float angleDegrees) {
	if (selectedEntity == NULL) {
		return;
	}
	selectedEntity->model.rotate(angleDegrees * DEG2RAD, Vector3(0, 1, 0));
}

//what to do when the image has to be draw
void Game::render(void)
{

	fbo->bind();

	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set flags
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	//set the camera as default
	camera->enable();
   
	//camera locked
	Matrix44 carModel = car.getModel();
	if (cameraLocked) {
		Vector3 desiredEye = carModel * Vector3(0, 5, 7); 
		Vector3 eye = Lerp(camera->eye, desiredEye, 5.f* elapsed_time);
		Vector3 center = carModel * Vector3(0, 0, -5);
		Vector3 up = Vector3(0, 1, 0);

		camera->lookAt(eye, center, up);
	}

	//sky
	Texture* skyTex = Texture::Get("data/cielo.tga");
	Mesh* skyMesh = Mesh::Get("data/cielo.ASE");
	Matrix44 skyModel; 
	skyModel.translate(camera->eye.x, camera->eye.y - 40.0f, camera->eye.z);
	glDisable(GL_DEPTH_TEST);
	RenderMesh(GL_TRIANGLES, skyModel, skyMesh, skyTex, shader, camera);
	glEnable(GL_DEPTH_TEST);
	//car
	Texture* carTex = Texture::Get("data/color-atlas-new.png");
	Mesh* carMesh = Mesh::Get("data/car.obj");
	RenderMesh(GL_TRIANGLES, carModel, carMesh, carTex, shader, camera);
	//ground
	Texture* groundTex = Texture::Get("data/grass.tga");
	Mesh* groundMesh = new Mesh();
	groundMesh->createPlane(1000);
	RenderMesh(GL_TRIANGLES, Matrix44(), groundMesh, groundTex, shader, camera, 200.0f);
	//entities
	for (size_t i = 0; i < entities.size(); i++)
	{
		Entity* entity = entities[i];
		RenderMesh(GL_TRIANGLES, entity->model, entity->mesh, entity->texture, shader, camera);

	}

	fbo->unbind();
	fbo->color_textures[0]->toViewport();
	//Draw the floor grid
	drawGrid();

	//render the FPS, Draw Calls, etc
	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed)
{
	float speed = seconds_elapsed * mouse_speed; //the speed is defined by the seconds_elapsed so it goes constant

	//slower actions
	if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) {
		seconds_elapsed *= 0.2f;
	}
	//example
	angle += (float)seconds_elapsed * 10.0f;

	//mouse input to rotate the cam
	if ((Input::mouse_state & SDL_BUTTON_LEFT) || mouse_locked ) //is left button pressed?
	{
		camera->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f,-1.0f,0.0f));
		camera->rotate(Input::mouse_delta.y * 0.005f, camera->getLocalVector( Vector3(-1.0f,0.0f,0.0f)));
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_TAB)) {
		cameraLocked = !cameraLocked;
	}
	//movimientos coche
	if (cameraLocked) {
		//se podria meter en algun sitio
		Matrix44 model = car.getModel();
		float acceleration = 30.0f * seconds_elapsed;
		float drag = 2.0f * seconds_elapsed;
		float side_drag = 1.0f * seconds_elapsed;
		float max_vel = 60.0f;

		float angular_acceleration = 300.0f * seconds_elapsed;
		float angular_drag = 10.0f * seconds_elapsed;
		float max_angular_drag = 200.0f;

		if (Input::isKeyPressed(SDL_SCANCODE_D)) { car.angular_vel += angular_acceleration; }
		else if (Input::isKeyPressed(SDL_SCANCODE_A)){car.angular_vel -= angular_acceleration; }
		else {
			car.angular_vel = car.angular_vel - (car.angular_vel * angular_drag);
		}

		car.angular_vel = clamp(car.angular_vel, -max_angular_drag, max_angular_drag);
		car.yaw = car.yaw + (car.angular_vel * seconds_elapsed);

		Vector3 carForward = model.rotateVector(Vector3(0, 0, -1));

		if (Input::isKeyPressed(SDL_SCANCODE_W)) {
			car.vel = car.vel + (carForward * acceleration);
		}
		else if (Input::isKeyPressed(SDL_SCANCODE_S)) {
			car.vel = car.vel - (carForward * acceleration);
		}
		else {
			car.vel = car.vel - (car.vel * drag);
		}

		Matrix44 inv_car_model = model;
		inv_car_model.inverse();
		Vector3 local_vel = inv_car_model.rotateVector(car.vel);
		local_vel.x = local_vel.x - (local_vel.x * side_drag);
		car.vel = model.rotateVector(local_vel);

		if (car.vel.length() > max_vel)
		{
			car.vel.normalize();
			car.vel = car.vel * max_vel;
		}

		car.pos = car.pos + (car.vel * seconds_elapsed);
		
	}
	else {
		SDL_ShowCursor(true);
		//async input to move the camera around
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
		if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_E))  camera->move(Vector3(0.0f, -1.0f, 0.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_Q))  camera->move(Vector3(0.0f, 1.0f, 0.0f) * speed);
	}
	//to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();
}

//Keyboard event handler (sync input)
void Game::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
		case SDLK_F1: Shader::ReloadAll(); break; 
		case SDLK_1: AddEntityInFront(camera, "data/sphere.obj", "data/texture.tga"); break;
		case SDLK_2: RayPick(camera); break;
		case SDLK_3: RotateSelected(10.0f); break;
		case SDLK_4: RotateSelected(-10.0f); break;
	}
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Game::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Game::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
	}
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
	mouse_speed *= event.y > 0 ? 1.1 : 0.9;
}

void Game::onResize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

