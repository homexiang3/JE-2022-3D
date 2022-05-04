#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"
#include "entity.h"

#include <cmath>

//some globals
EntityMesh islandMesh;
EntityMesh planeMesh;
Matrix44 bombOffset;
EntityMesh bombMesh;
bool cameraLocked = true; //util para debug
bool bombAttached = true;

Shader* shader = NULL;
Animation* anim = NULL;
float angle = 0;
float mouse_speed = 100.0f;
FBO* fbo = NULL;

Game* Game::instance = NULL;

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

	//bomb offset
	bombOffset.setTranslation(0.0f, -2.0f, 0.0f);
	//create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f,100.f, 100.f),Vector3(0.f,0.f,0.f), Vector3(0.f,1.f,0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective //far plane is last argument to get more vision!

	//load colors
	islandMesh.color = Vector4(1, 1, 1, 1);
	planeMesh.color = Vector4(1, 1, 1, 1);
	bombMesh.color = Vector4(1, 1, 1, 1);
	//load one texture without using the Texture Manager (Texture::Get would use the manager)
	islandMesh.texture = Texture::Get("data/island_color.tga");
 	planeMesh.texture = Texture::Get("data/spitfire_color_spec.tga");
	bombMesh.texture = Texture::Get("data/torpedo.tga");
	// example of loading Mesh from Mesh Manager
	islandMesh.mesh = Mesh::Get("data/island.ASE");
	planeMesh.mesh = Mesh::Get("data/spitfire.ASE");
	bombMesh.mesh = Mesh::Get("data/torpedo.ASE");
	// example of shader loading using the shaders manager
	islandMesh.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
	planeMesh.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
	bombMesh.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}
/*test old example
void renderIslands() {
	if (shader)
	{
		Camera* camera = Camera::current;
		float time = Game::instance->time;
		//enable shader
		shader->enable();

		//upload uniforms
		shader->setUniform("u_color", Vector4(1, 1, 1, 1));
		shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
		shader->setUniform("u_texture", texture, 0);
		shader->setUniform("u_time", time);

		//instancias de objetos
		Matrix44 m;

		


		//disable shader
		shader->disable();
	}
}
*/
//what to do when the image has to be draw
void Game::render(void)
{
	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	//set flags
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE); //culling to optimize (no carga caras que no se ven)
   
	if (cameraLocked) {
		//camera following plane
		Vector3 eye = planeMesh.model * Vector3(0.0f, 10.0f, 15.0f);
		Vector3 center = planeMesh.model * Vector3(0.0f, 0.0f, -20.0f);
		Vector3 up = planeMesh.model.rotateVector(Vector3(0.0f, 1.0f, 0.0f));
		//set the camera as default
		camera->enable();
		camera->lookAt(eye, center, up);
	}
	
	//Render
	//islandMesh.render();
	float padding = 20.0f;
	float no_render_distance = 1000.0f;
	for (size_t i = 0; i <100; i++)
	{
		for (size_t j = 0; j < 100; j++)
		{
			Vector3 planePos = planeMesh.model.getTranslation();
			Vector3 camPos = camera->eye;
			float dist = planePos.distance(camPos);

			planeMesh.model.setTranslation(i*padding, 0.0f, j*padding);
			if (dist > no_render_distance) { //don't render if is out of distance
				continue;
			}

			BoundingBox worldAABB = transformBoundingBox(planeMesh.model, planeMesh.mesh->box);//don't render if bounding box is out of camera add to render..
			if (!camera->testBoxInFrustum(worldAABB.center, worldAABB.halfsize)) {
				continue;
			}

			planeMesh.render();
		}
	}
	//bombMesh.render();
	//islandMesh.mesh->renderBounding(islandMesh.model); debug too see bounding
	//renderIslands(); old function

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

	if (cameraLocked){
		float planeSpeed = 30.0f * elapsed_time;
		float rotSpeed = 90.0f * DEG2RAD * elapsed_time;

		if (Input::isKeyPressed(SDL_SCANCODE_W)) planeMesh.model.translate(0.0f,0.0f,-planeSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_S)) planeMesh.model.translate(0.0f, 0.0f, planeSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_A)) planeMesh.model.rotate(-rotSpeed, Vector3(0.0f, 1.0f, 0.0f));
		if (Input::isKeyPressed(SDL_SCANCODE_D)) planeMesh.model.rotate(rotSpeed, Vector3(0.0f, 1.0f, 0.0f));
		if (Input::isKeyPressed(SDL_SCANCODE_E)) planeMesh.model.rotate(-rotSpeed, Vector3(0.0f, 0.0f, 1.0f));
		if (Input::isKeyPressed(SDL_SCANCODE_Q)) planeMesh.model.rotate(rotSpeed, Vector3(0.0f, 0.0f, 1.0f));

	}
	else {
		//async input to move the camera around
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
		if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_E)) camera->move(Vector3(0.0f, -1.0f, 0.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_Q)) camera->move(Vector3(0.0f, 1.0f, 0.0f) * speed);
	}
	//Bomba test
	if (Input::wasKeyPressed(SDL_SCANCODE_F)) {
		bombAttached = false;
	}
	//bomba attached
	if (bombAttached) {
		bombMesh.model = bombOffset * planeMesh.model;
	}
	else {
		bombMesh.model.translateGlobal(0.0f, -9.8f * elapsed_time * 6, 0.0f);
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

