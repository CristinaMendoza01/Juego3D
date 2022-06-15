#pragma once
#include "entity.h"

class Player : public Entity
{
	Matrix44 model;
	Mesh* mesh;
	Texture* texture;

	Player();


	void Update();

};