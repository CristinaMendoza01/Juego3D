#include "player.h"
#include "shader.h"
#include "texture.h"

void player::RenderPlayer(Matrix44 model, Mesh* mesh, Texture* tex, Animation* anim, Shader* shader, Camera* cam,int primitive, float yaw, float pitch, float t)
{
	if (!shader) return;
	//enable shader
	shader->enable();

	anim->assignTime(t);

	//upload uniforms
	shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
	if (tex != NULL) {
		shader->setUniform("u_texture", tex, 0);
	}
	shader->setUniform("u_time", time);
	shader->setUniform("u_tex_tiling", 1.0f);
	shader->setUniform("u_model", model);
	mesh->renderAnimated(primitive, &anim->skeleton);

	//disable shader
	shader->disable();
}
