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


Shader* shader = NULL;
Animation* anim = NULL;
float angle = 0;
float mouse_speed = 100.0f;
FBO* fbo = NULL;
float gravityConstant = 9.8f;

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


	//create Scene
	scene = new Scene();

	//create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f,100.f, 100.f),Vector3(0.f,0.f,0.f), Vector3(0.f,1.f,0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective //far plane is last argument to get more vision!

	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}

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
   
	camera->enable();

	Matrix44 playerModel;
	playerModel.translate(scene->player.pos.x, scene->player.pos.y, scene->player.pos.z);
	playerModel.rotate(scene->player.yaw * DEG2RAD, Vector3(0, 1, 0));
	scene->playerEntity->model = playerModel;
	scene->player.character_mesh->model = playerModel;

	if (scene->cameraLocked) {
		//camera following plane
		Vector3 eye = playerModel * Vector3(0.0f, 3.0f, 3.0f);
		Vector3 center = playerModel * Vector3(0.0f, 0.0f, -5.0f);
		Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
		if (scene->firstPerson) {
			//si es first person no renderizar el personaje o solo manos/piernas
			Matrix44 firstPersonModel = playerModel;
			firstPersonModel.rotate(scene->player.pitch * DEG2RAD, Vector3(1, 0, 0));

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
	scene->skyMesh->render();//cielo
	glEnable(GL_DEPTH_TEST);

	scene->groundMesh->render(); //suelo

	scene->player.character_mesh->render();

	for (size_t i = 0; i < scene->entities.size(); i++)//entities added
	{
		EntityMesh* entity = scene->entities[i];
		entity->render();
	}

	for (size_t i = 0; i < scene->numParticles; i++)//particles
	{	
		//actualizarlas por si se mueven
		sParticle* particle = scene->particles[i];
		Matrix44 model;
		model.translate(particle->pos.x, particle->pos.y, particle->pos.z);

		EntityMesh* entity = scene->particlesMesh[i];
		entity->model = model;
		entity->render();
	}
	//bombMesh.render();
	//playerEntity.mesh->renderBounding(playerEntity.model); debug too see bounding

	//Draw the floor grid
	drawGrid();

	//render the FPS, Draw Calls, etc
	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed)
{

	//movement code
	
	float playerX = scene->player.pos[0];
	float playerY = scene->player.pos[1];
	float playerZ = scene->player.pos[2];

	bool isRunning = false;
	float walk_speed = 10.0f * elapsed_time;
	float run_speed = 30.0f * elapsed_time;

	scene->player.jumpLock = max(0.0f, scene->player.jumpLock - seconds_elapsed);

	//sprint
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) isRunning = true;
	float playerSpeed = (isRunning == true) ? run_speed : walk_speed;

	float speed = seconds_elapsed * mouse_speed; //the speed is defined by the seconds_elapsed so it goes constant

	//example
	angle += (float)seconds_elapsed * 10.0f;

	//escalar tiempo para que vayan mas lentas las particulas
	if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) {
		seconds_elapsed *= 0.2f;
	}
	//ejemplo particulas
	for (size_t i = 0; i < scene->numParticles; i++)//particles
	{
		sParticle* particle = scene->particles[i];
		//mover a su velocidad
		particle->pos = particle->pos + (particle->vel * seconds_elapsed);

		//para que caigan con gravedad
		float gravity = gravityConstant * seconds_elapsed;
		particle->vel = particle->vel - Vector3(0.0f, gravity, 0.0f);

		//friccion
		particle->vel = particle->vel - (particle->vel * 0.01f * seconds_elapsed);

		//para que reboten en el suelo
		if (particle->pos.y < particle->radius) {
			particle->pos.y = particle->radius;
			particle->vel.y = -particle->vel.y;
			particle->vel = particle->vel * 0.8f;
		}
	}

	//mouse input to rotate the cam
	if ((Input::mouse_state & SDL_BUTTON_LEFT) || mouse_locked ) //is left button pressed?
	{
		camera->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f,-1.0f,0.0f));
		camera->rotate(Input::mouse_delta.y * 0.005f, camera->getLocalVector( Vector3(-1.0f,0.0f,0.0f)));
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_TAB)) {
		scene->cameraLocked = !scene->cameraLocked;
	}

	if (scene->cameraLocked){
		//float playerSpeed = 20.0f * elapsed_time;
		float rotSpeed = 120.0f  * elapsed_time;

		if (scene->firstPerson) {
			scene->player.pitch += -Input::mouse_delta.y * 10.0f * elapsed_time;
			scene->player.yaw += -Input::mouse_delta.x * 10.0f * elapsed_time;
			Input::centerMouse();
			SDL_ShowCursor(false);
		}
		else {
			if (Input::isKeyPressed(SDL_SCANCODE_E)) scene->player.yaw += rotSpeed;
			if (Input::isKeyPressed(SDL_SCANCODE_Q)) scene->player.yaw -= rotSpeed;
		}

		Matrix44 playerRotation;
		playerRotation.rotate(scene->player.yaw * DEG2RAD, Vector3(0, 1, 0));

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
		if (Input::wasKeyPressed(SDL_SCANCODE_C)&& playerY <= 0.0f) {

			scene->player.jumpLock = 0.3f;
		}

		if (scene->player.jumpLock != 0.0f) {
			playerVel[1] += 0.15f;
		}

		if (playerY > 0.0f) {
			playerVel[1] -= seconds_elapsed * 3;
		}

		Vector3 nextPos = scene->player.pos + playerVel;
		
		//dash
		Vector2 dash_dir = Vector2(playerX - nextPos[0], playerZ - nextPos[2]).normalize();
		if (dash_dir.x != 0 || dash_dir.y != 0)scene->player.dash_direction = dash_dir;
		
		std::cout << scene->player.dash_direction.x <<"  "<< scene->player.dash_direction.y << std::endl;

		Vector3 playerVec = scene->playerEntity->model.frontVector().normalize();
		float sumX = 200.0 * scene->player.dash_direction.x;
		float sumZ = 200.0 * scene->player.dash_direction.y;
		if (Input::wasKeyPressed(SDL_SCANCODE_X)) {
			playerVel[0] -= sumX;
			playerVel[2] -= sumZ;
		}

		nextPos = scene->player.pos + playerVel;

		//TEST COLLISIONS, HABRIA QUE TENER DINAMICAS-ESTATICAS, DINAMICAS-DINAMICAS, PLAYER-COSAS ETC...
		//calculamos el centro de la esfera de colisión del player elevandola hasta la cintura
		Vector3 character_center = nextPos + Vector3(0, 1, 0);

		//para cada objecto de la escena...
		for (size_t i = 0; i < scene->entities.size(); i++)
		{
			EntityMesh* currentEntity = scene->entities[i];
			//comprobamos si colisiona el objeto con la esfera (radio 3)
			Vector3 coll;
			Vector3 collnorm;
			if (!currentEntity->mesh->testSphereCollision(currentEntity->model, character_center, 0.5f, coll, collnorm))
				continue; //si no colisiona, pasamos al siguiente objeto

			//si la esfera está colisionando muevela a su posicion anterior alejandola del objeto
			Vector3 push_away = normalize(coll - character_center) * elapsed_time;
			nextPos = scene->player.pos - push_away; //move to previous pos but a little bit further

			//cuidado con la Y, si nuestro juego es 2D la ponemos a 0
			nextPos.y = 0;

			//reflejamos el vector velocidad para que de la sensacion de que rebota en la pared
			//velocity = reflect(velocity, collnorm) * 0.95;
		}

		
		scene->player.pos = nextPos;
														  
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
		scene->bombAttached = false;
	}
	//bomba attached
	if (scene->bombAttached) {
		scene->bombMesh->model = scene->bombOffset * scene->playerEntity->model;
	}
	else {
		scene->bombMesh->model.translateGlobal(0.0f, -9.8f * elapsed_time * 6, 0.0f);
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
		case SDLK_2: scene->addEntityOnFront(); break; //debug to add sphere on front
		case SDLK_3: scene->testCollisionOnFront(); break; //debug to see collision point on front (col in console)
		case SDLK_4: scene->rotateSelected(10.0f); break; //rotate selected item with 3
		case SDLK_5: scene->rotateSelected(-10.0f); break;
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

