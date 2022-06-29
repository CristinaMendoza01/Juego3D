#pragma once
#include "entity.h"

enum ePlayerState {
	IDLE,
	WALK,
	RUN
};

class Player : public Entity
{
public:
	Matrix44 model;
	Mesh* mesh;
	Texture* texture;

	float yaw;

	float pitch;

	ePlayerState player_state;

	Animation* anim_idle;
	Animation* anim_walk;
	Animation* anim_run;

	Player();
	Player(Entity* ent);

	void RenderPlayer(Matrix44 model, Mesh* mesh, Texture* textrure, Animation* anim, Shader* shader, Camera* cam, int primitive, float yaw, float pitch, float t);
	void UpdatePlayer();

};