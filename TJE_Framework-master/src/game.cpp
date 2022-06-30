#include "animation.h"
#include "audio.h"
#include "entity.h"
#include "fbo.h"
#include "game.h"
#include "input.h"
#include "utils.h"
#include "mesh.h"
#include "pathfinders.h"
#include "player.h"
#include "scene.h"
#include "shader.h"
#include "stage.h"
#include "texture.h"

#include <bass.h>
#include <cmath>

//some globals
Mesh* femaleMesh = NULL;
Texture* femaleTex = NULL;
Matrix44 femaleModel;
Animation* walkingf;

Mesh* maleMesh = NULL;
Texture* maleTex = NULL;
Matrix44 maleModel;

// IMPORTANTES
Mesh* skyMesh = NULL;
Texture* skyTex = NULL;
Matrix44 skyModel;

Mesh* floorMesh;
Texture* floorTex;
Matrix44 floorModel;

Mesh* campingMesh = NULL;
Texture* campingTex = NULL;
Matrix44 campingModel;

Mesh* houseMesh = NULL;
Texture* houseTex = NULL;
Matrix44 houseModel;

Mesh* cityMesh = NULL;
Texture* cityTex = NULL;
Matrix44 cityModel;

Player detective;
Matrix44 viewmodel;



Animation* anim = NULL;
float angle = 0;
float mouse_speed = 50.0f;
FBO* fbo = NULL;

Game* Game::instance = NULL;

// Audio
//HSAMPLE hSample1;
//HSAMPLE hSample2;
//HSAMPLE hSample3;
//Audio* audio;

Scene* scene = new Scene();

const int houses_width = 10;
const int houses_height = 10;
float padding = 10.0f; // Distancia entre las houses
float lodDist = 10.0f; // Para los LODs
float no_render_dist = 1000.0f; // Para el frustum
bool cameraLocked = false;
const bool firstP = true;
int cameracontroller = 0;
int level = 1;

Entity* selectedEntity = NULL;
std::vector<LightEntity*> lights;

// ----------------------------- PROFE: YO LO PONDRIA EN EL INPUT --------------------------------------------------------------
bool wasLeftMousePressed = false;
// ----------------------------- PROFE: YO LO PONDRIA EN EL INPUT --------------------------------------------------------------

// ----------------------------- IA -> PATHFINDERS --------------------------------------------------------------
int startx, starty, targetx, targety;
uint8* grid;
int output[100];
int W, H;
float tileSizeX, tileSizeY;
// ----------------------------- IA -> PATHFINDERS --------------------------------------------------------------

// ----------------------------- STAGES --------------------------------------------------------------
std::vector<Stage*> stages; // Vector stages
STAGE_ID currentStage = STAGE_ID::INTRO; //Índice que nos dice en que stage estamos, inicializamos con la INTRO
ePlayerState currentAnim = ePlayerState::IDLE; // Índice que nos dice qué animación tiene el detective, inicializamos con IDLE


void InitStages() {
    stages.reserve(7); //Reserva 7 elementos (hace un malloc de 7 porque tenemos 7 stages)
    stages.push_back(new IntroStage()); //Para añadir elementos a un std vector = pushback
    stages.push_back(new InfoStage());
    stages.push_back(new TutorialStage());
    stages.push_back(new Level1Stage());
	stages.push_back(new Level2Stage());
	stages.push_back(new Level3Stage());
	stages.push_back(new EndStage());
}
// ----------------------------- STAGES --------------------------------------------------------------

std::pair<Mesh*, Texture*> loadObject(const char* obj_filename, const char* tex_filename)
{
	Mesh* mesh = Mesh::Get(obj_filename);
	Texture* tex = Texture::Get(tex_filename);

	std::pair<Mesh*, Texture*> sol;

	sol.first = mesh;
	sol.second = tex;

	return sol;
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

	// OpenGL flags
	glEnable(GL_CULL_FACE); //render both sides of every triangle
	glEnable(GL_DEPTH_TEST); //check the occlusions using the Z buffer

	// Create our camera
	camera = new Camera();
	
	// Meshes principales
	skyMesh = Mesh::Get("data/assets/cielo.ASE");
	skyTex = Texture::Get("data/textures/cielo.tga");

	/*floorMesh = new Mesh();
	floorMesh->createPlane(1000);*/
	floorMesh = Mesh::Get("data/assets/sand2.obj");
	floorTex = Texture::Get("data/textures/sand.tga");

	// Levels
	scene->loadMap("data/scenes/TrainStation/train_station.scene");
	scene->loadMap("data/scenes/Pueblo/pueblo.scene");
	scene->loadMap("data/scenes/House/house.scene");

	std::cout << scene->entities[2] << std::endl;

	// GUI
	pause = Texture::Get("data/gui/pause.png");
	gui = Texture::Get("data/gui/gui.png");
	menu_inicial = Texture::Get("data/gui/menu_inicial.png");
	menu_game = Texture::Get("data/gui/menu_game.png");

	// example of shader loading using the shaders manager
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
	gui_shader = Shader::Get("data/shaders/basic.vs", "data/shaders/gui.fs");
	a_shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");



	InitStages();

	//// Init bass
	//if (BASS_Init(-1, 44100, 0, 0, NULL) == false) //-1 significa usar el por defecto del sistema operativo
	//{
	//	std::cout << "ERROR initializing audio" << std::endl;
	//}
	//
	//hSample1 = audio->loadAudio("data/audio/mistery.wav", BASS_SAMPLE_LOOP);
	//hSample2 = audio->loadAudio("data/audio/pasos.wav", BASS_SAMPLE_LOOP);
	//hSample3 = audio->loadAudio("data/audio/button.wav", 0);

	//scene = new Scene();
	//camera = scene->player->InitPlayerCamera();

	putCamera(viewmodel, camera, cameraLocked, window_width, window_height, cameracontroller);

	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}

void PathFinding() {
	//the map info should be an array W*H of bytes where 0 means block, 1 means walkable
	grid = new uint8[W*H];
	//here we must fill the map with all the info
	//...
	//when we want to find the shortest path, this array contains the shortest path, every value is the Nth position in the map, 100 steps max
	int output[100];

	//we call the path function, it returns the number of steps to reach target, otherwise 0
	int path_steps = AStarFindPathNoTieDiag(
			startx, starty, //origin (tienen que ser enteros)
			targetx, targety, //target (tienen que ser enteros)
			grid, //pointer to map data
			W, H, //map width and height
			output, //pointer where the final path will be stored
			100); //max supported steps of the final path

	//check if there was a path
	if( path_steps != -1 )
	{
		for(int i = 0; i < path_steps; ++i)
			std::cout << "X: " << (output[i]%W) << ", Y: " << floor(output[i]/W) << std::endl;
	}
}

void Game::RenderGUI(Texture* tex, Shader* a_shader, float centerx, float centery, float w, float h, Vector4 tex_range, Vector4 color = Vector4(1,1,1,1), bool flipYV = false) {
	int wWidth = Game::instance->window_width;
	int wHeight = Game::instance->window_height;

	Mesh quad;
	quad.createQuad(centerx, centery, w, h, flipYV); // center_x (centro del quad), center_y, widht, height

	Camera cam2D;
	cam2D.setOrthographic(0, wWidth, wHeight, 0, -1, 1);

	if (!a_shader) return;

	//enable shader
	a_shader->enable();

	//upload uniforms
	a_shader->setUniform("u_color", color);
	a_shader->setUniform("u_viewprojection", cam2D.viewprojection_matrix);
	if (tex != NULL) {
		a_shader->setUniform("u_texture", tex, 0);
	}
	a_shader->setUniform("u_time", Game::instance->time);
	a_shader->setUniform("u_tex_range", tex_range);
	a_shader->setUniform("u_model", quadModel);
	quad.render(GL_TRIANGLES);

	//disable shader
	a_shader->disable();
}

bool Game::RenderButton(Texture* tex, Shader* a_shader, float centerx, float centery, float w, float h, Vector4 tex_range, Vector4 color = Vector4(1, 1, 1, 1), bool flipYV = false) {
	
	Vector2 mouse = Input::mouse_position;
	float halfWidth = w * 0.5;
	float halfHeight = h * 0.5;
	float min_x = centerx - halfWidth;
	float max_x = centerx + halfWidth;
	float min_y = centery - halfHeight;
	float max_y = centery + halfHeight;

	bool hover = (mouse.x >= min_x) && (mouse.x <= max_x) && (mouse.y >= min_y) && (mouse.y <= max_y);
	Vector4 buttonColor = hover ? Vector4(1, 1, 1, 1) : Vector4(1, 1, 1, 0.4f); // Para cuando pasas encima del icono se "ilumina"

	RenderGUI(tex, a_shader, centerx, centery, w, h, tex_range, buttonColor, flipYV);
	return wasLeftMousePressed && hover;
}

void Game::RenderAllGUIs() {
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int wWidth = Game::instance->window_width;
	int wHeight = Game::instance->window_height;

	int gui_id = 0; // Hacer que si apretas un botón, cambie el gui_id y el usuario pueda ver el otro icono
	Vector4 sprite_gui[] =
	{
		(Vector4(0, 0, 0.25, 0.5)),
		(Vector4(0.25, 0, 0.25, 0.5)),
		(Vector4(0.5, 0, 0.25, 0.5)),
		(Vector4(0.75, 0, 0.25, 0.5))
	};

	// ----------------------------- BOTONES GUI ---------------------------------------------
	
	//// Durante el juego
	if (RenderButton(gui, gui_shader, 60, 60, 60, 60, sprite_gui[gui_id])) {
		std::cout << "left one" << std::endl;
		//audio->PlayAudio(hSample3);
	}
	//if (RenderButton(gui, gui_shader, 120, 60, 60, 60, sprite_gui[2])) {
	//	std::cout << "right one" << std::endl;
	//	glViewport(0, 0, wWidth, wHeight);
	//	audio->PlayAudio(hSample3);
	//}
	//// Al iniciar el juego
	//if (RenderButton(menu_inicial, gui_shader, 400, 300, 200, 100, Vector4(0, 0, 1, 0.5))) {
	//	std::cout << "start" << std::endl;
	//	audio->PlayAudio(hSample3);
	//}
	//if (RenderButton(menu_inicial, gui_shader, 400, 400, 200, 100, Vector4(0, 0.5, 1, 0.5))) {
	//	std::cout << "tutorial" << std::endl;
	//	audio->PlayAudio(hSample3);
	//}
	//// Hacer como menu desplegable al clicar el sprite_gui[0]
	//if (RenderButton(menu_game, gui_shader, 60, 200, 100, 70, Vector4(0, 0, 0.5, 0.5))) {
	//	std::cout << "play" << std::endl;
	//	audio->PlayAudio(hSample3);
	//}
	//if (RenderButton(menu_game, gui_shader, 60, 270, 100, 70, Vector4(0.5, 0, 0.5, 0.5))) {
	//	std::cout << "restart" << std::endl;
	//	audio->PlayAudio(hSample3);
	//}
	//if (RenderButton(menu_game, gui_shader, 60, 340, 100, 70, Vector4(0, 0.5, 0.5, 0.5))) {
	//	std::cout << "quit" << std::endl;
	//	audio->PlayAudio(hSample3);
	//}
	// ----------------------------- BOTONES GUI ---------------------------------------------

	glEnable(GL_DEPTH_TEST); // No necesarias creo
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void RenderScene(Camera* camera, Shader* shader, Shader* anim_shader, float time, int s, int f)
{
	//render entities
	//std::cout << scene->entities.size() << std::endl;
	for (int i = s; i < f; ++i)
	{
		Entity* ent = scene->entities[i];

		if (ent->entity_type == eEntityType::OBJECT || ent->entity_type == eEntityType::HINT) {
			RenderMesh(ent->model, ent->mesh, ent->texture, shader, camera, GL_TRIANGLES);
		}
		else if (ent->entity_type == eEntityType::PLAYER) {
			scene->player = new Player(ent);
			scene->player->model.setScale(0.01f, 0.01f, 0.01f);
			Vector3 pos = scene->player->model.getTranslation();
			scene->player->model.translate(pos.x * 10, pos.y * 10, pos.z * 10);
			if(currentAnim == ePlayerState::IDLE)
				ent->RenderMeshWithAnim(scene->player->model, scene->player->mesh, scene->player->texture, scene->player->anim_idle, anim_shader, camera, GL_TRIANGLES, time);
			if (currentAnim == ePlayerState::WALK)
				ent->RenderMeshWithAnim(scene->player->model, scene->player->mesh, scene->player->texture, scene->player->anim_walk, anim_shader, camera, GL_TRIANGLES, time);
			if (currentAnim == ePlayerState::RUN)
				ent->RenderMeshWithAnim(scene->player->model, scene->player->mesh, scene->player->texture, scene->player->anim_run, anim_shader, camera, GL_TRIANGLES, time);
		}
	}
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

	//set the camera as default
	camera->enable();

	// Sky
	RenderSky(skyModel, skyMesh, skyTex, shader, camera);

	// Floor
	RenderMesh(floorModel, floorMesh, floorTex, shader, camera, GL_TRIANGLES);
	floorModel.setScale(100, 0, 100);

	//Render Levels
	/*scene->player->model.translate(scene->player->pos.x, scene->player->pos.y, scene->player->pos.z);
	scene->player->model.rotate(scene->player->yaw * DEG2RAD, Vector3(0, 1, 0));*/
	if(level==1)
		RenderScene(camera, shader, a_shader,time, 0, 71);
	else if(level==2)
		RenderScene(camera, shader, a_shader, time, 71, 188);
	else if (level == 3)
		RenderScene(camera, shader, a_shader, time, 188, scene->entities.size());
	// Detective --> PLAYER

	// Render Stages
	GetStage(currentStage, stages)->Render();

	//Draw the floor grid
	//drawGrid();

	//render the FPS, Draw Calls, etc
	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);

	RenderAllGUIs();

	wasLeftMousePressed = false;

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

// ----------------------------- STAGES --------------------------------------------------------------
void NextStage() { // Para pasar de stage
    int nextStageIndex = (((int)currentStage) + 1) % stages.size();
    SetStage(&currentStage, (STAGE_ID)nextStageIndex);
}
// ----------------------------- STAGES --------------------------------------------------------------


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

	if (Input::wasKeyPressed(SDL_SCANCODE_TAB)) { //CAMBIAR MODO DE LA CAMARA
		cameraLocked = !cameraLocked;
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_P)) { //CAMBIAR MODO DE LA CAMARA
		if (level > 2)
			level = 1;
		else
			level++;
	}

	if (cameraLocked) //SI ESTAMOS EN 1a PERSONA
	{
		mouse_locked = true;
		scene->player->UpdatePlayer(elapsed_time, camera);
	}
	else { //CAMARA LIBRE
		mouse_locked = false;
		float playerSpeed = 120.0f * elapsed_time;
		float rotSpeed = 200.0f * elapsed_time;
		SDL_ShowCursor(true);

		if (Input::isKeyPressed(SDL_SCANCODE_E)) scene->player->yaw += rotSpeed;
		if (Input::isKeyPressed(SDL_SCANCODE_Q)) scene->player->yaw -= rotSpeed;;

		Matrix44 playerRotate;
		playerRotate.rotate(scene->player->yaw * DEG2RAD, Vector3(0, 1, 0));
		Vector3 forward = playerRotate.rotateVector(Vector3(0, 0, -1));
		Vector3 right = playerRotate.rotateVector(Vector3(1, 0, 0));

		Vector3 playerVel;

		if (Input::isKeyPressed(SDL_SCANCODE_S)) playerVel = playerVel + (forward * playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_D)) playerVel = playerVel - (right * playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_A)) playerVel = playerVel + (right * playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_W) && Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) {
			playerVel = playerVel - (forward * playerSpeed * 2);
			currentAnim = ePlayerState::RUN;
		} else if(Input::isKeyPressed(SDL_SCANCODE_W)) {
			playerVel = playerVel - (forward * playerSpeed);
			currentAnim = ePlayerState::WALK;
		}
		else {
			currentAnim = ePlayerState::IDLE;
		}

		if (currentAnim == ePlayerState::WALK) {
			anim = scene->player->anim_walk;
		}
		else if (currentAnim == ePlayerState::RUN) {
			anim = scene->player->anim_run;
		}

		playerVel = scene->player->PlayerCollisions(scene, camera, playerVel, elapsed_time);

		scene->player->model.translateGlobal(playerVel.x, playerVel.y, playerVel.z); // Character controller

		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 2; //move faster with left shift
		if (Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * (speed/2));
		if (Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f, -1.0f) * (speed / 2));
		if (Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * (speed / 2));
		if (Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * (speed / 2));
	}
	
	//scene->player->UpdatePlayer(elapsed_time, cameraLocked, camera, currentAnim);
	// ----------------------------- STAGES --------------------------------------------------------------
	GetStage(currentStage, stages)->Update(seconds_elapsed); // Actualiza la stage que toca
	// ----------------------------- STAGES --------------------------------------------------------------
	

	////Read the keyboard state, to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	if (Input::wasKeyPressed(SDL_SCANCODE_SPACE)) { //SPACE para pasar de Info a Tutorial a Level1
		if (currentStage < STAGE_ID::LEVEL1) {
			NextStage();
		}
		if (currentStage == STAGE_ID::END) {
			must_exit = true;
		}
	}
	// ----------------------------- STAGES --------------------------------------------------------------


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
	case SDLK_3: CheckCollision(camera, scene->entities, selectedEntity); break;
	case SDLK_4: CheckSkyCollision(camera, skyModel, skyMesh); break;
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
	if (event.button == SDL_BUTTON_LEFT) //middle mouse
	{
		wasLeftMousePressed = true;
	}

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