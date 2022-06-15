#pragma once
#include "entity.h"

class Scene
{
public:
	static Scene* instance;

	Vector3 background_color;
	Vector3 ambient_light;
	Camera main_camera;

	Scene();

	void clear();

	void addEntity(Entity* entity);

	bool loadMap(const char* filename);

	Entity* createEntity(std::string type);

	std::string filename;
	std::vector<Entity*> entities;



};

