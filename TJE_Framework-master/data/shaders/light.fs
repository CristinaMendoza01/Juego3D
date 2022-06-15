#version 330 core

in vec3 v_position;
in vec3 v_world_position;
in vec3 v_normal;
in vec2 v_uv;
in vec4 v_color;

uniform vec4 u_color;
uniform sampler2D u_color_texture;
uniform sampler2D u_emissive_texture;
uniform sampler2D u_occlusion_texture;
uniform sampler2D u_normalmap_texture;
uniform sampler2D u_lshadowmap;
uniform float u_time;
uniform float u_alpha_cutoff;

uniform vec3 u_lcolor;
uniform vec3 u_lpos;
uniform float u_lmaxdist;
uniform float u_coslcone_angle;
uniform float u_lcone_exp;
uniform float u_lshadowbias;
uniform vec3 u_ambient_light;
uniform float u_lareasize;
uniform int u_lastlight;
uniform int u_ltype;
uniform int u_lshadowcast;

uniform vec3 u_ldir;
uniform vec3 u_emissive_factor;

uniform mat4 u_lshadowmap_vp;

out vec4 FragColor;

float TestShadowMap(vec3 pos){
	if ( u_ltype == 1){
		//project our 3D position to the shadowmap
		vec4 proj_pos = u_lshadowmap_vp * vec4(pos,1.0);

		//from homogeneus space to clip space
		vec2 shadow_uv = proj_pos.xy / proj_pos.w;

		//from clip space to uv space
		shadow_uv = shadow_uv * 0.5 + vec2(0.5);

		//get point depth [-1 .. +1] in non-linear space
		float real_depth = (proj_pos.z - u_lshadowbias) / proj_pos.w;

		//normalize from [-1..+1] to [0..+1] still non-linear
		real_depth = real_depth * 0.5 + 0.5;

		//read depth from depth buffer in [0..+1] non-linear
		float shadow_depth = texture( u_lshadowmap, shadow_uv).x;

		//compute final shadow factor by comparing
		float shadow_factor = 1.0;

		//we can compare them, even if they are not linear
		if( shadow_depth < real_depth )
			shadow_factor = 0.0;
		return shadow_factor;
	}
}


mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx( p );
	vec3 dp2 = dFdy( p );
	vec2 duv1 = dFdx( uv );
	vec2 duv2 = dFdy( uv );
	
	// solve the linear system
	vec3 dp2perp = cross( dp2, N );
	vec3 dp1perp = cross( N, dp1 );
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
	// construct a scale-invariant frame 
	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
	return mat3( T * invmax, B * invmax, N );
}

// assume N, the interpolated vertex normal and 
// WP the world position
//vec3 normal_pixel = texture2D( normalmap, uv ).xyz; 
vec3 perturbNormal(vec3 N, vec3 WP, vec2 uv, vec3 normal_pixel)
{
	normal_pixel = normal_pixel * 255./127. - 128./127.;
	mat3 TBN = cotangent_frame(N, WP, uv);
	return normalize(TBN * normal_pixel);
}


void main()
{

	vec3 N = perturbNormal(v_normal, v_world_position, v_uv, texture(u_normalmap_texture, v_uv).xyz);

	vec2 uv = v_uv;
	vec4 color = u_color;
	color *= texture( u_color_texture, v_uv );

	if(color.a < u_alpha_cutoff)
		discard;


	vec3 light = vec3(u_ambient_light);

	//vector from the point to the light
	vec3 L = (u_lpos - v_world_position);

	float dist = length(L);

	L /= dist;

	float att = u_lmaxdist - dist;

	att /= u_lmaxdist;

	att = max(att, 0.0);

	att *= pow(att, 2);

	float NdotL = clamp(dot(N,L), 0.0, 1.0);

	vec3 R = L - 2*(NdotL)*N;

	if( u_ltype == 1){

		vec3 D = -normalize(u_ldir);

		float spotCosine = dot(D,L);

		float spotFactor = 0;

		if (spotCosine >= u_coslcone_angle) { 
		    spotFactor = pow(spotCosine, u_lcone_exp);
		}
		else { // The point is outside the cone of light from the spotlight.  
		    spotFactor = 0.0; // The light will add no color to the point.
		}

		// Light intensity will be multiplied by spotFactor
		NdotL *= spotFactor;

	} else if( u_ltype == 2){

		L = normalize(u_ldir);
		att = 1;
	
	}

	float shadow_factor = 1.0;

	if(u_lshadowcast == 1){
		shadow_factor= TestShadowMap(v_world_position);
	}

	light += ((NdotL * u_lcolor) * att * shadow_factor);

	color.xyz *= (light) + (texture( u_occlusion_texture, v_uv ).xyz * u_ambient_light);

	if(u_lastlight == 1){
		color.xyz += texture( u_emissive_texture, v_uv ).rgb;
	}
	
	

	FragColor = color;
}
