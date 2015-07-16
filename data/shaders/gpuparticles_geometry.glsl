#version 410

// input is points but output a quad
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

// input data from vertex shader
in vec3 vPosition[];
in float vLifetime[];
in float vLifespan[];

// output to fragment shader
out vec4 fColor;
out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fPosition;

uniform bool deferred;

uniform mat4 view;
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
	fColor = mix(startColor, endColor, vLifetime[0] / vLifespan[0]);
	
	// interpolate alpha of particle, based on its current time alive.
	fColor.a = min(vLifetime[0] / fadeIn, min((vLifespan[0] - vLifetime[0]) / fadeOut, 1));

	// calculate the size and create the corners of a quad
	float halfSize = mix(startSize,endSize,vLifetime[0]/vLifespan[0]) * 0.5f;

	vec3 corners[4];
	corners[0] = vec3( halfSize, -halfSize, 0 );
	corners[1] = vec3( halfSize, halfSize, 0 );
	corners[2] = vec3( -halfSize, -halfSize, 0 );
	corners[3] = vec3( -halfSize, halfSize, 0 );

	// billboard
	vec3 zAxis = normalize( camTransform[3].xyz - vPosition[0] );
	vec3 xAxis = cross( camTransform[1].xyz, zAxis );
	vec3 yAxis = cross( zAxis, xAxis );
	mat3 billboard = mat3(xAxis,yAxis,zAxis);

	fNormal = vec3(0,1,0);
	//if (deferred)
	//	fNormal = (view * vec4(fNormal, 1)).xyz;

	//Assign values to each vertex.
	gl_Position = projView*vec4(billboard*corners[0]+vPosition[0], 1);
	fTexCoord = vec2(1,0);
	fPosition = vec3(gl_Position.xy, 0);
	EmitVertex();

	gl_Position = projView*vec4(billboard*corners[1]+vPosition[0], 1);
	fTexCoord = vec2(1,1);
	fPosition = vec3(gl_Position.xy, 0);
	EmitVertex();

	gl_Position = projView*vec4(billboard*corners[2]+vPosition[0], 1);
	fTexCoord = vec2(0,0);
	fPosition = vec3(gl_Position.xy, 0);
	EmitVertex();

	gl_Position = projView*vec4(billboard*corners[3]+vPosition[0], 1);
	fTexCoord = vec2(0,1);
	fPosition = vec3(gl_Position.xy, 0);
	EmitVertex();
}