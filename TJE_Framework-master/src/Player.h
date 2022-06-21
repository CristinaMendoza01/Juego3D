#pragma once
#include "entity.h"


class Player : public Entity
{
public:
	Vector3 pos;
	float yaw;

	Player();


	void Update();

};