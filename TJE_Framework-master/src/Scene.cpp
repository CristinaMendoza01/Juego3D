#include "Scene.h"

Scene* Scene::instance = NULL;

Scene::Scene()
{
	instance = this;

}

void Scene::clear()
{
	for (int i = 0; i < entities.size(); ++i)
	{
		Entity* ent = entities[i];
		delete ent;
	}
	entities.resize(0);
}


void Scene::addEntity(Entity* entity)
{
	entities.push_back(entity); entity->scene = this;
}

bool Scene::loadMap(const char* filename)
{
	//TODO
}

Entity* Scene::createEntity(std::string type)
{
	if (type == "LIGHT")
		return new LightEntity();

	return NULL;
}
