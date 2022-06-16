#ifndef ENTITY_H
#define ENTITY_H

#include "framework.h"
#include "mesh.h"
#include "texture.h"
#include "camera.h"
#include "animation.h"

class Entity {
public:
	Matrix44 model;
	Mesh* mesh;
	Texture* texture;
};

void AddEntityInFront(Camera* cam, Mesh* a_mesh, Texture* tex, std::vector<Entity*> &entities);
void RenderMesh(Matrix44& model, Mesh* a_mesh, Texture* tex, Shader* a_shader, Camera* cam);
void RenderObjects(Mesh* mesh, Texture* tex, Shader* shader, int width, int height, float padding, float no_render_dist);
void RenderMeshWithAnim(Matrix44& model, Mesh* a_mesh, Texture* tex, Animation* anim, Shader* a_shader, Camera* cam, float t);

#endif