#version 330

uniform sampler2D cubeFrontTex;
uniform sampler2D cubeBackTex;
uniform samplerBuffer volumeTex;

uniform float stepSize;
uniform float intensity;
uniform float winSizeX;
uniform float winSizeY;

out vec4 color;

void main() {

	// Get window coordinates for cube texture sampling
	float xSample = gl_FragCoord.x / winSizeX;
	float ySample = gl_FragCoord.y / winSizeY;

	// Sample cube colors
	vec4 front = texture(cubeFrontTex, vec2(xSample, ySample));
	vec4 back = texture(cubeBackTex, vec2(xSample, ySample));

	// Adjust coord system
	front.xyz = vec3(front.z, 1.0-front.x, 1.0-front.y);
	back.xyz = vec3(back.z, 1.0-back.x, 1.0-back.y);

	// Calculate viewing direction and cross-section length
	vec3 direction = (back-front).xyz;
	float dirLength = length(direction);
	direction = normalize(direction);

	// Init traversal
	float sum = 0.0;
	vec3 sample = front.xyz;
	float traversedLength = 0.0;
	
	// Sample volume
	while (traversedLength < dirLength) {
		sample += stepSize * direction;
		traversedLength += stepSize;
		sum += texelFetch(volumeTex, int(256.0*sample.x*pow(256.0, 0.0)) + int(256.0*sample.y*pow(256.0, 1.0)) + int(256.0*sample.z*pow(256.0, 2.0)));
	}

	color = vec4(vec3(intensity*stepSize*sum), 1.0);//  * 0.001 + vec4(vec3(front), 1);

}