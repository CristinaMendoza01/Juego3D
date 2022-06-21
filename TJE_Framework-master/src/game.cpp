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
#include "Player.h"

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
Matrix44 femaleModel;
Animation* walkingf;

Mesh* maleMesh = NULL;
Texture* maleTex = NULL;
Matrix44 maleModel;

Animation* anim = NULL;
float angle = 0;
float mouse_speed = 100.0f;
FBO* fbo = NULL;

Game* Game::instance = NULL;

Player female;

const int houses_width = 10;
const int houses_height = 10;
float padding = 70.0f; // Distancia entre las houses
float lodDist = 10.0f; // Para los LODs
float no_render_dist = 1000.0f; // Para el frustum
bool cameraLocked = true;

std::vector<Entity*> entities;
std::vector<LightEntity*> lights;

// ----------------------------- STAGES --------------------------------------------------------------
//std::vector<Stage*> stages; // Vector stages
//STAGE_ID currentStage = STAGE_ID::INTRO; //Índice que nos dice en que stage estamos, inicializamos con la INTRO

//void InitStages() {
//    stages.reserve(4); //Reserva 7 elementos (hace un malloc de 7 porque tenemos 7 stages)
//    stages.push_back(new IntroStage()); //Para añadir elementos a un std vector = pushback
//    stages.push_back(new InfoStage());
//    stages.push_back(new TutorialStage());
//    stages.push_back(new Level1Stage());
//}
// ----------------------------- STAGES --------------------------------------------------------------

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


	// ----------------------------- ARNAU --------------------------------------------------------------
	Vector3 eye = maleModel * Vector3(0.f, 15.f, 1.f);
	Vector3 center = maleModel * Vector3(0.f, 15.f, 10.f);
	Vector3 up = maleModel.rotateVector(Vector3(0.f, 1.f, 0.f));

	//Create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f, 100.f, 100.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0
	// ----------------------------- ARNAU --------------------------------------------------------------
	if (cameraLocked) //Entramos en modo 1a Persona.
		camera->lookAt(eye, center, up);
	camera->setPerspective(70.f, window_width / (float)window_height, 0.1f, 1000000.f); //set the projection, we want to be perspective
	// Si añadimos unos ceros más en el último param -> Vemos más lejos

	skyMesh = Mesh::Get("data/cielo.ASE");
	skyTex = Texture::Get("data/cielo.tga");

	houseMesh = Mesh::Get("data/house1.ASE");
	houseTex = Texture::Get("data/houses_and_windows.tga");

	femaleMesh = Mesh::Get("data/female.mesh");
	femaleTex = Texture::Get("data/female.tga");
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
	//PlayAudio("data/audio/mistery.wav");

	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}

void RenderAllGUIs() {
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
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


	//set the camera as default
	camera->enable();

	// ----------------------------- ARNAU --------------------------------------------------------------

	RenderMesh(skyModel, skyMesh, skyTex, shader, camera);
	skyModel.setScale(100, 100, 100);

	RenderObjects(houseMesh, houseTex, shader, houses_width, houses_height, padding, no_render_dist);

	// Anim
	femaleModel.translate(female.pos.x, female.pos.y, female.pos.z);
	femaleModel.rotate(female.yaw * DEG2RAD, Vector3(0, 1, 0));
	RenderMeshWithAnim(femaleModel, femaleMesh, femaleTex, walkingf, shader, camera, time);

	// ----------------------------- ARNAU --------------------------------------------------------------
	//RenderMesh(maleModel, maleMesh, maleTex, shader, camera);
	// ----------------------------- ARNAU --------------------------------------------------------------

	for (size_t i = 0; i < entities.size(); i++) { // Para el AddEntityInFront
		Entity* entity = entities[i];
		RenderMesh(entity->model, entity->mesh, entity->texture, shader, camera);
	}

	// ----------------------------- STAGES --------------------------------------------------------------

	//GetStage(currentStage, stages)->Render();

	//if (GetStage(currentStage, stages) == GetStage(STAGE_ID::INTRO, stages)) {

	//}
	//if (GetStage(currentStage, stages) == GetStage(STAGE_ID::INFO, stages)) {

	//}
	//if (GetStage(currentStage, stages) == GetStage(STAGE_ID::TUTORIAL, stages)) {

	//}
	//if (GetStage(currentStage, stages) == GetStage(STAGE_ID::LEVEL1, stages)) {

		//    // ----------------------------- CAMARA 1A PERSONA --------------------------------------------------------------
		//Vector3 eye = maleModel * Vector3(0.f, 15.f, 1.f);
		//Vector3 center = maleModel * Vector3(0.f, 15.f, 10.f);
		//Vector3 up = maleModel.rotateVector(Vector3(0.f, 1.f, 0.f));
		// ----------------------------- CAMARA 1A PERSONA --------------------------------------------------------------

		////set the camera as default
		//camera->enable();

		//// ----------------------------- camara 1a persona --------------------------------------------------------------
		//if (cameralocked) {//entramos en modo 1a persona.
		//    vector3 eye = malemodel * vector3(0, 15, 1);
		//    vector3 center = malemodel * vector3(0, 15, 10);
		//    vector3 up = vector3(0, 1, 0);
		//    camera->lookat(eye, center, up);
		//}
	//// ----------------------------- camara 1a persona --------------------------------------------------------------

		//rendermesh(skymodel, skymesh, skytex, shader, camera);
		//skymodel.setscale(100, 100, 100);

		//renderobjects(housemesh, housetex, shader, houses_width, houses_height, padding, no_render_dist);

		//// anim
		//femalemodel.translate(female.pos.x, female.pos.y, female.pos.z);
		//femalemodel.rotate(female.yaw * deg2rad, vector3(0, 1, 0));
		//rendermeshwithanim(femalemodel, femalemesh, femaletex, walkingf, shader, camera, time);

		//// ----------------------------- mesh 1a persona --------------------------------------------------------------
		////rendermesh(malemodel, malemesh, maletex, shader, camera);
		//// ----------------------------- mesh 1a persona --------------------------------------------------------------

		//for (size_t i = 0; i < entities.size(); i++) { // para el addentityinfront
		//    entity* entity = entities[i];
		//    rendermesh(entity->model, entity->mesh, entity->texture, shader, camera);
		//}
	//}
	// ----------------------------- STAGES --------------------------------------------------------------


	//Draw the floor grid
	drawGrid();

	//render the FPS, Draw Calls, etc
	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

// ----------------------------- STAGES --------------------------------------------------------------
//void NextStage() { // Para pasar de stage
//    int nextStageIndex = (((int)currentStage) + 1) % stages.size();
//    SetStage(&currentStage, (STAGE_ID)nextStageIndex);
//}
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
		Vector3 Player_Move;
		float rotSpeed = 60.f * DEG2RAD * elapsed_time;
		maleModel.rotate(Input::mouse_delta.x * rotSpeed, Vector3(0.0f, -1.0f, 0.0f)); //Rotamos el modelo del jugador y con él, la camara.
		camera->rotate(Input::mouse_delta.x * rotSpeed, Vector3(0.0f, -1.0f, 0.0f));
		camera->rotate(Input::mouse_delta.y * rotSpeed, camera->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));

		float playerSpeed = 50.f * elapsed_time;

		if (Input::isKeyPressed(SDL_SCANCODE_W)) Player_Move= Vector3(0.f, 0.f, playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_S)) Player_Move = Vector3(0.f, 0.f, -playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_A)) Player_Move = Vector3(playerSpeed, 0.f, 0.f);
		if (Input::isKeyPressed(SDL_SCANCODE_D)) Player_Move = Vector3(-playerSpeed, 0.f, 0.f);
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) playerSpeed *= 10; //move faster with left shift

		Vector3 wpv_player = camera->getLocalVector(Player_Move);

		camera->eye.x -= wpv_player.x;
		camera->eye.z -= wpv_player.z;

		camera->center.x -= wpv_player.x;
		camera->center.z -= wpv_player.z;

		maleModel.translateGlobal(-wpv_player.x, 0.0f, -wpv_player.z);
		
	}
	else { //CAMARA LIBRE
		mouse_locked = false;
		float playerSpeed = 0.05f * elapsed_time;
		float rotSpeed = 120.0f * DEG2RAD * elapsed_time;

		if (Input::isKeyPressed(SDL_SCANCODE_E)) female.yaw += rotSpeed;
		if (Input::isKeyPressed(SDL_SCANCODE_Q)) female.yaw -= rotSpeed;;

		Matrix44 playerRotate;
		playerRotate.rotate(female.yaw * DEG2RAD, Vector3(0, 1, 0));
		Vector3 forward = playerRotate.rotateVector(Vector3(0, 0, -1));
		Vector3 right = playerRotate.rotateVector(Vector3(1, 0, 0));

		Vector3 playerVel;

		if (Input::isKeyPressed(SDL_SCANCODE_S)) playerVel = playerVel + (forward * playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_W)) playerVel = playerVel - (forward * playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_D)) playerVel = playerVel - (right * playerSpeed);
		if (Input::isKeyPressed(SDL_SCANCODE_A)) playerVel = playerVel + (right * playerSpeed);

		female.pos = female.pos + playerVel; // Character controller

		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
		if (Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);

		////async input to move the camera around
		//if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
		//if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
		//if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
		//if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
		//if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);

	}

	// ----------------------------- STAGES --------------------------------------------------------------
	//GetStage(currentStage, stages)->Update(seconds_elapsed); // Actualiza la stage que toca

	//if (GetStage(currentStage, stages) == GetStage(STAGE_ID::LEVEL1, stages)) {
	//	if (Input::wasKeyPressed(SDL_SCANCODE_TAB)) { //CAMBIAR MODO DE LA CAMARA
	//		cameraLocked = !cameraLocked;
	//	}

	//	if (cameraLocked) //SI ESTAMOS EN 1a PERSONA
	//	{
	//		mouse_locked = true;
	//		float rotSpeed = 60.f * DEG2RAD * elapsed_time;
	//		maleModel.rotate(Input::mouse_delta.x * rotSpeed, Vector3(0.0f, -1.0f, 0.0f)); //Rotamos el modelo del jugador y con Ã©l, la camara.
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