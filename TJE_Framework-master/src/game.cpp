#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"
#include "entity.h"
#include "audio.h"
//#include "stage.h"

#include <bass.h>
#include <cmath>

//some globals
Shader* shader = NULL;

Mesh* skyMesh = NULL;
Texture* skyTex = NULL;
Matrix44 skyModel;

Mesh* houseMesh = NULL;
Texture* houseTex = NULL;
Matrix44 houseModel;

Mesh* femaleMesh = NULL;
Texture* femaleTex = NULL;
Animation* walkingf;
Matrix44 femaleModel;

Mesh* maleMesh = NULL;
Texture* maleTex = NULL;
Matrix44 maleModel;

Animation* anim = NULL;
float angle = 0;
float mouse_speed = 100.0f;
FBO* fbo = NULL;

Game* Game::instance = NULL;

struct sParticle {
	Vector3 pos;
	Vector3 vel;
};

sParticle female;

const int houses_width = 10;
const int houses_height = 10;
float padding = 70.0f; // Distancia entre las houses
float lodDist = 10.0f; // Para los LODs
float no_render_dist = 1000.0f; // Para el frustum
bool cameraLocked = true;

std::vector<Entity*> entities;

//std::vector<Stage*> stages; // Vector stages
//STAGE_ID currentStage = STAGE_ID::INTRO; //Índice que nos dice en que stage estamos, inicializamos con la INTRO

//void InitStages() {
//	stages.reserve(4); //Reserva 7 elementos (hace un malloc de 7 porque tenemos 7 stages)
//	stages.push_back(new IntroStage()); //Para añadir elementos a un std vector = pushback
//	stages.push_back(new InfoStage());
//	stages.push_back(new TutorialStage());
//	stages.push_back(new Level1Stage());
//}

HSAMPLE loadAudio(const char* fileName) {
	//El handler para un sample
	HSAMPLE hSample;
	//use BASS_SAMPLE_LOOP in the last param to have a looped sound
	hSample = BASS_SampleLoad(false, fileName, 0, 0, 3, BASS_SAMPLE_LOOP);
	if (hSample == 0)
	{
		std::cout << "ERROR loading " << fileName << std::endl;
	}
	std::cout << " + AUDIO load " << fileName << std::endl;
	return hSample;
}

void PlayAudio(const char* fileName) {
	//Cargamos un sample del disco duro (memoria, filename, offset, length, max, flags)
	HSAMPLE hSample = loadAudio(fileName);

	//El handler para un canal
	HCHANNEL hSampleChannel;

	//Creamos un canal para el sample
	hSampleChannel = BASS_SampleGetChannel(hSample, false);

	//Lanzamos un sample
	BASS_ChannelPlay(hSampleChannel, true);
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
	glEnable(GL_CULL_FACE); //render both sides of every triangle
	glEnable(GL_DEPTH_TEST); //check the occlusions using the Z buffer

	//Create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f, 100.f, 100.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f, window_width / (float)window_height, 0.1f, 1000000.f); //set the projection, we want to be perspective
	// Si añadimos unos ceros más en el último param -> Vemos más lejos

	skyMesh = Mesh::Get("data/cielo.ASE");
	skyTex = Texture::Get("data/cielo.tga");

	houseMesh = Mesh::Get("data/house1.ASE");
	houseTex = Texture::Get("data/houses_and_windows.tga");

	femaleMesh = Mesh::Get("data/female.mesh");
	femaleTex = Texture::Get("data/female.tga");

	//femaleMesh = Mesh::Get("data/fantasyf.mesh");
	//femaleTex = Texture::Get("data/polyTexture.png");
	walkingf = Animation::Get("data/walking_female.skanim");

	maleMesh = Mesh::Get("data/male.mesh");
	maleTex = Texture::Get("data/male.tga");

	// example of shader loading using the shaders manager
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	// Init bass
	if (BASS_Init(-1, 44100, 0, 0, NULL) == false) //-1 significa usar el por defecto del sistema operativo
	{
		std::cout << "ERROR initializing audio" << std::endl;
	}
	
	//Audio::PlayAudio("data/audio/mistery.wav");
	PlayAudio("data/audio/mistery.wav");

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
	glDisable(GL_CULL_FACE);


	// ----------------------------- ARNAU --------------------------------------------------------------
	Vector3 eye = maleModel * Vector3(0.f, 15.f, 1.f);
	Vector3 center = maleModel * Vector3(0.f, 15.f, 10.f);
	Vector3 up = maleModel.rotateVector(Vector3(0.f, 1.f, 0.f));
	// ----------------------------- ARNAU --------------------------------------------------------------

	//set the camera as default
	camera->enable();

	// ----------------------------- ARNAU --------------------------------------------------------------
	if (cameraLocked) //Entramos en modo 1a Persona.
		camera->lookAt(eye, center, up);
	// ----------------------------- ARNAU --------------------------------------------------------------

	RenderMesh(skyModel, skyMesh, skyTex, shader, camera);
	skyModel.setScale(100, 100, 100);

	RenderObjects(houseMesh, houseTex, shader, houses_width, houses_height, padding, no_render_dist);

	// Anim
	//RenderMeshWithAnim(femaleModel, femaleMesh, femaleTex, walkingf, shader, camera, time);

	// ----------------------------- ARNAU --------------------------------------------------------------
	RenderMesh(maleModel, maleMesh, maleTex, shader, camera);
	// ----------------------------- ARNAU --------------------------------------------------------------

	for (size_t i = 0; i < entities.size(); i++) { // Para el AddEntityInFront
		Entity* entity = entities[i];
		RenderMesh(entity->model, entity->mesh, entity->texture, shader, camera);
	}

	//GetStage(currentStage, stages)->Render();

	//if (GetStage(currentStage, stages) == GetStage(STAGE_ID::INTRO, stages)) {

	//}
	//if (GetStage(currentStage, stages) == GetStage(STAGE_ID::INFO, stages)) {

	//}
	//if (GetStage(currentStage, stages) == GetStage(STAGE_ID::TUTORIAL, stages)) {

	//}
	//if (GetStage(currentStage, stages) == GetStage(STAGE_ID::LEVEL1, stages)) {

	//	// ----------------------------- ARNAU --------------------------------------------------------------
	//	Vector3 eye = maleModel * Vector3(0.f, 15.f, 1.f);
	//	Vector3 center = maleModel * Vector3(0.f, 15.f, 10.f);
	//	Vector3 up = maleModel.rotateVector(Vector3(0.f, 1.f, 0.f));
	//	// ----------------------------- ARNAU --------------------------------------------------------------


	//	//set the camera as default
	//	camera->enable();

	//	// ----------------------------- ARNAU --------------------------------------------------------------
	//	if (cameraLocked) //Entramos en modo 1a Persona.
	//		camera->lookAt(eye, center, up);
	//	// ----------------------------- ARNAU --------------------------------------------------------------

	//	RenderMesh(skyModel, skyMesh, skyTex, shader, camera);
	//	skyModel.setScale(100, 100, 100);

	//	RenderHouses(houseMesh);

	//	// ----------------------------- ARNAU --------------------------------------------------------------
	//	RenderMesh(maleModel, maleMesh, maleTex, shader, camera);
	//	// ----------------------------- ARNAU --------------------------------------------------------------

	//	for (size_t i = 0; i < entities.size(); i++) { // Para el AddEntityInFront
	//		Entity* entity = entities[i];
	//		RenderMesh(entity->model, entity->mesh, entity->texture, shader, camera);
	//	}
	//}

	//Draw the floor grid
	drawGrid();

	//render the FPS, Draw Calls, etc
	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

//void NextStage() { // Para pasar de stage
//	int nextStageIndex = (((int)currentStage) + 1) % stages.size();
//	SetStage(&currentStage, (STAGE_ID)nextStageIndex);
//}


void Game::update(double seconds_elapsed)
{
	float speed = seconds_elapsed * mouse_speed; //the speed is defined by the seconds_elapsed so it goes constant

	//example
	angle += (float)seconds_elapsed * 10.0f;

	//mouse input to rotate the cam
	if ((Input::mouse_state & SDL_BUTTON_LEFT)) //is left button pressed?
	{
		camera->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f, -1.0f, 0.0f));
		camera->rotate(Input::mouse_delta.y * 0.005f, camera->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));
	}

	//async input to move the camera around
	/*if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
	if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);*/
	// Esto reemplaza a lo de abajo

	// ----------------------------- ARNAU --------------------------------------------------------------

	if (Input::wasKeyPressed(SDL_SCANCODE_TAB)) { //CAMBIAR MODO DE LA CAMARA
		cameraLocked = !cameraLocked;
	}

	if (cameraLocked) //SI ESTAMOS EN 1a PERSONA
	{
		mouse_locked = true;
		float rotSpeed = 60.f * DEG2RAD * elapsed_time;
		maleModel.rotate(Input::mouse_delta.x * rotSpeed, Vector3(0.0f, -1.0f, 0.0f)); //Rotamos el modelo del jugador y con él, la camara.
		maleModel.rotate(Input::mouse_delta.y * rotSpeed, Vector3(1.0f, 0.0f, 0.0f));
		float playerSpeed = 50.f * elapsed_time;
		if (Input::isKeyPressed(SDL_SCANCODE_W)) maleModel.translate(0.f, 0.f, playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_S)) maleModel.translate(0.f, 0.f, -playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_A)) maleModel.translate(playerSpeed, 0.f, 0.f);
		if (Input::isKeyPressed(SDL_SCANCODE_D)) maleModel.translate(-playerSpeed, 0.f, 0.f);
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) playerSpeed *= 10; //move faster with left shift
	}
	else { //CAMARA LIBRE
		mouse_locked = false;
		//async input to move the camera around
		//float rotSpeed = 60.f * DEG2RAD * elapsed_time;
		//femaleModel.rotate(Input::mouse_delta.x * rotSpeed, Vector3(0.0f, -1.0f, 0.0f)); //Rotamos el modelo del jugador y con él, la camara.
		////maleModel.rotate(Input::mouse_delta.y * rotSpeed, Vector3(-1.0f, 0.0f, 0.0f));
		//float playerSpeed = 50.f * elapsed_time;
		//if (Input::isKeyPressed(SDL_SCANCODE_W)) femaleModel.translate(0.f, 0.f, playerSpeed);
		//if (Input::isKeyPressed(SDL_SCANCODE_S)) femaleModel.translate(0.f, 0.f, -playerSpeed);
		//if (Input::isKeyPressed(SDL_SCANCODE_A)) femaleModel.translate(playerSpeed, 0.f, 0.f);
		//if (Input::isKeyPressed(SDL_SCANCODE_D)) femaleModel.translate(-playerSpeed, 0.f, 0.f);
		//if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) playerSpeed *= 10; //move faster with left shift
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
		if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);

	}

	//GetStage(currentStage, stages)->Update(seconds_elapsed); // Actualiza la stage que toca

	//if (GetStage(currentStage, stages) == GetStage(STAGE_ID::LEVEL1, stages)) {
	//	if (Input::wasKeyPressed(SDL_SCANCODE_TAB)) { //CAMBIAR MODO DE LA CAMARA
	//		cameraLocked = !cameraLocked;
	//	}

	//	if (cameraLocked) //SI ESTAMOS EN 1a PERSONA
	//	{
	//		mouse_locked = true;
	//		float rotSpeed = 60.f * DEG2RAD * elapsed_time;
	//		maleModel.rotate(Input::mouse_delta.x * rotSpeed, Vector3(0.0f, -1.0f, 0.0f)); //Rotamos el modelo del jugador y con él, la camara.
	//		//maleModel.rotate(Input::mouse_delta.y * rotSpeed, Vector3(-1.0f, 0.0f, 0.0f));
	//		float playerSpeed = 50.f * elapsed_time;
	//		if (Input::isKeyPressed(SDL_SCANCODE_W)) maleModel.translate(0.f, 0.f, playerSpeed);
	//		if (Input::isKeyPressed(SDL_SCANCODE_S)) maleModel.translate(0.f, 0.f, -playerSpeed);
	//		if (Input::isKeyPressed(SDL_SCANCODE_A)) maleModel.translate(playerSpeed, 0.f, 0.f);
	//		if (Input::isKeyPressed(SDL_SCANCODE_D)) maleModel.translate(-playerSpeed, 0.f, 0.f);
	//		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) playerSpeed *= 10; //move faster with left shift
	//	}
	//	else { //CAMARA LIBRE
	//		mouse_locked = false;
	//		//async input to move the camera around
	//		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
	//		if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
	//		if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
	//		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
	//		if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);

	//	}
	//}

	////Read the keyboard state, to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	//if (Input::wasKeyPressed(SDL_SCANCODE_S)) { //S para pasar de Info a Tutorial a Level1
	//	if (currentStage == 0 || currentStage == 1 || currentStage == 2 || currentStage == 3) {
	//		NextStage();
	//	}
	//}

	//to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();
}

//Keyboard event handler (sync input)
void Game::onKeyDown(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
	case SDLK_F1: Shader::ReloadAll(); break;
	case SDLK_1: AddEntityInFront(camera, maleMesh, maleTex, entities);  break;
	case SDLK_2: AddEntityInFront(camera, femaleMesh, femaleTex, entities);  break;
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

void Game::onMouseButtonDown(SDL_MouseButtonEvent event)
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
	glViewport(0, 0, width, height);
	camera->aspect = width / (float)height;
	window_width = width;
	window_height = height;
}

