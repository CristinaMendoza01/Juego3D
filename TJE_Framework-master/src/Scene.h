#pragma once
#include "entity.h"

class Scene
{
public:
	static Scene* instance;

	Entity* entity;

	Entity getEntity();
	void setEntity(Entity e);

	void renderScene();
};

