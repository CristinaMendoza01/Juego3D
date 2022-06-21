#ifndef ENTITY_H
#define ENTITY_H

#include "framework.h"
#include "mesh.h"
#include "texture.h"
#include "camera.h"

enum eEntityType {
	NONE = 0,
	LIGHT = 1,
	PLAYER = 2,
	OBJECT = 3,
};

enum eLightType {
	POINTLight = 0,
	SPOT = 1,
	DIRECTIONAL = 2
};

class cJSON;
class Scene;

class Entity {
public:
	Scene* scene;
	Matrix44 model;
	Mesh* mesh;
	Texture* texture;
	eEntityType entity_type;
};

void RenderMesh(Matrix44& model, Mesh* a_mesh, Texture* tex, Shader* a_shader, Camera* cam);

class LightEntity : public Entity {
public:
	//General features
	Vector3 color;
	float intensity;
	eLightType light_type;
	float max_distance;

	//Spot Light
	float cone_angle;
	float cone_exp;

	//Directional Light
	float area_size;

	LightEntity();
	LightEntity(eLightType light_type);

	void configure(cJSON* json);
};
#endif