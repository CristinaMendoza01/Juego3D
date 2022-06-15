#include "entity.h"
#include "shader.h"
#include "animation.h"

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

void RenderMeshWithAnim(Matrix44& model, Mesh* a_mesh, Texture* tex, Animation* anim, Shader* a_shader, Camera* cam, float t) {
	a_shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");
	if (!a_shader) return;
	//enable shader
	a_shader->enable();

	anim->assignTime(t);

	//upload uniforms
	a_shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	a_shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
	a_shader->setUniform("u_texture", tex, 0);
	a_shader->setUniform("u_time", time);
	a_shader->setUniform("u_tex_tiling", 1.0f);
	a_shader->setUniform("u_model", model);
	a_mesh->renderAnimated(GL_TRIANGLES, &anim->skeleton);

	//disable shader
	a_shader->disable();
}
