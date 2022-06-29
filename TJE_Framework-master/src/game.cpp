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
Shader* shader = NULL;
Shader* gui_shader = NULL;
Shader* a_shader = NULL;

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

//GUIs
Matrix44 quadModel;
Texture* pause = NULL;
Texture* gui = NULL;
Texture* menu_inicial = NULL;
Texture* menu_game = NULL;

Animation* anim = NULL;
float angle = 0;
float mouse_speed = 100.0f;
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

Entity* selectedEntity = NULL;
std::vector<LightEntity*> lights;

std::pair <Mesh*, Texture*> campingLevel;
std::pair <Mesh*, Texture*> cityLevel;
std::pair <Mesh*, Texture*> houseLevel;

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
	

	// Some meshes
	femaleMesh = Mesh::Get("data/assets/female.mesh");
	femaleTex = Texture::Get("data/textures/female.tga");
	walkingf = Animation::Get("data/animations/walking_female.skanim");

	maleMesh = Mesh::Get("data/assets/male.mesh");
	maleTex = Texture::Get("data/textures/male.tga");

	// Meshes principales
	skyMesh = Mesh::Get("data/assets/cielo.ASE");
	skyTex = Texture::Get("data/textures/cielo.tga");

	/*floorMesh = new Mesh();
	floorMesh->createPlane(1000);*/
	floorMesh = Mesh::Get("data/assets/sand2.obj");
	floorTex = Texture::Get("data/textures/sand.tga");

	// Levels
	//campingLevel = scene->loadScene("data/scenes/Level1/camping.obj", "data/scenes/Level1/camping.png");
	//cityLevel = scene->loadScene("data/scenes/Level2/city.obj", "data/scenes/Level2/city.png");
	//houseLevel = scene->loadScene("data/scenes/Level3/house.obj", "data/scenes/Level3/house.png");

	scene->loadMap("data/scenes/Pueblo/pueblo.scene");

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
	putCamera(Matrix44(), camera, cameraLocked, window_width, window_height);

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

void RenderGUI(Texture* tex, Shader* a_shader, float centerx, float centery, float w, float h, Vector4 tex_range, Vector4 color = Vector4(1,1,1,1), bool flipYV = false) {
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

bool RenderButton(Texture* tex, Shader* a_shader, float centerx, float centery, float w, float h, Vector4 tex_range, Vector4 color = Vector4(1, 1, 1, 1), bool flipYV = false) {
	
	Vector2 mouse = Input::mouse_position;
	float halfWidth = w * 0.5;
	float halfHeight = h * 0.5;
	float min_x = centerx - halfWidth;
	float max_x = centerx + halfWidth;
	float min_y = centery - halfHeight;
	float max_y = centery + halfHeight;

	bool hover = (mouse.x >= min_x) && (mouse.x <= max_x) && (mouse.y >= min_y) && (mouse.y <= max_y);
	Vector4 buttonColor = hover ? Vector4(1, 1, 1, 1) : Vector4(1, 1, 1, 0.7f); // Para cuando pasas encima del icono se "ilumina"

	RenderGUI(tex, a_shader, centerx, centery, w, h, tex_range, buttonColor, flipYV);
	return wasLeftMousePressed && hover;
}

void RenderAllGUIs() {
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
	//if (RenderButton(gui, gui_shader, 60, 60, 60, 60, sprite_gui[gui_id])) {
	//	std::cout << "left one" << std::endl;
	//	audio->PlayAudio(hSample3);
	//}
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

void RenderScene(Camera* camera, float time)
{
	//render entities
	for (int i = 0; i < scene->entities.size(); ++i)
	{
		Entity* ent = scene->entities[i];

		if (ent->entity_type == eEntityType::OBJECT) {
			RenderMesh(ent->model, ent->mesh, ent->texture, shader, camera, GL_TRIANGLES);
		}
		else if (ent->entity_type == eEntityType::PLAYER) {
			scene->player = new Player(ent);
			scene->player->model.setScale(0.01f, 0.01f, 0.01f);
			if(currentAnim == ePlayerState::IDLE)
				RenderMeshWithAnim(scene->player->model, scene->player->mesh, scene->player->texture, scene->player->anim_idle, a_shader, camera, GL_TRIANGLES, time);
			if (currentAnim == ePlayerState::WALK)
				RenderMeshWithAnim(scene->player->model, scene->player->mesh, scene->player->texture, scene->player->anim_walk, a_shader, camera, GL_TRIANGLES, time);
			if (currentAnim == ePlayerState::RUN)
				RenderMeshWithAnim(scene->player->model, scene->player->mesh, scene->player->texture, scene->player->anim_run, a_shader, camera, GL_TRIANGLES, time);
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

	//Render Level
	scene->player->model.translate(scene->player->pos.x, scene->player->pos.y, scene->player->pos.z);
	scene->player->model.rotate(scene->player->yaw * DEG2RAD, Vector3(0, 1, 0));

	RenderScene(camera, time);

	

	// ----------------------------- LEVELS ---------------------------------------------
	// Level 1
	//RenderMesh(campingModel, campingLevel.first, campingLevel.second, shader, camera, GL_TRIANGLES);
	//campingModel.setScale(100, 100, 100);
	//MiniMapa(player, detectiveModel, houseLevel, shader);

	// Level 2
	//RenderMesh(cityModel, cityLevel.first, cityLevel.second, shader, camera, GL_TRIANGLES);
	//cityModel.setScale(100, 100, 100);
	//MiniMapa(player, detectiveModel, cityLevel, shader);


	// Level 3
	/*RenderMesh(houseModel, houseLevel.first, houseLevel.second, shader, camera, GL_TRIANGLES);
	houseModel.setScale(50, 50, 50);*/
	//MiniMapa(player, detectiveModel, houseLevel, shader);

	// ----------------------------- LEVELS ---------------------------------------------
	
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

void DetectiveCollisions(Player* player, Vector3 nexPos, float elapsed_time) {
	//calculamos el centro de la esfera de colisión del player elevandola hasta la cintura
	Vector3 character_center = nexPos + Vector3(0, 4, 0);

	//para cada objecto de la escena...

	Vector3 coll;
	Vector3 collnorm;
	for (size_t i = 0; i < scene->entities.size(); i++) {
		Entity* entity = scene->entities[i];
		//comprobamos si colisiona el objeto con la esfera (radio 3)
		if (!entity->mesh->testSphereCollision(entity->model, character_center, 0.5f, coll, collnorm))
			continue; //si no colisiona, pasamos al siguiente objeto
		//si la esfera está colisionando muevela a su posicion anterior alejandola del objeto
		Vector3 push_away = normalize(coll - character_center) * elapsed_time;
		nexPos = player->model.getTranslation() - push_away; //move to previous pos but a little bit further

		//cuidado con la Y, si nuestro juego es 2D la ponemos a 0
		nexPos.y = 0;

		//reflejamos el vector velocidad para que de la sensacion de que rebota en la pared
		//velocity = reflect(velocity, collnorm) * 0.95;
	}
	player->model.translate(nexPos.x, nexPos.y, nexPos.z);

}

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

	if (cameraLocked) //SI ESTAMOS EN 1a PERSONA
	{
		mouse_locked = true;
		Vector3 Player_Move;
		float rotSpeed = 60.f * DEG2RAD * elapsed_time;
		scene->player->model.rotate(Input::mouse_delta.x * rotSpeed, Vector3(0.0f, -1.0f, 0.0f)); //Rotamos el modelo del jugador y con él, la camara.
		camera->rotate(Input::mouse_delta.x * rotSpeed, Vector3(0.0f, -1.0f, 0.0f));
		camera->rotate(Input::mouse_delta.y * rotSpeed, camera->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));

		float playerSpeed = 50.f * elapsed_time;

		if (firstP) {
			scene->player->pitch += Input::mouse_delta.y * 10.0f * elapsed_time;
			scene->player->yaw += Input::mouse_delta.x * 10.0f * elapsed_time;
			Input::centerMouse();
			SDL_ShowCursor(false);
		}

		if (Input::isKeyPressed(SDL_SCANCODE_W)) Player_Move= Vector3(0.f, 0.f, playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_S)) Player_Move = Vector3(0.f, 0.f, -playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_A)) Player_Move = Vector3(playerSpeed, 0.f, 0.f);
		if (Input::isKeyPressed(SDL_SCANCODE_D)) Player_Move = Vector3(-playerSpeed, 0.f, 0.f);
		if (Input::isKeyPressed(SDL_SCANCODE_W) && Input::isKeyPressed(SDL_SCANCODE_A)) Player_Move = Vector3(playerSpeed, 0.f, playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_S) && Input::isKeyPressed(SDL_SCANCODE_A)) Player_Move = Vector3(playerSpeed, 0.f, -playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_W) && Input::isKeyPressed(SDL_SCANCODE_D)) Player_Move = Vector3(-playerSpeed, 0.f, playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_S) && Input::isKeyPressed(SDL_SCANCODE_D)) Player_Move = Vector3(-playerSpeed, 0.f, -playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) playerSpeed *= 10; //move faster with left shift

		Vector3 wpv_player = camera->getLocalVector(Player_Move);

		camera->eye.x -= wpv_player.x;
		camera->eye.z -= wpv_player.z;

		camera->center.x -= wpv_player.x;
		camera->center.z -= wpv_player.z;

		scene->player->model.translateGlobal(-wpv_player.x, 0.0f, -wpv_player.z);
		
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
		scene->player->UpdatePlayer(currentAnim);

		//scene->player->model.translateGlobal(playerVel.x, playerVel.y, playerVel.z); // Character controller
		scene->player->pos = scene->player->pos + playerVel;
		//Vector3 nexPos = scene->player->model.getTranslation() + playerVel;
		Vector3 nexPos = scene->player->pos + playerVel;
		DetectiveCollisions(scene->player, nexPos, elapsed_time);

		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 2; //move faster with left shift
		if (Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);

	}
	

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
	case SDLK_1: AddEntityInFront(camera, maleMesh, maleTex, scene->entities);  break;
	case SDLK_2: AddEntityInFront(camera, femaleMesh, femaleTex, scene->entities);  break;
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

/*void RenderScene(Scene* scene, Camera* camera) {

	//set the clear color (the background color)
	glClearColor(scene->background_color.x, scene->background_color.y, scene->background_color.z, 1.0);

	// Clear the color and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	checkGLErrors();

	render_calls.clear();
	lights.clear();
	//render entities
	for (int i = 0; i < scene->entities.size(); ++i)
	{
		Entity* ent = scene->entities[i];

		if (ent->entity_type == eEntityType::LIGHT) {
			LightEntity* lent = (LightEntity*)ent;
			lights.push_back(lent);
		}
		
	}
}

//renders a mesh given its transform and material
void Game::MultiPassRender(const Matrix44 model, Mesh* mesh, std::vector<LightEntity*> l, Camera* camera)
{
	//in case there is nothing to do
	if (!mesh || !mesh->getNumVertices() || !material)
		return;
	assert(glGetError() == GL_NO_ERROR);

	//define locals to simplify coding
	Shader* shader = NULL;
	Texture* color_texture = NULL;
	Texture* emissive_texture = NULL;
	Texture* occlusion_texture = NULL;
	Texture* normalmap_texture = NULL;
	Texture* metallic_roughness_texture = NULL;
	Scene* scene = Scene::instance;

	int num_lights = l.size();
	if (!num_lights)
		return;

	color_texture = material->color_texture.texture;
	emissive_texture = material->emissive_texture.texture;
	metallic_roughness_texture = material->metallic_roughness_texture.texture;
	occlusion_texture = material->occlusion_texture.texture;
	normalmap_texture = material->normal_texture.texture;
	if (color_texture == NULL)
		color_texture = Texture::getWhiteTexture(); //a 1x1 white texture
	if (emissive_texture == NULL)
		emissive_texture = Texture::getBlackTexture(); //a 1x1 white texture
	if (occlusion_texture == NULL)
		occlusion_texture = Texture::getWhiteTexture(); //a 1x1 white texture
	if (metallic_roughness_texture == NULL)
		metallic_roughness_texture = Texture::getWhiteTexture(); //a 1x1 white texture
	if (normalmap_texture == NULL) int N_ip = 1; else int N_ip = 0;

	//Vector3 emissive_factor = material->emissive_factor;
	//select the blending
	//if (material->alpha_mode == GTR::eAlphaMode::BLEND)
		//return;


	//select if render both sides of the triangles
	if (material->two_sided)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE);
	assert(glGetError() == GL_NO_ERROR);

	//chose a shader
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/light.fs");

	assert(glGetError() == GL_NO_ERROR);

	//no shader? then nothing to render
	if (!shader)
		return;
	shader->enable();

	//upload uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	float t = getTime();
	shader->setUniform("u_time", t);

	shader->setUniform("u_color", material->color);
	if (color_texture)
		shader->setUniform("u_color_texture", color_texture, 0);
	if (emissive_texture)
		shader->setUniform("u_emissive_texture", emissive_texture, 1);
	if (occlusion_texture)
		shader->setUniform("u_occlusion_texture", occlusion_texture, 2);
	if (normalmap_texture)
		shader->setUniform("u_normalmap_texture", normalmap_texture, 3);
	if (metallic_roughness_texture)
		shader->setUniform("u_metallic_roughness_texture", metallic_roughness_texture, 4);

	shader->setUniform("u_ambient_light", scene->ambient_light);

	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	for (int i = 0; i < l.size(); i++)
	{
		if (i == 0) {
			if (material->alpha_mode == eAlphaMode::BLEND)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else
				glDisable(GL_BLEND);
		}
		else {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		}

		LightEntity* light = lights[i];


		shader->setUniform("u_lcolor", light->color * light->intensity);
		shader->setUniform("u_lpos", light->model * Vector3());
		shader->setUniform("u_lmaxdist", light->max_distance);
		shader->setUniform("u_ltype", (int)light->light_type);

		if (/*light->shadowmap && light->cast_shadows) {
			shader->setUniform("u_lshadowcast", 1);
			//shader->setUniform("u_lshadowmap", light->shadowmap, 8);
			shader->setUniform("u_lshadowmap_vp", light->light_camera->viewprojection_matrix);
			shader->setUniform("u_lshadowbias", light->shadow_bias);
		}
		else {
			shader->setUniform("u_lshadowcast", 0);

		}

		//SPOT
		shader->setUniform("u_coslcone_angle", (float)cos(light->cone_angle * DEG2RAD)); //we calculate the cos and covert the angle into radiands.
		shader->setUniform("u_lcone_exp", light->cone_exp);
		shader->setUniform("u_ldir", light->model.rotateVector(Vector3(0, 0, -1)));

		//DIRECTIONAL
		shader->setUniform("u_lareasize", light->area_size);

		shader->setUniform("u_lastlight", 0);

		if (lights.size() - 1 == i) {
			shader->setUniform("u_lastlight", 1);
		}

		mesh->render(GL_TRIANGLES);
		shader->setUniform("u_ambient_light", Vector3());
	}

	//glDisable(GL_BLEND);
	//do the draw call that renders the mesh into the screen
	//mesh->render(GL_TRIANGLES);

	//disable shader
	shader->disable();

	//set the render state as it was before to avoid problems with future renders

	glDepthFunc(GL_LESS);
	glDisable(GL_BLEND);
}*/