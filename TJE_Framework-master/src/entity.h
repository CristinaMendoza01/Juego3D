#ifndef ENTITY_H
#define ENTITY_H

#include "framework.h"
#include "mesh.h"
#include "texture.h"
#include "camera.h"
#include "animation.h"

enum eEntityType {
	NONE = 0,
	PROP = 1,
	PLAYER = 2,
	OBJECT = 3,
};

class cJSON;
class Scene;

class Entity {
public:
	Matrix44 model;
	Mesh* mesh;
	Texture* texture;
	eEntityType entity_type;
};

class LightEntity : public Entity {
public:
	//General features
	Vector3 color;
	float intensity;
	float max_distance;

	//Spot Light
	float cone_angle;
	float cone_exp;

	//Directional Light
	float area_size;

	LightEntity();

	void RenderLight(Camera* cam, Shader* shader);
};

void AddEntityInFront(Camera* cam, Mesh* a_mesh, Texture* tex, std::vector<Entity*>& entities);
void CheckCollision(Camera* cam, std::vector<Entity*>& entities, Entity* sEnt);
void CheckSkyCollision(Camera* camera, Matrix44 skyModel, Mesh* skyMesh);
void RenderMesh(Matrix44& model, Mesh* a_mesh, Texture* tex, Shader* a_shader, Camera* cam, int primitive);
void RenderObjects(Mesh* mesh, Texture* tex, Shader* shader, int width, int height, float padding, float no_render_dist);
void RenderMeshWithAnim(Matrix44& model, Mesh* a_mesh, Texture* tex, Animation* anim, Shader* a_shader, Camera* cam, int primitive, float t);
#endif