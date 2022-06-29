#include "entity.h"
#include "shader.h"
#include "utils.h"
#include "game.h"
#include "input.h"
#include "animation.h"
#include "extra/cJSON.h"

void RenderMesh(Matrix44& model, Mesh* a_mesh, Texture* tex, Shader* a_shader, Camera* cam, int primitive) {

	if (!a_shader) return;

	//enable shader
	a_shader->enable();

	//upload uniforms
	a_shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	a_shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
	if (tex != NULL) {
		a_shader->setUniform("u_texture", tex, 0);
	}
	a_shader->setUniform("u_time", time);
	a_shader->setUniform("u_tex_tiling", 1.0f);
	a_shader->setUniform("u_model", model);
	a_mesh->render(primitive);

	//disable shader
	a_shader->disable();
}

Vector3 RayDirection(Camera* cam) {
	// Para definir punto donde spawnear el objeto
	Game* g = Game::instance;
	Vector2 mouse = Input::mouse_position; // Conseguimos la posición del mouse -> Pondremos mesh donde puntero mouse
	Vector3 dir = cam->getRayDirection(mouse.x, mouse.y, g->window_width, g->window_height); // Sacamos la dirección

	return dir;
}
void AddEntityInFront(Camera* cam, Mesh* a_mesh, Texture* tex, std::vector<Entity*>& entities) {
	
	Vector3 dir = RayDirection(cam);
	Vector3 rOrigin = cam->eye;

	Vector3 spawnPos = RayPlaneCollision(Vector3(), Vector3(0, 1, 0), rOrigin, dir);; // Sacar la intersección entre la dirección y el plano

	Matrix44 model;
	model.translate(spawnPos.x, spawnPos.y, spawnPos.z);
	//model.scale(3.0f, 3.0f, 3.0f); // Depende de a que distancia estés, te lo pone en un tamaño u otro

	Entity* entity = new Entity();
	entity->model = model;

	entity->mesh = a_mesh;
	entity->texture = tex;
	entities.push_back(entity);
}

void CheckCollision(Camera* cam, std::vector<Entity*>& entities, Entity* sEnt) {

	Vector3 dir = RayDirection(cam);
	Vector3 rOrigin = cam->eye;
	// Colisiones para los elementos del vector entities --> Cambiarlo a las scenes tmb + sky
	for (size_t i = 0; i < entities.size(); i++) {
		Vector3 pos;
		Vector3 normal;
		Entity* entity = entities[i];
		if (entity->mesh->testRayCollision(entity->model, rOrigin, dir, pos, normal)) {
			std::cout << "Collision" << std::endl;
			sEnt = entity;
			break;
		}
	}
}

void CheckSkyCollision(Camera* camera, Matrix44 skyModel, Mesh* skyMesh) {
	Vector3 dir = RayDirection(camera);
	Vector3 rOrigin = camera->eye;
	Vector3 pos;
	Vector3 normal;
	if (skyMesh->testRayCollision(skyModel, rOrigin, dir, pos, normal)) {
		std::cout << "Collision" << std::endl;
	}
}


void RenderObjects(Mesh* mesh, Texture* tex, Shader* shader, int width, int height, float padding, float no_render_dist) {

	if (shader)
	{
		//enable shader
		shader->enable();

		Camera* cam = Game::instance->camera;
		//upload uniforms
		shader->setUniform("u_color", Vector4(1, 1, 1, 1));
		shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
		shader->setUniform("u_texture", tex, 0);
		shader->setUniform("u_time", time);

		//do the draw call
		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {

				Matrix44 hModel;
				hModel.scale(8, 8, 8);
				hModel.translate(i * padding, 0.0f, j * padding);

				Vector3 hPos = hModel.getTranslation();
				Vector3 camPos = cam->eye; //Posición de la camara
				float dist = hPos.distance(camPos);

				if (dist > no_render_dist) {
					continue;
				}

				// Para renderizar lo que necesitamos
				BoundingBox box = transformBoundingBox(hModel, mesh->box);
				if (!cam->testBoxInFrustum(box.center, box.halfsize)) { //Si no tenemos la pos de la house, no la renderizamos
					continue;
				}

				shader->setUniform("u_model", hModel);
				mesh->render(GL_TRIANGLES);
			}
		}
		//disable shader
		shader->disable();
	}
}

void RenderMeshWithAnim(Matrix44& model, Mesh* a_mesh, Texture* tex, Animation* anim, Shader* a_shader, Camera* cam, int primitive, float t) {
	if (!a_shader) return;
	//enable shader
	a_shader->enable();

	anim->assignTime(t);

	//upload uniforms
	a_shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	a_shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
	if (tex != NULL) {
		a_shader->setUniform("u_texture", tex, 0);
	}
	a_shader->setUniform("u_time", Game::instance->time);
	a_shader->setUniform("u_tex_tiling", 1.0f);
	a_shader->setUniform("u_model", model);
	a_mesh->renderAnimated(primitive, &anim->skeleton);

	//disable shader
	a_shader->disable();
}

LightEntity::LightEntity()
{
	//entity_type = eEntityType::LIGHT;
	color.set(1, 1, 1);
	intensity = 10;
	max_distance = 100;
	cone_angle = 45;
	cone_exp = 60;
	area_size = 1000;

	//light_type = eLightType::POINTLight;
}
