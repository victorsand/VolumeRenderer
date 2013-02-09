#version 330

uniform sampler2D cubeFrontTex;
uniform sampler2D cubeBackTex;
uniform sampler3D volumeTex;

void main() {

	float delta = 0.01;
	float amp = 9.f;

	float winSize = 600.0;
	float xSample = gl_FragCoord.x / winSize;
	float ySample = gl_FragCoord.y / winSize;

	vec4 front = texture(cubeFrontTex, vec2(xSample, ySample));
	vec4 back = texture(cubeBackTex, vec2(xSample, ySample));

	// Adjust coord system
	front.xyz = vec3(front.z, 1.0-front.x, 1.0-front.y);
	back.xyz = vec3(back.z, 1.0-back.x, 1.0-back.y);

	vec3 direction = (back-front).xyz;
	float dirLength = length(direction);
	direction = normalize(direction);

	float intensity = 0.0;
	vec3 sample = front.xyz;
	float traversedLength = 0.f;
	
	while (traversedLength < dirLength) {
		sample += delta * direction;
		traversedLength += delta;
		intensity += texture(volumeTex, sample).r;
	}

	gl_FragColor = vec4(vec3(amp*delta*intensity), 1.0);// * 0.001 + vec4(vec3(front), 1);

}