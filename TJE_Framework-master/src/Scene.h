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

	Entity* createEntity(int type, Mesh* mesh, Texture* texture, Vector3 position, Vector3 rotation, Vector3 scale);
	
	std::string filename;
	std::vector<Entity*> entities;

	std::pair<Mesh*, Texture*> Scene::loadScene(const char* obj_filename, const char* tex_filename);
};

void putCamera(Matrix44 model, Camera* camera, bool locked, int w, int h);


