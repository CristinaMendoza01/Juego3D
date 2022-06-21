#include "entity.h"
#include "shader.h"
#include "utils.h"
#include "extra/cJSON.h"

void RenderMesh(Matrix44& model, Mesh* a_mesh, Texture* tex, Shader* a_shader, Camera* cam) {

	if (!a_shader) return;

	//enable shader
	a_shader->enable();

	//upload uniforms
	a_shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	a_shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
	a_shader->setUniform("u_texture", tex, 0);
	a_shader->setUniform("u_time", time);

	a_shader->setUniform("u_model", model);
	a_mesh->render(GL_TRIANGLES);

	//disable shader
	a_shader->disable();
}

LightEntity::LightEntity()
{
	entity_type = eEntityType::LIGHT;
	color.set(1, 1, 1);
	intensity = 10;
	max_distance = 100;
	cone_angle = 45;
	cone_exp = 60;
	area_size = 1000;

	light_type = eLightType::POINTLight;
}

void LightEntity::configure(cJSON* json)
{
	color = readJSONVector3(json, "color", color);
	intensity = readJSONNumber(json, "intensity", intensity);
	max_distance = readJSONNumber(json, "max_dist", max_distance);
	std::string str = readJSONString(json, "light_type", "");
	if (str == "POINT")
	{
		light_type = eLightType::POINTLight;
	}
	else if (str == "SPOT") {
		light_type = eLightType::SPOT;
		cone_angle = readJSONNumber(json, "cone_angle", cone_angle);
		cone_exp = readJSONNumber(json, "cone_exp", cone_exp);
	}
	else if (str == "DIRECTIONAL") {
		light_type = eLightType::DIRECTIONAL;
		area_size = readJSONNumber(json, "area_size", area_size);
	}
}
