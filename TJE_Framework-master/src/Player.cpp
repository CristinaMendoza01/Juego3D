#include "player.h"
#include "shader.h"
#include "texture.h"
#include "input.h"
#include "game.h"
#include "scene.h"
#include "entity.h"

Player::Player()
{
}

Player::Player(Entity* ent)
{
	this->mesh = ent->mesh;
	this->model = ent->model;
	this->texture = ent->texture;
	this->pitch = 0.f;
	this->yaw = 25.f * DEG2RAD;
	this->anim_idle = Animation::Get("data/animations/detective_idle.skanim");
	this->anim_walk = Animation::Get("data/animations/detective_walk.skanim");
	this->anim_run = Animation::Get("data/animations/detective_running.skanim");
	this->current_anim = this->anim_idle;
	this->player_state = IDLE;
	this->camera = InitPlayerCamera();
}

Camera* Player::InitPlayerCamera()
{
	Camera* camera = new Camera();
	Vector3 eye = this->model * Vector3(0.f, 1.f, -20.f);
	Vector3 center = this->model * Vector3(0.f, 1.f, -10.f);
	Vector3 up = this->model.rotateVector(Vector3(0.f, 1.f, 0.f));
	camera->lookAt(eye, center, up);
	camera->setPerspective(70.f, Game::instance->window_width / (float)Game::instance->window_height, 0.1f, 1000000.f); //set the projection, we want to be perspective

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
		//Vector3 push_away = normalize(coll - character_center) * elapsed_time * 10.0f;
		//nextPos = this->model.getTranslation() - push_away; //move to previous pos but a little bit further

		//cuidado con la Y, si nuestro juego es 2D la ponemos a 0
		nextPos.y = 0;

		//reflejamos el vector velocidad para que de la sensacion de que rebota en la pared
		//velocity = reflect(velocity, collnorm) * 0.95;
	}

	Vector3 movement = nextPos - playerPos;
	Vector3 modelCoordinates = Vector3(movement.x, 0.0f, movement.z);

	return modelCoordinates;
}

int Player::CheckCollision(Camera* cam, std::vector<Entity*>& entities, Entity* sEnt) {
	int object_type = 0;
	Vector3 dir = RayDirection(cam);
	Vector3 rOrigin = cam->eye;
	// Colisiones para los elementos del vector entities --> Cambiarlo a las scenes tmb + sky
	for (size_t i = 0; i < entities.size(); i++) {
		Vector3 pos;
		Vector3 normal;
		Entity* entity = entities[i];
		if (entity->mesh->testRayCollision(entity->model, rOrigin, dir, pos, normal)) {
			if (entity->entity_type == eEntityType::HINT) {
				object_type = 2;
				//std::cout << "HINT" << std::endl;
			}
			else
				object_type = 3;
			//std::cout << "Collision" << std::endl;
			sEnt = entity;
			break;
		}
	}
	return object_type;
}

bool Player::DetectHint(Camera* cam, std::vector<Entity*>& entities, Entity* sEnt)
{
	if (CheckCollision(cam, entities, sEnt) == 2) {
		return true;
	}
	return false;
}

void Player::RenderPlayer(Matrix44 model, Mesh* mesh, Texture* tex, Animation* anim, Shader* shader, Camera* cam, int primitive, float yaw, float pitch, float t)
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