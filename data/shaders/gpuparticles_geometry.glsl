#version 410

// input is points but output a quad
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

// input data from vertex shader
in vec3 position[];
in float lifetime[];
in float lifespan[];

// output to fragment shader
out vec4 vColor;
out vec2 fragTexCoord;

uniform mat4 projView;
uniform mat4 camTransform;

uniform float startSize;
uniform float endSize;

uniform vec4 startColor;
uniform vec4 endColor;

uniform float deltaTime;
uniform float fadeIn;
uniform float fadeOut;

void main() {
	// interpolate colour
	vColor = mix(startColor, endColor, lifetime[0] / lifespan[0]);
	
	// interpolate alpha of particle, based on its current time alive.
	vColor.a = min(lifetime[0] / fadeIn, min((lifespan[0] - lifetime[0]) / fadeOut, 1));

	// calculate the size and create the corners of a quad
	float halfSize = mix(startSize,endSize,lifetime[0]/lifespan[0]) * 0.5f;

	vec3 corners[4];
	corners[0] = vec3( halfSize, -halfSize, 0 );
	corners[1] = vec3( halfSize, halfSize, 0 );
	corners[2] = vec3( -halfSize, -halfSize, 0 );
	corners[3] = vec3( -halfSize, halfSize, 0 );

	// billboard
	vec3 zAxis = normalize( camTransform[3].xyz - position[0] );
	vec3 xAxis = cross( camTransform[1].xyz, zAxis );
	vec3 yAxis = cross( zAxis, xAxis );
	mat3 billboard = mat3(xAxis,yAxis,zAxis);

	//Assign values to each vertex.
	gl_Position = projView*vec4(billboard*corners[0]+position[0], 1);
	fragTexCoord = vec2(1,0);
	EmitVertex();

	gl_Position = projView*vec4(billboard*corners[1]+position[0], 1);
	fragTexCoord = vec2(1,1);
	EmitVertex();

	gl_Position = projView*vec4(billboard*corners[2]+position[0], 1);
	fragTexCoord = vec2(0,0);
	EmitVertex();

	gl_Position = projView*vec4(billboard*corners[3]+position[0], 1);
	fragTexCoord = vec2(0,1);
	EmitVertex();
}