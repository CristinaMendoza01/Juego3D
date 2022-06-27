#include "scene.h"
#include "utils.h"
#include "extra/textparser.h"
#include "input.h"
#include "game.h"
#include "player.h"

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

void putCamera(Matrix44 model, Camera* camera, bool locked, int w, int h) {
	Vector3 eye = model * Vector3(0.f, 150.f, 1.f);
	Vector3 center = model * Vector3(0.f, 15.f, 10.f);
	Vector3 up = model.rotateVector(Vector3(0.f, 150.f, 0.f));

	//Create our camera
	camera->lookAt(Vector3(0.f, 100.f, 100.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0

	if (locked) //Entramos en modo 1a Persona.
		camera->lookAt(eye, center, up);
	camera->setPerspective(70.f, w / (float)h, 0.1f, 1000000.f); //set the projection, we want to be perspective
	// Si añadimos unos ceros más en el último param -> Vemos más lejos
}


void MiniMapa(sPlayer player, Matrix44 model, std::pair <Mesh*, Texture*> cityLevel, Shader* shader) {
	int wWidth = Game::instance->window_width;
	int wHeight = Game::instance->window_height;
	glViewport(wWidth - 200, wHeight - 200, 200, 200);
	glClear(GL_DEPTH_BUFFER_BIT);

	// CÓDIGO PROFE --> NO FUNCIONA
	Camera cam;
	cam.setPerspective(60, 1, 0.1f, 1000.f);
	Vector3 eye = player.pos + Vector3(0, 100, 0);
	Vector3 center = player.pos;
	Vector3 up = model.rotateVector(Vector3(0, 0, -1));
	cam.lookAt(eye, center, up);
	Matrix44 mapModel;
	mapModel.scale(100, 100, 100);
	RenderMesh(mapModel, cityLevel.first, cityLevel.second, shader, &cam, GL_TRIANGLES);

	glViewport(0, 0, wWidth, wHeight);
}

void RenderSky(Matrix44 skyModel, Mesh* skyMesh, Texture* skyTex, Shader* shader, Camera* camera) {
	//skyModel.translate(camera->eye.x, camera->eye.y - 50.0f, camera->eye.z);
	//glDisable(GL_DEPTH_TEST);
	RenderMesh(skyModel, skyMesh, skyTex, shader, camera, GL_TRIANGLES);
	skyModel.setScale(1000, 1000, 1000);
	//glEnable(GL_DEPTH_TEST);
}