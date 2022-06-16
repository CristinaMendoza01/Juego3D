#include "entity.h"
#include "shader.h"
#include "animation.h"
#include "game.h"
#include "input.h"

void AddEntityInFront(Camera* cam, Mesh* a_mesh, Texture* tex, std::vector<Entity*> &entities) {

	// Para definir punto donde spawnear el objeto
	Game* g = Game::instance;
	Vector2 mouse = Input::mouse_position; // Conseguimos la posición del mouse -> Pondremos mesh donde puntero mouse
	Vector3 dir = cam->getRayDirection(mouse.x, mouse.y, g->window_width, g->window_height); // Sacamos la dirección
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

void RenderMesh(Matrix44& model, Mesh* a_mesh, Texture* tex, Shader* a_shader, Camera* cam) {

	if (!a_shader) return;

	//enable shader
	a_shader->enable();

	//upload uniforms
	a_shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	a_shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
	a_shader->setUniform("u_texture", tex, 0);
	a_shader->setUniform("u_time", time);

	a_shader->setUniform("u_model", model);
	a_mesh->render(GL_TRIANGLES);

	//disable shader
	a_shader->disable();
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

				//hMesh = Mesh::Get("data/house1.ASE"); // mesh = mesh_low
				//if (dist < lodDist) {	//Si tenemos lods --> mesh_low_1, mesh_low_2 etc
					// mesh = mesh_normal
				//}

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


void RenderMeshWithAnim(Matrix44& model, Mesh* a_mesh, Texture* tex, Animation* anim, Shader* a_shader, Camera* cam, float t) {
	a_shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");
	if (!a_shader) return;
	//enable shader
	a_shader->enable();

	anim->assignTime(t);

	//upload uniforms
	a_shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	a_shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
	a_shader->setUniform("u_texture", tex, 0);
	a_shader->setUniform("u_time", time);
	a_shader->setUniform("u_tex_tiling", 1.0f);
	a_shader->setUniform("u_model", model);
	a_mesh->renderAnimated(GL_TRIANGLES, &anim->skeleton);

	//disable shader
	a_shader->disable();
}
