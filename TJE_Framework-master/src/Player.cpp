#include "player.h"
#include "shader.h"
#include "texture.h"
#include "input.h"
#include "game.h"
#include "scene.h"

Player::Player()
{
}

Player::Player( Entity* ent)
{
	this->mesh = ent->mesh;
	this->model = ent->model;
	this->texture = ent->texture;
	this->pitch = 2.f;
	this->yaw = 50.f * DEG2RAD;
	this->anim_idle = Animation::Get("data/animations/detective_idle.skanim");
	this->anim_walk = Animation::Get("data/animations/detective_walk.skanim");
	this->anim_run = Animation::Get("data/animations/detective_running.skanim");
	this->player_state = IDLE;
}

Camera* Player::InitPlayerCamera()
{
	Camera* camera = new Camera();
	camera->lookAt(Vector3(this->model.getTranslation().x, 1.9f, this->model.getTranslation().z), Vector3(this->model.getTranslation().x, 1.9f, this->model.getTranslation().z - 1), Vector3(0.f, 1.f, 0.f));
	camera->setPerspective(70.f, Game::instance->window_width / (float)Game::instance->window_height, 0.1f, 10000.f); //set the projection, we want to be perspective
	return camera;
}

Vector3 Player::PlayerCollisions(Scene* scene, Camera* camera, Vector3 playerVel, float elapsed_time) {
	//calculamos el centro de la esfera de colisión del player elevandola hasta la cintura
	

	Vector3 playerPos = this->model.getTranslation();
	//para cada objecto de la escena...
	Vector3 localDelta = camera->getLocalVector(playerVel);
	localDelta.y = 0;
	Vector3 nextPos = playerPos - localDelta;
	Vector3 character_center = nextPos + Vector3(0, 4, 0);

	for (size_t i = 0; i < scene->entities.size(); i++) {
		Entity* entity = scene->entities[i];

		Vector3 coll;
		Vector3 collnorm;
		
		if (entity->entity_type == eEntityType::PLAYER)
			continue;
		//comprobamos si colisiona el objeto con la esfera (radio 3)
		if (!entity->mesh->testSphereCollision(entity->model, character_center, 0.6f, coll, collnorm))
			continue; //si no colisiona, pasamos al siguiente objeto
		//si la esfera está colisionando muevela a su posicion anterior alejandola del objeto
		Vector3 push_away = normalize(coll - character_center) * elapsed_time * 10.0f;
		nextPos = this->model.getTranslation() - push_away; //move to previous pos but a little bit further

		//cuidado con la Y, si nuestro juego es 2D la ponemos a 0
		nextPos.y = 0;

		//reflejamos el vector velocidad para que de la sensacion de que rebota en la pared
		//velocity = reflect(velocity, collnorm) * 0.95;
	}

	Vector3 movement = nextPos - playerPos;
	Vector3 modelCoordinates = Vector3(movement.x, 0.0f, movement.z);

	return modelCoordinates;
}

void Player::RenderPlayer(Matrix44 model, Mesh* mesh, Texture* tex, Animation* anim, Shader* shader, Camera* cam,int primitive, float yaw, float pitch, float t)
{
	if (!shader) return;
	//enable shader
	shader->enable();

	anim->assignTime(t);

	//upload uniforms
	shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
	if (tex != NULL) {
		shader->setUniform("u_texture", tex, 0);
	}
	shader->setUniform("u_time", time);
	shader->setUniform("u_tex_tiling", 1.0f);
	shader->setUniform("u_model", model);
	mesh->renderAnimated(primitive, &anim->skeleton);

	//disable shader
	shader->disable();
}

void Player::UpdatePlayer(float elapsed_time, Camera* camera)
{
	ePlayerState currentAnim;
	//float speed = elapsed_time * 50.f;
	Scene* scene = Scene::instance;
	Vector3 Player_Move;
	float rotSpeed = 50.f * DEG2RAD * elapsed_time;
	pitch += Input::mouse_delta.y * 10.0f * elapsed_time;
	yaw += Input::mouse_delta.x * 10.0f * elapsed_time;
	Input::centerMouse();
	SDL_ShowCursor(false);

	this->model.rotate(Input::mouse_delta.x * rotSpeed, Vector3(0.0f, -1.0f, 0.0f)); //Rotamos el modelo del jugador y con él, la camara.
	camera->rotate(Input::mouse_delta.x * rotSpeed, Vector3(0.0f, -1.0f, 0.0f));
	camera->rotate(Input::mouse_delta.y * rotSpeed, camera->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));

	float playerSpeed = 10.f * elapsed_time;

		
	

	if (Input::isKeyPressed(SDL_SCANCODE_S)) Player_Move = Player_Move + Vector3(0.f, 0.f, playerSpeed);
	if (Input::isKeyPressed(SDL_SCANCODE_D)) Player_Move = Player_Move + Vector3(playerSpeed, 0.f, 0.f);
	if (Input::isKeyPressed(SDL_SCANCODE_A)) Player_Move = Player_Move + Vector3(-playerSpeed, 0.f, 0.f);

	if (Input::isKeyPressed(SDL_SCANCODE_W) && Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) {
		Player_Move = Player_Move + (Vector3(0.f, 0.f, -playerSpeed)*2);
		currentAnim = ePlayerState::RUN;
	}
	else if (Input::isKeyPressed(SDL_SCANCODE_W)) {
		Player_Move = Player_Move + Vector3(0.f, 0.f, -playerSpeed);
		currentAnim = ePlayerState::WALK;
	}
	else {
		currentAnim = ePlayerState::IDLE;
	}

	if (Input::isKeyPressed(SDL_SCANCODE_W) && Input::isKeyPressed(SDL_SCANCODE_A)) Player_Move = Player_Move + Vector3(playerSpeed, 0.f, -playerSpeed);
	if (Input::isKeyPressed(SDL_SCANCODE_S) && Input::isKeyPressed(SDL_SCANCODE_A)) Player_Move = Player_Move + Vector3(playerSpeed, 0.f, playerSpeed);
	if (Input::isKeyPressed(SDL_SCANCODE_W) && Input::isKeyPressed(SDL_SCANCODE_D)) Player_Move = Player_Move + Vector3(-playerSpeed, 0.f, -playerSpeed);
	if (Input::isKeyPressed(SDL_SCANCODE_S) && Input::isKeyPressed(SDL_SCANCODE_D)) Player_Move = Player_Move + Vector3(-playerSpeed, 0.f, playerSpeed);

	//Vector3 wp_vector_player = camera->getLocalVector(Player_Move);

	Player_Move = PlayerCollisions(scene, camera, Player_Move, elapsed_time);

	model.translateGlobal(-Player_Move.x, 0.0f, -Player_Move.z);

	//camera->updateViewMatrix();

	camera->eye.x -= Player_Move.x;
	camera->eye.z -= Player_Move.z;

	camera->center.x -= Player_Move.x;
	camera->center.z -= Player_Move.z;

	Animation* anim;
	if (currentAnim == ePlayerState::WALK) {
		anim = anim_walk;
	}
	else if (currentAnim == ePlayerState::RUN) {
		anim = anim_run;
	}
}
