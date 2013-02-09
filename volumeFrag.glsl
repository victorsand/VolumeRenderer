#version 330

uniform sampler2D cubeFrontTex;
uniform sampler2D cubeBackTex;
uniform sampler3D volumeTex;

void main() {

	float delta = 0.01;

	float winSize = 600.0;
	float xSample = gl_FragCoord.x / winSize;
	float ySample = gl_FragCoord.y / winSize;

	vec4 front = texture(cubeFrontTex, vec2(xSample, ySample));
	vec4 back = texture(cubeBackTex, vec2(xSample, ySample));
	vec4 direction = back-front;
	float length = length(direction);
	direction = normalize(direction);

	vec3 color = vec3(0.0, 0.0, 0.0);
	vec3 sample = front.xyz;
	float sampleLength = 0;
	while (sampleLength < length) {
		float value = texture(volumeTex, sample);
		color += vec3(value);
		sample = sample + 0.01 * direction.xyz;
		sampleLength += delta;
	}

	gl_FragColor = vec4(color, 1.0);

}