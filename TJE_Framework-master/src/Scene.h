#pragma once
#include "entity.h"
#include "player.h"

class Scene
{
public:
	static Scene* instance;

	Vector3 background_color;
	Vector3 ambient_light;
	Camera main_camera;

	Player* player;

	Scene();

	void clear();

	void loadMap(const char* filename);

	Entity* createEntity(int type, Mesh* mesh, Texture* texture, Vector3 position, Vector3 rotation, Vector3 scale);
	
	std::string filename;

	std::vector<Entity*> entities;
};

void putCamera(Matrix44 model, Camera* camera, bool locked, int w, int h, int cameracontroller);
void MiniMapa(Player player, Matrix44 model, std::pair <Mesh*, Texture*> cityLevel, Shader* shader);
void RenderSky(Matrix44 skyModel, Mesh* skyMesh, Texture* skyTex, Shader* shader, Camera* camera);

