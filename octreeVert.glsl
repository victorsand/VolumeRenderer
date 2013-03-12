#version 330

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

in vec4 position; 

out vec4 eye;
out vec4 cubeOrigin;
out float cubeSize;

void main() {

	// Transform vertex position
	gl_Position = projMatrix * viewMatrix * modelMatrix * position;
	
	// Keep track of where the cube is in eye space, and how large it is
	cubeOrigin = vec4(0.f, 0.f, 0.f, 1.f);
	vec4 v1 = vec4(1.f, 0.f, 0.f, 1.f);
	cubeSize = length(v1-cubeOrigin); 

	eye = vec4(0.f, 0.f, 0.f, 1.f);
}