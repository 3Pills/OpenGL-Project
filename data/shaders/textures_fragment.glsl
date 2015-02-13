#version 410

in vec2 frag_tex_coord;
out vec4 frag_color;
uniform sampler2D diffuse;
uniform sampler2D normal;

void main(){
	frag_color = texture(diffuse, frag_tex_coord);
	vec4 norm_color = texture(normal, frag_tex_coord);
	frag_color = frag_color * norm_color;
}