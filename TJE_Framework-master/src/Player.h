#pragma once
#include "entity.h"

struct sPlayer {
	Vector3 pos;
	float yaw;
	float pitch;
};

class player : public Entity
{
	Matrix44 model;
	Mesh* mesh;
	Texture* texture;

	float yaw;

	float pitch;

	Animation* anim_idle;
	Animation* anim_walk;
	Animation* anim_run;

	player();

	void RenderPlayer(Matrix44 model, Mesh* mesh, Texture* textrure, Animation* anim, Shader* shader, Camera* cam, int primitive, float yaw, float pitch, float t);

};