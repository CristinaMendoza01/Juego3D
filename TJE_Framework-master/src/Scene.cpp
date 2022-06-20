#include "Scene.h"
#include "utils.h"

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
	clear();

	std::string map_content;
	readFile(filename, map_content);

	//divide the file into lines (entities per line)
	std::vector<std::string> lines = split(map_content, '\n');
	
	//for each line read the parameters to load every entity
	for (int i = 0; i < lines.size(); ++i) {
		std::vector<std::string> line = split(lines[i], ' ');

		int type = stoi(line[TYPE]);
		Mesh* mesh = Mesh::Get(line[MESH].c_str());
		Texture* texture = new Texture();
		texture->load(line[TEXTURE].c_str());

		//Position
		std::vector<std::string> position_str = split(line[POSITION], ',');
		Vector3 position = Vector3(stof(position_str[0]), stof(position_str[1]), stof(position_str[2]));
		position = position * 10; //adapt scale

		//Rotation
		std::vector<std::string> rotation_str = split(line[ROTATION], ',');
		Vector3 rotation = Vector3(stof(rotation_str[0]), stof(rotation_str[1]), stof(rotation_str[2]));

		//Rotation
		std::vector<std::string> scale_str = split(line[SCALE], ',');
		Vector3 scale = Vector3(stof(scale_str[0]), stof(scale_str[1]), stof(scale_str[2]));


		Entity* ent = createEntity(type, mesh, texture, position, rotation, scale);

		addEntity(ent);
	}
}

Entity* Scene::createEntity(int type, Mesh* mesh, Texture* texture, Vector3 position, Vector3 rotation, Vector3 scale)
{
	if (type == eEntityType::OBJECT) {
		Entity* ent = new Entity();

		ent->mesh = mesh;
		ent->texture = texture;
		//ent->model = ;

		return ent;
	}
	if (type == eEntityType::LIGHT) {

	}
	if (type == eEntityType::PLAYER) {

	}

	return NULL;
}
