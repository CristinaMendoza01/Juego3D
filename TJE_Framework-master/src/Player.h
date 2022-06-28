#pragma once
#include "entity.h"

struct sPlayer {
	Vector3 pos;
	float yaw;
	float pitch;
};

class player : public Entity
{
	
	Vector3 pos;
	Animation* anim_idle;
	Animation* anim_walk;
	Animation* anim_run;
	float yaw;

	player();


	void Update();

};