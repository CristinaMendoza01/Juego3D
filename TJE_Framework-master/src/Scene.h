#pragma once
#include "entity.h"

enum eReadMap {
	TYPE = 0,
	MESH = 1,
	TEXTURE = 2,
	POSITION = 3,
	ROTATION = 4,
	SCALE = 5
};

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

	Entity* createEntity(int type, Mesh* mesh, Texture* texture, Vector3 position, Vector3 rotation, Vector3 scale);
	
	std::string filename;
	std::vector<Entity*> entities;



};

