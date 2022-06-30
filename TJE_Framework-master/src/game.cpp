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

// Some globals
Mesh* skyMesh = NULL;
Texture* skyTex = NULL;
Matrix44 skyModel;

Mesh* floorMesh;
Texture* floorTex;
Matrix44 floorModel;

Matrix44 viewmodel;

Animation* anim = NULL;
float angle = 0;
float mouse_speed = 50.0f;
FBO* fbo = NULL;

Game* Game::instance = NULL;

Scene* scene = new Scene();

const int houses_width = 10;
const int houses_height = 10;
float padding = 10.0f; // Distancia entre las houses
float lodDist = 10.0f; // Para los LODs
float no_render_dist = 1000.0f; // Para el frustum
bool cameraLocked = false;
const bool firstP = true;
int objectType = 0;
int cameracontroller = 0;


Entity* selectedEntity = NULL;
std::vector<LightEntity*> lights;

bool wasLeftMousePressed = false;

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
    stages.reserve(8); //Reserva 7 elementos (hace un malloc de 7 porque tenemos 7 stages)
    stages.push_back(new IntroStage()); //Para añadir elementos a un std vector = pushback
    stages.push_back(new InfoStage());
    stages.push_back(new TutorialStage());
    stages.push_back(new Level1Stage());
	stages.push_back(new Level2Stage());
	stages.push_back(new Level3Stage());
	stages.push_back(new EndStage());
	stages.push_back(new ExitStage());
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

	floorMesh = Mesh::Get("data/assets/sand2.obj");
	floorTex = Texture::Get("data/textures/sand.tga");

	// Levels
	scene->loadMap("data/scenes/TrainStation/train_station.scene");
	scene->loadMap("data/scenes/Pueblo/pueblo.scene");
	scene->loadMap("data/scenes/House/house.scene");

	std::cout << scene->entities[2] << std::endl;

	// GUI
	menu_inicial = Texture::Get("data/gui/intro.png");
	menu = Texture::Get("data/gui/menu.png");
	menu1 = Texture::Get("data/gui/menu1.png");

	// Shaders
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
	gui_shader = Shader::Get("data/shaders/basic.vs", "data/shaders/gui.fs");
	a_shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");

	// Inicializar Stages
	InitStages();
	level = 0;
	numPistas = 0;

	// Init bass --> Para el audio
	if (BASS_Init(-1, 44100, 0, 0, NULL) == false) //-1 significa usar el por defecto del sistema operativo
	{
		std::cout << "ERROR initializing audio" << std::endl;
	}
	//
	hSample1 = audio->loadAudio("data/audio/intro.wav", BASS_SAMPLE_LOOP);
	hSample2 = audio->loadAudio("data/audio/button.wav", 0);

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
}

void Game::IntroGUI() {
	//// Al iniciar el juego
	audio->PlayAudio(hSample1);
	if (RenderButton(menu_inicial, gui_shader, 400, 300, 200, 100, Vector4(0, 0, 1, 0.5))) {
		audio->PlayAudio(hSample2);
		// Cambiar al Info
		currentStage = STAGE_ID::INFO;
		level = 0;
	}
	if (RenderButton(menu_inicial, gui_shader, 400, 400, 200, 100, Vector4(0, 0.5, 1, 0.5))) {
		audio->PlayAudio(hSample2);
		// Cambiar al Tutorial
		currentStage = STAGE_ID::TUTORIAL;
		level = 0;
	}
}

void Game::LevelsGUI() {

	if (RenderButton(menu, gui_shader, 40, 60, 60, 60, Vector4(0, 0.05, 0.5, 1))) {
		audio->PlayAudio(hSample2);
		// Cambiar a Intro
		currentStage = STAGE_ID::INTRO;
		level = 0;
	}
	if (RenderButton(menu, gui_shader, 97, 60, 60, 60, Vector4(0.5, 0.03, 0.5, 1))) {
		audio->PlayAudio(hSample2);
		// Cambiar a End
		currentStage = STAGE_ID::END;
		level = 0;
	}
}

void Game::InfoGUI() {
	if (RenderButton(menu1, gui_shader, 400, 350, 200, 100, Vector4(0, 0, 0.5, 0.5))) {
		audio->PlayAudio(hSample2);
		// Cambiar a Intro
		currentStage = STAGE_ID::LEVEL1;
	}
	if (RenderButton(menu_inicial, gui_shader, 400, 450, 200, 100, Vector4(0, 0.5, 1, 0.5))) {
		audio->PlayAudio(hSample2);
		// Cambiar a End
		currentStage = STAGE_ID::TUTORIAL;
	}
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
			Vector3 pos = scene->player->model.getTranslation();
			scene->player->model.translate(pos.x, pos.y, pos.z);
			scene->player->model.rotate(scene->player->yaw, Vector3(0.f, 1.f, 0.f));
			camera = scene->player->camera;
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
	if(level==1)
		RenderScene(camera, shader, a_shader,time, 0, 71);
	else if(level==2)
		RenderScene(camera, shader, a_shader, time, 71, 188);
	else if (level == 3)
		RenderScene(camera, shader, a_shader, time, 188, scene->entities.size());

	// Render Stages
	GetStage(currentStage, stages)->Render();

	//if (currentStage == STAGE_ID::LEVEL1 || currentStage == STAGE_ID::LEVEL2 || currentStage == STAGE_ID::LEVEL3) {
	//	objectType = scene->player->CheckCollision(camera, scene->entities, selectedEntity);
	//}

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
	
	// ----------------------------- STAGES --------------------------------------------------------------
	GetStage(currentStage, stages)->Update(seconds_elapsed); // Actualiza la stage que toca
	// ----------------------------- STAGES --------------------------------------------------------------
	
	if (currentStage == STAGE_ID::LEVEL1 || currentStage == STAGE_ID::LEVEL2 || currentStage == STAGE_ID::LEVEL3) {
		//1a PERSONA
		Vector3 Player_Move;
		float rotSpeed = 10.f * DEG2RAD * elapsed_time;
		scene->player->model.rotate(Input::mouse_delta.x * rotSpeed, Vector3(0.0f, -1.0f, 0.0f)); //Rotamos el modelo del jugador y con �l, la camara.
		camera->rotate(Input::mouse_delta.x * rotSpeed, Vector3(0.0f, -1.0f, 0.0f));
		camera->rotate(Input::mouse_delta.y * rotSpeed, camera->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));

		float playerSpeed = 3.f * elapsed_time;

		if (firstP) {
			scene->player->pitch += Input::mouse_delta.y * 10.0f * elapsed_time;
			scene->player->yaw += Input::mouse_delta.x * 10.0f * elapsed_time;
			Input::centerMouse();
			SDL_ShowCursor(true);
		}

		if (Input::isKeyPressed(SDL_SCANCODE_W)) Player_Move = Vector3(0.f, 0.f, playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_S)) Player_Move = Vector3(0.f, 0.f, -playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_A)) Player_Move = Vector3(playerSpeed, 0.f, 0.f);
		if (Input::isKeyPressed(SDL_SCANCODE_D)) Player_Move = Vector3(-playerSpeed, 0.f, 0.f);
		if (Input::isKeyPressed(SDL_SCANCODE_W) && Input::isKeyPressed(SDL_SCANCODE_A)) Player_Move = Vector3(playerSpeed, 0.f, playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_S) && Input::isKeyPressed(SDL_SCANCODE_A)) Player_Move = Vector3(playerSpeed, 0.f, -playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_W) && Input::isKeyPressed(SDL_SCANCODE_D)) Player_Move = Vector3(-playerSpeed, 0.f, playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_S) && Input::isKeyPressed(SDL_SCANCODE_D)) Player_Move = Vector3(-playerSpeed, 0.f, -playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT) && Input::isKeyPressed(SDL_SCANCODE_W)) Player_Move = Vector3(0.f, 0.f, playerSpeed * 2); //move faster with left shift

		Vector3 wpv_player = camera->getLocalVector(Player_Move);

		camera->eye.x -= wpv_player.x;
		camera->eye.z -= wpv_player.z;

		camera->center.x -= wpv_player.x;
		camera->center.z -= wpv_player.z;

		scene->player->model.translateGlobal(-wpv_player.x, 0.0f, -wpv_player.z);


		if (scene->player->DetectHint(camera, scene->entities, selectedEntity)) {
			//MOSTRAMOS POR PANTALLA "F PARA INVESTIGAR PISTA"
			if (Input::wasKeyPressed(SDL_SCANCODE_F)) { // Update Pistas
				numPistas++;
			}
		}
		if (numPistas == 3 && level == 1) {
			level = 2;
			NextStage();

		}
		else if (numPistas == 5 && level == 2) {
			level = 3;
			NextStage();

		}
		else if (numPistas == 10) {
			currentStage = STAGE_ID::END;
		}
	}

	////Read the keyboard state, to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	if (Input::wasKeyPressed(SDL_SCANCODE_SPACE)) { //SPACE para pasar de Info a Tutorial a Level1
		if (currentStage < STAGE_ID::EXIT) {
			NextStage();
		}
		if (currentStage == STAGE_ID::EXIT) {
			must_exit = true;
		}
	}
	
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