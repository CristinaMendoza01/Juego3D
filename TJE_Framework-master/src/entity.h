#ifndef ENTITY_H
#define ENTITY_H

#include "framework.h"
#include "mesh.h"
#include "texture.h"
#include "camera.h"

class Entity {
public:
	Matrix44 model;
	Mesh* mesh;
	Texture* texture;
};

void RenderMesh(Matrix44& model, Mesh* a_mesh, Texture* tex, Shader* a_shader, Camera* cam);
#endif