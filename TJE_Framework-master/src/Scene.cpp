#include "scene.h"
#include "utils.h"
#include "extra/textparser.h"

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
	TextParser* parser = new TextParser(filename);

	clear();

	while (!parser->eof())
	{

		// entity name
		int type = parser->getint();

		if (type == 10) { break; }

		//std::cout << ent_name << "\n";

		//mesh file
		Mesh* mesh = Mesh::Get(parser->getword());
		// texture file
		Texture* texture = Texture::Get(parser->getword());

		// matrix model
		float pos_x = parser->getfloat();
		float pos_y = parser->getfloat();
		float pos_z = parser->getfloat();


		float rot_x = parser->getfloat();
		float rot_y = parser->getfloat();
		float rot_z = parser->getfloat();

		float scale_x = parser->getfloat();
		float scale_y = parser->getfloat();
		float scale_z = parser->getfloat();

		Vector3 position = Vector3(pos_x, pos_y,pos_z);
		Vector3 rotation = Vector3(rot_x, rot_y, rot_z);
		Vector3 scale = Vector3(scale_x, scale_y, scale_z);

		//create the entity
		Entity* ent = createEntity(type, mesh, texture, position, rotation, scale);

		//add the entity to the list
		addEntity(ent);
	}
	
	
}

Entity* Scene::createEntity(int type, Mesh* mesh, Texture* texture, Vector3 position, Vector3 rotation, Vector3 scale)
{
	Entity* ent = new Entity();

		ent->mesh = mesh;
		ent->texture = texture;
		ent->model.translate(position.x, position.y, position.z);
		ent->model.rotate(180*DEG2RAD, rotation);
		ent->model.scale(scale.x, scale.y, scale.z);
		ent->entity_type = (eEntityType) type;

		return ent;
	
}

std::pair<Mesh*, Texture*> Scene::loadScene(const char* obj_filename, const char* tex_filename)
{
	Mesh* mesh = Mesh::Get(obj_filename);
	Texture* tex = Texture::Get(tex_filename);

	std::pair<Mesh*, Texture*> sol;

	sol.first = mesh;
	sol.second = tex;

	return sol;
}
