
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec4 u_color;
uniform sampler2D u_texture;
uniform float u_time;
uniform vec4 u_tex_range; // [startx, starty, w, h] 

void main()
{
	vec2 uv = v_uv;
	// Agranda el rango y desplaza por el eje de las x
	uv.x = (u_tex_range.z) * uv.x + u_tex_range.x; // z = w
	uv.y = (u_tex_range.w) * uv.y + u_tex_range.y;
	gl_FragColor = u_color * texture2D( u_texture, uv );
}
